#!/usr/bin/env python3
"""Иконки приложения из assets/icon.png (иконка проекта).

Исходник один — картинка, которая лежит в репозитории (480x480, кот на синем
круге). Скрипт делает из неё всё, что нужно Android:

    android/app/src/main/res/mipmap-*/ic_launcher.png          обычная иконка
    android/app/src/main/res/mipmap-*/ic_launcher_round.png    круглая
    android/app/src/main/res/mipmap-*/ic_launcher_foreground.png  слой адаптивной
    android/app/src/main/res/mipmap-anydpi-v26/ic_launcher*.xml   адаптивная иконка
    android/app/src/main/res/values/ic_launcher_background.xml    её фон

    python3 tools/make_icon.py            # пересобрать иконки
    python3 tools/make_icon.py --check    # только проверить, что файлы на месте

PIL и ImageMagick в проекте нет, поэтому PNG читается и пишется своим кодом
(как в tests/check_shot.py): zlib + фильтры PNG, уменьшение — усреднением.
"""

import argparse
import math
import os
import struct
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SOURCE = os.path.join(ROOT, "assets", "icon.png")
RES = os.path.join(ROOT, "android", "app", "src", "main", "res")

# плотность -> (размер иконки, размер слоя адаптивной иконки: 108dp)
DENSITIES = {
    "mdpi": (48, 108),
    "hdpi": (72, 162),
    "xhdpi": (96, 216),
    "xxhdpi": (144, 324),
    "xxxhdpi": (192, 432),
}

# фон адаптивной иконки — серо-синий круг исходника: на нём чёрный кот
# читается лучше, чем на тёмно-синем фоне (как в оригинальной иконке)
BACKGROUND = (116, 124, 156)


# --- PNG ------------------------------------------------------------------

def read_png(path):
    data = open(path, "rb").read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise SystemExit(f"{path}: это не PNG")
    pos = 8
    width = height = 0
    ctype = 6
    idat = b""
    while pos + 8 <= len(data):
        (length,) = struct.unpack(">I", data[pos:pos + 4])
        kind = data[pos + 4:pos + 8]
        body = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b"IHDR":
            width, height, depth, ctype, _, _, interlace = struct.unpack(">IIBBBBB", body)
            if depth != 8 or interlace != 0 or ctype not in (2, 6):
                raise SystemExit(f"{path}: ждали PNG 8 бит RGBA/RGB без интерлейса")
        elif kind == b"IDAT":
            idat += body
        elif kind == b"IEND":
            break
    bpp = 4 if ctype == 6 else 3
    raw = zlib.decompress(idat)
    stride = width * bpp
    pixels = bytearray(width * height * 4)
    prev = bytearray(stride)
    off = 0
    for y in range(height):
        ftype = raw[off]
        line = bytearray(raw[off + 1:off + 1 + stride])
        off += 1 + stride
        for i in range(stride):
            a = line[i - bpp] if i >= bpp else 0
            b = prev[i]
            c = prev[i - bpp] if i >= bpp else 0
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
        prev = line
        for x in range(width):
            src = x * bpp
            dst = (y * width + x) * 4
            pixels[dst] = line[src]
            pixels[dst + 1] = line[src + 1]
            pixels[dst + 2] = line[src + 2]
            pixels[dst + 3] = line[src + 3] if bpp == 4 else 255
    return width, height, pixels


def write_png(path, width, height, pixels):
    raw = bytearray()
    stride = width * 4
    for y in range(height):
        raw.append(0)                      # фильтр 0: строки как есть
        raw += pixels[y * stride:(y + 1) * stride]

    def chunk(kind, body):
        return (struct.pack(">I", len(body)) + kind + body +
                struct.pack(">I", zlib.crc32(kind + body) & 0xFFFFFFFF))

    header = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    data = (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) +
            chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b""))
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as handle:
        handle.write(data)


# --- картинка -------------------------------------------------------------

