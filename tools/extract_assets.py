#!/usr/bin/env python3
"""Извлекает все картинки/звуки из импортированного Pocket Code проекта в удобное дерево.

    python3 tools/extract_assets.py --src original --out assets

Результат:

    assets/project.lua                 структура проекта для движка (C++/Lua)
    assets/index.json                  машинный индекс (пути, id, хеши, размеры)
    assets/README.md                   отчёт: сколько файлов, дубликаты, что пропущено
    assets/scenes/<scene>/<object>/images/<имя>.png
    assets/scenes/<scene>/<object>/sounds/<имя>.mp3
    assets/icons/<scene>.png           иконки сцен
    assets/icon.png                    иконка проекта

Имена транслитерируются в ASCII (кириллица -> латиница), исходные имена сохраняются в
index.json и в project.lua.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import struct
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from catrobat import load_project  # noqa: E402

TRANSLIT = {
    "а": "a", "б": "b", "в": "v", "г": "g", "д": "d", "е": "e", "ё": "e",
    "ж": "zh", "з": "z", "и": "i", "й": "y", "к": "k", "л": "l", "м": "m",
    "н": "n", "о": "o", "п": "p", "р": "r", "с": "s", "т": "t", "у": "u",
    "ф": "f", "х": "h", "ц": "c", "ч": "ch", "ш": "sh", "щ": "sch", "ъ": "",
    "ы": "y", "ь": "", "э": "e", "ю": "yu", "я": "ya",
}

# --- правки картинок после извлечения -------------------------------------
#
# Оригинал нарисован так, что часть интерфейса «запечена» в PNG. Счётчик кубков
# рисует движок (белым, в жёлтой полосе панели «Путь к богатству»), поэтому
# нарисованный в оригинале кубок в этой полосе закрашиваем цветом полосы.
IMAGE_PATCHES = {
    "scenes/01_menyu/008_put_k_bogatstvu/images/0006_put_k_bogatstvu.png":
        ((296, 132, 371, 200), (0xF0, 0xA6, 0x06, 0xFF)),
}


def _png_to_rgba(path: str):
    """Прочитать PNG 8-bit RGBA без интерлейса: (w, h, bytearray)."""
    data = open(path, "rb").read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        return None
    pos = 8
    width = height = 0
    idat = bytearray()
    while pos + 8 <= len(data):
        length = struct.unpack(">I", data[pos:pos + 4])[0]
        kind = data[pos + 4:pos + 8]
        body = data[pos + 8:pos + 8 + length]
        if kind == b"IHDR":
            width, height, depth, color, _, _, interlace = struct.unpack(">IIBBBBB", body)
            if depth != 8 or color != 6 or interlace != 0:
                return None
        elif kind == b"IDAT":
            idat += body
        elif kind == b"IEND":
            break
        pos += 12 + length
    if width <= 0 or height <= 0:
        return None
    raw = zlib.decompress(bytes(idat))
    stride = width * 4
    out = bytearray(stride * height)
    prev = bytearray(stride)
    src = 0
    for row in range(height):
        ftype = raw[src]
        src += 1
        line = bytearray(raw[src:src + stride])
        src += stride
        for i in range(stride):
            a = line[i - 4] if i >= 4 else 0
            b = prev[i]
            c = prev[i - 4] if i >= 4 else 0
            if ftype == 1:
                line[i] = (line[i] + a) & 0xFF
            elif ftype == 2:
                line[i] = (line[i] + b) & 0xFF
            elif ftype == 3:
                line[i] = (line[i] + ((a + b) >> 1)) & 0xFF
            elif ftype == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pred = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[i] = (line[i] + pred) & 0xFF
        out[row * stride:(row + 1) * stride] = line
        prev = line
    return width, height, out


def _rgba_to_png(path: str, width: int, height: int, pixels: bytearray) -> None:
    stride = width * 4
    raw = bytearray()
    for row in range(height):
        raw.append(0)                        # filter: none
        raw += pixels[row * stride:(row + 1) * stride]

    def chunk(kind: bytes, body: bytes) -> bytes:
        return (struct.pack(">I", len(body)) + kind + body +
                struct.pack(">I", zlib.crc32(kind + body) & 0xFFFFFFFF))

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    data = (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) +
            chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b""))
    with open(path, "wb") as fh:
        fh.write(data)


def patch_image(path: str, rel: str) -> None:
    """Закрасить часть картинки, которую в порте рисует движок."""
    patch = IMAGE_PATCHES.get(rel)
    if not patch:
        return
    rect, color = patch
    decoded = _png_to_rgba(path)
    if not decoded:
        print(f"предупреждение: {rel} — правка пропущена (не RGBA8)", file=sys.stderr)
        return
    width, height, pixels = decoded
    x0, y0, x1, y1 = rect
    for y in range(max(0, y0), min(height - 1, y1) + 1):
        for x in range(max(0, x0), min(width - 1, x1) + 1):
            off = (y * width + x) * 4
            pixels[off:off + 4] = bytes(color)
    _rgba_to_png(path, width, height, pixels)


def slug(text: str, fallback: str = "item") -> str:
    out = []
    for ch in str(text).strip().lower():
        low = ch
        if low in TRANSLIT:
            out.append(TRANSLIT[low])
        elif ch.isalnum():
            out.append(ch)
        elif ch in " -_().[]":
            out.append("_")
    res = "".join(out)
    while "__" in res:
        res = res.replace("__", "_")
    res = res.strip("_")
    return res or fallback


def sha256(path: str) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def lua_str(text: str) -> str:
    return '"' + str(text).replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n") + '"'


def lua_num(value) -> str:
    try:
        f = float(value)
    except (TypeError, ValueError):
        return "0"
    if f == int(f):
        return str(int(f))
    return repr(f)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", default="original")
    ap.add_argument("--out", default="assets")
    args = ap.parse_args()

    project = load_project(args.src)
    out = args.out
    index: dict = {"project": project.name, "stage": [project.stage_w, project.stage_h],
                   "orientation": project.orientation,
                   "scenes": [], "missing": [], "duplicates": {}}
    seen_hash: dict[str, str] = {}
    total_png = total_mp3 = 0
    bytes_png = bytes_mp3 = 0

    os.makedirs(os.path.join(out, "scenes"), exist_ok=True)
    os.makedirs(os.path.join(out, "icons"), exist_ok=True)

    proj_icon = os.path.join(args.src, "icon.png")
    if os.path.exists(proj_icon):
        shutil.copyfile(proj_icon, os.path.join(out, "icon.png"))

    lua_scenes: list[str] = []
    for scene in project.scenes:
        sslug = f"{scene.scene_id:02d}_{slug(scene.name, 'scene')}"
        sdir = os.path.join(out, "scenes", sslug)
        icon_src = os.path.join(scene.path, "icon.png")
        if os.path.exists(icon_src):
            shutil.copyfile(icon_src, os.path.join(out, "icons", sslug + ".png"))

        lua_objects: list[str] = []
        scene_entry = {"id": scene.scene_id, "name": scene.name, "slug": sslug,
                       "objects": [], "functions": [
                           {"id": fid, "name": fname, "arity": arity}
                           for fid, fname, arity in scene.functions]}

        for obj in scene.objects:
            oslug = f"{obj.object_id:03d}_{slug(obj.name, 'object')}"
            odir = os.path.join(sdir, oslug)
            img_dir = os.path.join(odir, "images")
            snd_dir = os.path.join(odir, "sounds")
            os.makedirs(img_dir, exist_ok=True)
            os.makedirs(snd_dir, exist_ok=True)

            obj_entry = {"id": obj.object_id, "name": obj.name, "slug": oslug,
                         "rotation": obj.rotation, "size": obj.size,
                         "images": [], "sounds": [], "default_image": None}
            lua_images: list[str] = []
            lua_sounds: list[str] = []

            for iname, iid in obj.images:
                src = os.path.join(obj.path, f"image_{iid}.png")
                if not os.path.exists(src):
                    index["missing"].append(os.path.relpath(src, args.src))
                    continue
                digest = sha256(src)
                dst_name = f"{iid:04d}_{slug(iname, 'image')}.png"
                dst = os.path.join(img_dir, dst_name)
                shutil.copyfile(src, dst)
                rel = os.path.relpath(dst, out).replace(os.sep, "/")
                patch_image(dst, rel)
                size = os.path.getsize(dst)
                total_png += 1
                bytes_png += size
                if digest in seen_hash:
                    index["duplicates"].setdefault(digest, [seen_hash[digest]]).append(rel)
                else:
                    seen_hash[digest] = rel
                entry = {"id": iid, "name": iname, "path": rel, "bytes": size,
                         "sha256": digest}
                obj_entry["images"].append(entry)
                lua_images.append(
                    "{id=%d, name=%s, path=%s}" % (iid, lua_str(iname), lua_str(rel)))
                if obj.default_image and os.path.basename(obj.default_image) == f"image_{iid}.png":
                    obj_entry["default_image"] = rel

            for sname, sid in obj.sounds:
                src = os.path.join(obj.path, f"sound_{sid}.mp3")
                if not os.path.exists(src):
                    index["missing"].append(os.path.relpath(src, args.src))
                    continue
                digest = sha256(src)
                dst_name = f"{sid:04d}_{slug(sname, 'sound')}.mp3"
                dst = os.path.join(snd_dir, dst_name)
                shutil.copyfile(src, dst)
                rel = os.path.relpath(dst, out).replace(os.sep, "/")
                size = os.path.getsize(dst)
                total_mp3 += 1
                bytes_mp3 += size
                if digest in seen_hash:
                    index["duplicates"].setdefault(digest, [seen_hash[digest]]).append(rel)
                else:
                    seen_hash[digest] = rel
                entry = {"id": sid, "name": sname, "path": rel, "bytes": size,
                         "sha256": digest}
                obj_entry["sounds"].append(entry)
                lua_sounds.append(
                    "{id=%d, name=%s, path=%s}" % (sid, lua_str(sname), lua_str(rel)))

            if obj_entry["default_image"] is None and obj_entry["images"]:
                obj_entry["default_image"] = obj_entry["images"][0]["path"]

            obj_entry["vars"] = [{"id": vid, "name": vname} for vid, vname, *_ in obj.variables]
            scene_entry["objects"].append(obj_entry)
            lua_objects.append(
                "{id=%d, name=%s, slug=%s, rotation=%s, size=%s, image=%s, images={%s}, sounds={%s}, vars={%s}}"
                % (obj.object_id, lua_str(obj.name), lua_str(oslug),
                   lua_num(obj.rotation), lua_num(obj.size),
                   lua_str(obj_entry["default_image"] or ""),
                   ",".join(lua_images), ",".join(lua_sounds),
                   ",".join("{id=%d,name=%s}" % (vid, lua_str(vname))
                            for vid, vname, *_ in obj.variables)))

        index["scenes"].append(scene_entry)
        lua_scenes.append(
            "{id=%d, name=%s, slug=%s, objects={%s}, functions={%s}}"
            % (scene.scene_id, lua_str(scene.name), lua_str(sslug),
               ",".join(lua_objects),
               ",".join("{id=%d, name=%s, arity=%d}" % (fid, lua_str(fname), arity)
                        for fid, fname, arity in scene.functions)))

    with open(os.path.join(out, "index.json"), "w", encoding="utf-8") as fh:
        json.dump(index, fh, ensure_ascii=False, indent=1)

    with open(os.path.join(out, "project.lua"), "w", encoding="utf-8") as fh:
        fh.write("-- Сгенерировано tools/extract_assets.py из проекта Pocket Code.\n")
        fh.write("-- Структура проекта для движка cb-classic (C++/Lua).\n")
        fh.write("return {\n")
        fh.write("  name = %s,\n" % lua_str(project.name))
        fh.write("  stage = { w = %d, h = %d },\n" % (project.stage_w, project.stage_h))
        fh.write("  orientation = %s,\n" % lua_str(project.orientation))
        fh.write("  variables = {%s},\n" % ",".join(
            "{id=%d, name=%s}" % (vid, lua_str(vname)) for vid, vname, *_ in project.variables))
        fh.write("  arrays = {%s},\n" % ",".join(
            "{id=%d, name=%s}" % (aid, lua_str(aname)) for aid, aname, *_ in project.arrays))
        fh.write("  scenes = {\n    %s\n  },\n" % ",\n    ".join(lua_scenes))
        fh.write("}\n")

    dup_count = sum(len(v) - 1 for v in index["duplicates"].values())
    with open(os.path.join(out, "README.md"), "w", encoding="utf-8") as fh:
        fh.write("# Ассеты проекта\n\n")
        fh.write("Извлечено из `original/` скриптом `tools/extract_assets.py`.\n\n")
        fh.write("| что | штук | размер |\n|---|---|---|\n")
        fh.write(f"| картинки PNG | {total_png} | {bytes_png / 1e6:.1f} МБ |\n")
        fh.write(f"| звуки MP3 | {total_mp3} | {bytes_mp3 / 1e6:.1f} МБ |\n")
        fh.write(f"| сцен | {len(project.scenes)} | — |\n")
        fh.write(f"| объектов | {sum(len(s.objects) for s in project.scenes)} | — |\n")
        fh.write(f"| дубликатов по содержимому | {dup_count} | — |\n\n")
        fh.write("Имена файлов транслитерированы (`<id>_<имя>.png`), исходные имена "
                 "лежат в `index.json` и `project.lua`.\n")
        if index["missing"]:
            fh.write("\n## Не найдено в архиве\n\n")
            for m in index["missing"]:
                fh.write(f"- `{m}`\n")
        fh.write("\n## Крупные файлы, не вошедшие в репозиторий\n\n")
        fh.write("Два mp3 по 93 МБ (`scene_26/object_21/sound_8.mp3`, `sound_9.mp3`) — "
                 "байт-в-байт один и тот же файл; скачать его можно из релиза "
                 "`YOU_DON'T_NEED_IT` (см. `ops/excluded-large-files.txt`).\n")

    print(f"PNG: {total_png} ({bytes_png/1e6:.1f} MB), MP3: {total_mp3} ({bytes_mp3/1e6:.1f} MB), "
          f"duplicates: {dup_count}, missing: {len(index['missing'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
