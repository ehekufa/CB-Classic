#!/usr/bin/env python3
"""Кадры видео-интро из проекта Pocket Code.

В новой версии игры в сцене «Игра» есть объект «Интро»: скрипт вызывает
createVideo(videos, 2|3), показывает ролик на 2-3 секунды, затем deleteVideo.
Это MP4 1920x1080 (H.264), который движок порта сам декодировать не умеет,
поэтому кадры вытаскиваются заранее:

    tools/extract_videos.py --src original --out assets

Скрипт берёт ffmpeg (из PATH или из python-пакета imageio-ffmpeg), режет ролик
на кадры 480 px шириной с частотой 12 кадров/с и, если есть ImageMagick,
сжимает их в палитровый PNG (PNG-8) — так весь интро весит ~2 МБ вместо 6.
Рядом пишется assets/videos.lua:

    return { [2] = { prefix = "videos/538_2/", frames = 33, fps = 12 }, ... }

Движок (src/lua/runtime/engine.lua, E.video_play) проигрывает эти кадры поверх
сцены, а после deleteVideo кадры выгружаются из памяти.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys

FPS_DEFAULT = 12
WIDTH_DEFAULT = 480
COLORS_DEFAULT = 160


def ffmpeg_path() -> str | None:
    exe = shutil.which("ffmpeg")
    if exe:
        return exe
    try:  # ffmpeg, который тащит за собой imageio-ffmpeg
        import imageio_ffmpeg

        return imageio_ffmpeg.get_ffmpeg_exe()
    except Exception:
        return None


def convert_path() -> str | None:
    return shutil.which("convert") or shutil.which("magick")


def read_videos(object_dir: str) -> list[tuple[str, int]]:
    """videos.txt объекта: [["Обычный кубик", 2], ["Настя кубик", 3]]"""
    path = os.path.join(object_dir, "videos.txt")
    if not os.path.exists(path):
        return []
    try:
        with open(path, encoding="utf-8") as handle:
            data = json.load(handle)
    except (OSError, json.JSONDecodeError):
        return []
    out = []
    for entry in data:
        if isinstance(entry, list) and len(entry) >= 2:
            out.append((str(entry[0]), int(entry[1])))
    return out


def extract(video: str, target: str, ffmpeg: str, fps: int, width: int) -> int:
    if os.path.isdir(target):
        shutil.rmtree(target)
    os.makedirs(target, exist_ok=True)
    cmd = [
        ffmpeg, "-hide_banner", "-loglevel", "error", "-y",
        "-i", video,
        "-vf", f"fps={fps},scale={width}:-1",
        os.path.join(target, "f_%04d.png"),
    ]
    subprocess.run(cmd, check=True)
    return len([n for n in os.listdir(target) if n.endswith(".png")])


def shrink(frames_dir: str, convert: str, colors: int) -> None:
    """Палитровый PNG: без него интро весит в три раза больше."""
    for name in sorted(os.listdir(frames_dir)):
        if not name.endswith(".png"):
            continue
        path = os.path.join(frames_dir, name)
        tmp = path + ".tmp.png"
        cmd = [convert, path, "-dither", "None", "-colors", str(colors),
               "-depth", "8", "PNG8:" + tmp]
        if subprocess.run(cmd, stdout=subprocess.DEVNULL,
                          stderr=subprocess.DEVNULL).returncode == 0:
            os.replace(tmp, path)
        elif os.path.exists(tmp):
            os.remove(tmp)


def main() -> int:
    parser = argparse.ArgumentParser(description="Кадры видео-интро из Pocket Code")
    parser.add_argument("--src", default="original")
    parser.add_argument("--out", default="assets")
    parser.add_argument("--fps", type=int, default=FPS_DEFAULT)
    parser.add_argument("--width", type=int, default=WIDTH_DEFAULT)
    parser.add_argument("--colors", type=int, default=COLORS_DEFAULT)
    args = parser.parse_args()

    ffmpeg = ffmpeg_path()
    convert = convert_path()
    videos_root = os.path.join(args.out, "videos")
    os.makedirs(videos_root, exist_ok=True)

    entries: dict[int, dict] = {}
    total_bytes = 0
    total_frames = 0
    for scene in sorted(os.listdir(args.src)):
        scene_dir = os.path.join(args.src, scene)
        if not scene.startswith("scene_") or not os.path.isdir(scene_dir):
            continue
        for obj in sorted(os.listdir(scene_dir)):
            obj_dir = os.path.join(scene_dir, obj)
            if not obj.startswith("object_") or not os.path.isdir(obj_dir):
                continue
            for title, vid in read_videos(obj_dir):
                source = os.path.join(obj_dir, f"video_{vid}.mp4")
                if not os.path.exists(source):
                    print(f"видео {vid} «{title}»: нет файла {source}", file=sys.stderr)
                    continue
                if not ffmpeg:
                    print("видео: ffmpeg не найден — интро будет без кадров",
                          file=sys.stderr)
                    continue
                object_id = int(obj.split("_", 1)[1])
                slug = f"{object_id}_{vid}"
                target = os.path.join(videos_root, slug)
                frames = extract(source, target, ffmpeg, args.fps, args.width)
                if convert:
                    shrink(target, convert, args.colors)
                size = sum(os.path.getsize(os.path.join(target, n))
                           for n in os.listdir(target))
                total_bytes += size
                total_frames += frames
                entries[vid] = {"prefix": f"videos/{slug}/", "frames": frames,
                                "fps": args.fps, "title": title}
                print(f"видео {vid} «{title}»: {frames} кадров, {size // 1024} КБ")

    with open(os.path.join(args.out, "videos.lua"), "w", encoding="utf-8") as handle:
        handle.write("-- Сгенерировано tools/extract_videos.py из проекта Pocket Code.\n")
        handle.write("-- Кадры видео-интро (createVideo/deleteVideo).\n")
        handle.write("return {\n")
        for vid in sorted(entries):
            meta = entries[vid]
            handle.write(
                '  [%d] = { prefix = "%s", frames = %d, fps = %d, title = "%s" },\n'
                % (vid, meta["prefix"], meta["frames"], meta["fps"],
                   meta["title"].replace('"', '\\"')))
        handle.write("}\n")

    if not entries:
        with open(os.path.join(args.out, "videos.lua"), "w", encoding="utf-8") as handle:
            handle.write("-- Сгенерировано tools/extract_videos.py (видео в проекте нет).\n")
            handle.write("return {}\n")

    print("видео: %d, кадров %d, %.1f МБ"
          % (len(entries), total_frames, total_bytes / (1024 * 1024)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