def resize(width, height, pixels, new_w, new_h):
    """Уменьшение усреднением (оно же сглаживает края при вырезании маски)."""
    out = bytearray(new_w * new_h * 4)
    for y in range(new_h):
        y0 = y * height / new_h
        y1 = (y + 1) * height / new_h
        for x in range(new_w):
            x0 = x * width / new_w
            x1 = (x + 1) * width / new_w
            acc = [0.0, 0.0, 0.0, 0.0]
            weight = 0.0
            for sy in range(int(math.floor(y0)), min(height, max(int(math.floor(y0)) + 1, int(math.ceil(y1))))):
                cover_y = min(y1, sy + 1) - max(y0, sy)
                if cover_y <= 0:
                    continue
                for sx in range(int(math.floor(x0)), min(width, max(int(math.floor(x0)) + 1, int(math.ceil(x1))))):
                    cover_x = min(x1, sx + 1) - max(x0, sx)
                    if cover_x <= 0:
                        continue
                    w = cover_x * cover_y
                    src = (sy * width + sx) * 4
                    acc[0] += pixels[src] * w
                    acc[1] += pixels[src + 1] * w
                    acc[2] += pixels[src + 2] * w
                    acc[3] += pixels[src + 3] * w
                    weight += w
            dst = (y * new_w + x) * 4
            if weight <= 0.0:
                continue
            out[dst] = int(acc[0] / weight + 0.5)
            out[dst + 1] = int(acc[1] / weight + 0.5)
            out[dst + 2] = int(acc[2] / weight + 0.5)
            out[dst + 3] = int(acc[3] / weight + 0.5)
    return out


def mascot_mask(width, height, pixels):
    """Вырезать кота: он почти чёрный (нейтральный), глаза — белые.

    Фон (#1C233E) и круг (#747C9C) синеватые, поэтому по разнице b-r их легко
    отличить от чёрного силуэта.
    """
    out = bytearray(len(pixels))
    bbox = [width, height, 0, 0]
    for y in range(height):
        for x in range(width):
            i = (y * width + x) * 4
            r, g, b = pixels[i], pixels[i + 1], pixels[i + 2]
            darkest = min(r, g, b)
            brightest = max(r, g, b)
            dark_neutral = brightest <= 40 and (b - r) <= 25
            eye = darkest >= 200
            if not (dark_neutral or eye):
                continue
            out[i] = r
            out[i + 1] = g
            out[i + 2] = b
            out[i + 3] = 255
            bbox[0] = min(bbox[0], x)
            bbox[1] = min(bbox[1], y)
            bbox[2] = max(bbox[2], x)
            bbox[3] = max(bbox[3], y)
    return out, bbox


def crop(width, height, pixels, box):
    left, top, right, bottom = box
    w = right - left + 1
    h = bottom - top + 1
    out = bytearray(w * h * 4)
    for y in range(h):
        src = ((top + y) * width + left) * 4
        out[y * w * 4:(y + 1) * w * 4] = pixels[src:src + w * 4]
    return w, h, out


def paste_centered(canvas_w, canvas_h, canvas, sprite_w, sprite_h, sprite):
    ox = (canvas_w - sprite_w) // 2
    oy = (canvas_h - sprite_h) // 2
    for y in range(sprite_h):
        for x in range(sprite_w):
            src = (y * sprite_w + x) * 4
            alpha = sprite[src + 3]
            if alpha == 0:
                continue
            dst = ((oy + y) * canvas_w + (ox + x)) * 4
            # накладываем на прозрачный слой (спрайт уже с мягкими краями)
            if alpha == 255:
                canvas[dst:dst + 4] = sprite[src:src + 4]
                continue
            inv = 255 - alpha
            canvas[dst] = sprite[src]
            canvas[dst + 1] = sprite[src + 1]
            canvas[dst + 2] = sprite[src + 2]
            canvas[dst + 3] = max(canvas[dst + 3], alpha)
            del inv


def round_mask(size, pixels):
    """Круглая иконка: то, что вышло за круг, становится прозрачным."""
    out = bytearray(pixels)
    center = (size - 1) / 2.0
    radius = size / 2.0
    for y in range(size):
        for x in range(size):
            dist = math.hypot(x - center, y - center)
            if dist <= radius:
                continue
            i = (y * size + x) * 4
            out[i + 3] = 0
    return out


