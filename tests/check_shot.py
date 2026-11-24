#!/usr/bin/env python3
"""Проверки картинки кадра по пикселям (без PIL и ImageMagick).

Свои PNG-кадры движок пишет как RGBA 8 бит без интерлейса
(см. cb_png_write в src/c/png.c), поэтому файл разбирается штатным zlib.

    python3 tests/check_shot.py toast  frame.png
    python3 tests/check_shot.py field  frame.png 400 -100
    python3 tests/check_shot.py scrolled before.png after.png
"""

import struct
import sys
import zlib


def read_png(path):
    data = open(path, "rb").read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise SystemExit(f"{path}: это не PNG")
    pos = 8
    width = height = 0
    idat = b""
    while pos + 8 <= len(data):
        (length,) = struct.unpack(">I", data[pos:pos + 4])
        kind = data[pos + 4:pos + 8]
        body = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b"IHDR":
            width, height, depth, ctype, _, _, interlace = struct.unpack(">IIBBBBB", body)
            if depth != 8 or ctype != 6 or interlace != 0:
                raise SystemExit(f"{path}: ждали RGBA 8 бит без интерлейса")
        elif kind == b"IDAT":
            idat += body
        elif kind == b"IEND":
            break
    raw = zlib.decompress(idat)
    stride = width * 4
    rows = []
    prev = bytearray(stride)
    off = 0
    for _ in range(height):
        ftype = raw[off]
        line = bytearray(raw[off + 1:off + 1 + stride])
        off += 1 + stride
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
        rows.append(line)
        prev = line
    return width, height, rows


def pixel(rows, x, y):
    line = rows[y]
    return line[x * 4], line[x * 4 + 1], line[x * 4 + 2], line[x * 4 + 3]


def check_toast(path):
    """Внизу по центру — тёмно-серая (#323232) скруглённая плашка с белым текстом."""
    width, height, rows = read_png(path)
    band = range(max(0, height - 150), height)
    pill_rows = {}
    for y in band:
        xs = []
        for x in range(width):
            r, g, b, a = pixel(rows, x, y)
            # ровно цвет toast'а, допуск на сглаживание краёв
            if a > 220 and abs(r - 50) <= 8 and abs(g - 50) <= 8 and abs(b - 50) <= 8:
                xs.append(x)
        if len(xs) >= 100:
            pill_rows[y] = (min(xs), max(xs))
    if not pill_rows:
        return "не нашли тёмную плашку уведомления внизу кадра"
    top, bottom = min(pill_rows), max(pill_rows)
    height_px = bottom - top + 1
    if not 40 <= height_px <= 90:
        return f"высота плашки не похожа на toast: {height_px} px"
    left = min(v[0] for v in pill_rows.values())
    right = max(v[1] for v in pill_rows.values())
    width_px = right - left + 1
    if width_px < 140:
        return f"плашка уведомления слишком узкая: {width_px} px"
    center = (left + right) // 2
    if abs(center - width // 2) > width * 0.1:
        return f"плашка не по центру: центр {center}, кадр {width // 2}"
    if bottom > height - 8:
        return "плашка прилипла к самому низу кадра"
    # скруглённые углы: верхний левый угол плашки не должен быть цветом плашки
    r, g, b, a = pixel(rows, left, top)
    if a > 220 and abs(r - 50) <= 8 and abs(g - 50) <= 8 and abs(b - 50) <= 8:
        return "углы плашки не скруглены"
    # белый текст внутри плашки
    white = 0
    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            r, g, b, a = pixel(rows, x, y)
            if a > 200 and r > 220 and g > 220 and b > 220:
                white += 1
    if white < 60:
        return "внутри плашки нет белого текста"
    return None


def check_field(path, x, y):
    """Поле ввода в сценических координатах — сплошная чёрная подложка."""
    width, height, rows = read_png(path)
    px = int(round(x + width / 2.0))
    py = int(round(height / 2.0 - y))
    if not (0 <= px < width and 0 <= py < height):
        return f"точка {x},{y} вне кадра"
    r, g, b, a = pixel(rows, px, py)
    if a < 200 or r > 16 or g > 16 or b > 16:
        return f"фон поля не чёрный: rgba={r},{g},{b},{a}"
    return None


def check_scrolled(before, after):
    """Список кубов листается стрелками: содержимое съехало вниз на шаг 80,
    а сами стрелки и «Назад» стоят на месте."""
    w1, h1, a = read_png(before)
    w2, h2, b = read_png(after)
    if (w1, h1) != (w2, h2):
        return "кадры разного размера"
    # область списка (левая половина кадра) — по ней ищем сдвиг
    x0, x1 = int(w1 * 0.08), int(w1 * 0.52)
    best_shift, best_err = 0, None
    for shift in range(-160, 161, 4):
        err = 0
        for y in range(max(0, 40 - shift), min(h1, h1 - 40 - shift), 4):
            for x in range(x0, x1, 4):
                pa = pixel(a, x, y)
                pb = pixel(b, x, y + shift)
                err += abs(pa[0] - pb[0]) + abs(pa[1] - pb[1]) + abs(pa[2] - pb[2])
        if best_err is None or err < best_err:
            best_shift, best_err = shift, err
    if abs(best_shift - 80) > 8:
        return f"список не съехал на шаг прокрутки: сдвиг {best_shift} px, ждём 80"
    # стрелки и «Назад» не должны двигаться вместе со списком
    for cx, cy in ((int(w1 * 0.587), 350), (int(w1 * 0.587), 490), (int(w1 * 0.77), 230)):
        for y in range(cy - 40, cy + 40, 2):
            for x in range(cx - 40, cx + 40, 2):
                if pixel(a, x, y) != pixel(b, x, y):
                    return f"кнопка управления уехала вместе со списком в ({x},{y})"
    return None


def main(argv):
    if len(argv) < 3:
        raise SystemExit(__doc__)
    kind, path = argv[1], argv[2]
    if kind == "toast":
        problem = check_toast(path)
    elif kind == "field" and len(argv) >= 5:
        problem = check_field(path, float(argv[3]), float(argv[4]))
    elif kind == "scrolled" and len(argv) >= 4:
        problem = check_scrolled(path, argv[3])
    else:
        raise SystemExit(__doc__)
    if problem:
        print(problem)
        return 1
    print("OK")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