def build():
    width, height, pixels = read_png(SOURCE)
    mask, bbox = mascot_mask(width, height, pixels)
    if bbox[2] <= bbox[0] or bbox[3] <= bbox[1]:
        raise SystemExit("не нашли кота на иконке")
    mascot_w, mascot_h, mascot = crop(width, height, mask, bbox)

    for density, (icon_size, adaptive_size) in DENSITIES.items():
        folder = os.path.join(RES, f"mipmap-{density}")
        square = resize(width, height, pixels, icon_size, icon_size)
        write_png(os.path.join(folder, "ic_launcher.png"), icon_size, icon_size, square)
        write_png(os.path.join(folder, "ic_launcher_round.png"), icon_size, icon_size,
                  round_mask(icon_size, square))

        # Адаптивная иконка: спрайт занимает ~60% холста 108dp — так он не
        # обрезается ни круглой, ни квадратной маской лаунчера.
        inner = int(adaptive_size * 0.60)
        scale = min(inner / mascot_w, inner / mascot_h)
        sprite_w = max(1, int(mascot_w * scale))
        sprite_h = max(1, int(mascot_h * scale))
        sprite = resize(mascot_w, mascot_h, mascot, sprite_w, sprite_h)
        canvas = bytearray(adaptive_size * adaptive_size * 4)
        paste_centered(adaptive_size, adaptive_size, canvas, sprite_w, sprite_h, sprite)
        write_png(os.path.join(folder, "ic_launcher_foreground.png"),
                  adaptive_size, adaptive_size, canvas)

    anydpi = os.path.join(RES, "mipmap-anydpi-v26")
    os.makedirs(anydpi, exist_ok=True)
    layers = """<?xml version="1.0" encoding="utf-8"?>
<adaptive-icon xmlns:android="http://schemas.android.com/apk/res/android">
    <background android:drawable="@color/ic_launcher_background"/>
    <foreground android:drawable="@mipmap/ic_launcher_foreground"/>
</adaptive-icon>
"""
    for name in ("ic_launcher.xml", "ic_launcher_round.xml"):
        with open(os.path.join(anydpi, name), "w", encoding="utf-8") as handle:
            handle.write(layers)

    values = os.path.join(RES, "values")
    os.makedirs(values, exist_ok=True)
    with open(os.path.join(values, "ic_launcher_background.xml"), "w", encoding="utf-8") as handle:
        handle.write(
            '<?xml version="1.0" encoding="utf-8"?>\n<resources>\n'
            f'    <color name="ic_launcher_background">#{BACKGROUND[0]:02X}{BACKGROUND[1]:02X}{BACKGROUND[2]:02X}</color>\n'
            "</resources>\n")
    print(f"иконки собраны из {os.path.relpath(SOURCE, ROOT)}")


def check():
    missing = []
    for density, (icon_size, adaptive_size) in DENSITIES.items():
        folder = os.path.join(RES, f"mipmap-{density}")
        for name, size in (("ic_launcher.png", icon_size),
                           ("ic_launcher_round.png", icon_size),
                           ("ic_launcher_foreground.png", adaptive_size)):
            path = os.path.join(folder, name)
            if not os.path.exists(path):
                missing.append(os.path.relpath(path, ROOT))
                continue
            w, h, _ = read_png(path)
            if (w, h) != (size, size):
                missing.append(f"{os.path.relpath(path, ROOT)}: {w}x{h}, ждали {size}x{size}")
    for name in ("mipmap-anydpi-v26/ic_launcher.xml", "mipmap-anydpi-v26/ic_launcher_round.xml",
                 "values/ic_launcher_background.xml"):
        if not os.path.exists(os.path.join(RES, name)):
            missing.append(name)
    if missing:
        print("нет иконок:\n  " + "\n  ".join(missing))
        return 1
    print("иконки на месте")
    return 0


def main(argv):
    parser = argparse.ArgumentParser(description="Иконки приложения из assets/icon.png")
    parser.add_argument("--check", action="store_true", help="только проверить файлы")
    args = parser.parse_args(argv[1:])
    if args.check:
        return check()
    build()
    return check()


if __name__ == "__main__":
    sys.exit(main(sys.argv))
