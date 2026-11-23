#!/usr/bin/env python3
"""Собирает docs/BRICKS.md — таблицу покрытия блоков Pocket Code.

    python3 tools/report.py

Считает, какие блоки/выражения реально встречаются в проекте, во что они
превращаются в Lua и что из этого — полноценная реализация, а что заглушка.
"""

from __future__ import annotations

import collections
import glob
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from catrobat import load_project, split_scripts  # noqa: E402

# блок -> (Lua-вызов, статус)
BRICKS = {
    "start": ("триггер сцены", "готово"),
    "whenTheTruth": ("опрос условия каждый кадр", "готово"),
    "onTouchObject": ("тап по объекту", "готово"),
    "touchObject": ("касание объекта", "готово"),
    "startClone": ("триггер клона", "готово"),
    "touchBack": ("кнопка «назад»", "готово"),
    "if": ("if/else", "готово"),
    "else": ("встроено в if", "готово"),
    "endIf": ("структурный: закрывает if", "готово"),
    "endRepeat": ("структурный: закрывает repeat / repeatIsTrue", "готово"),
    "endCycleForever": ("структурный: закрывает forever", "готово"),
    "repeat": ("for + E.frame()", "готово"),
    "cycleForever": ("while true + E.frame()", "готово"),
    "repeatIsTrue": ("while условие + E.frame()", "готово"),
    "wait": ("E.wait", "готово"),
    "waitIfTrue2": ("E.wait_until", "готово"),
    "setVariable": ("E.set_gv / E.set_lv", "готово"),
    "editVariable": ("E.add_gv / E.add_lv", "готово"),
    "saveVariable": ("E.store_var (локальное хранилище)", "готово"),
    "readVariable": ("E.load_var", "готово"),
    "addElementArray": ("E.array_add", "готово"),
    "setPosition": ("E.set_pos", "готово"),
    "setPositionX": ("E.set_x", "готово"),
    "setPositionY": ("E.set_y", "готово"),
    "editPositionX": ("E.add_x", "готово"),
    "editPositionY": ("E.add_y", "готово"),
    "goTo": ("E.go_to", "готово"),
    "goSteps": ("E.go_steps", "готово"),
    "setRotate": ("E.set_rot", "готово"),
    "editRotateRight": ("E.add_rot", "готово"),
    "setRotateToObject": ("E.face", "готово"),
    "transitionPosition2": ("E.transition (плавный ход)", "готово"),
    "setSize": ("E.set_size", "готово"),
    "editSize": ("E.add_size", "готово"),
    "setAlpha": ("E.set_alpha", "готово"),
    "editAlpha": ("E.add_alpha", "готово"),
    "setBrightness": ("E.set_bright", "готово"),
    "show": ("E.show", "готово"),
    "hide": ("E.hide", "готово"),
    "setLayer": ("E.set_layer", "готово"),
    "setImageToName": ("E.set_image_by_id", "готово"),
    "setImageToId": ("E.set_image_by_id", "готово"),
    "nextImage": ("E.next_image", "готово"),
    "setBackgroundColorHex": ("E.set_bg_color", "готово"),
    "playSound": ("E.play_sound (журнал)", "частично: без MP3-декодера"),
    "playSoundAndWait": ("E.play_sound_wait", "частично: без MP3-декодера"),
    "runScene": ("E.run_scene", "готово"),
    "clone": ("E.clone", "готово"),
    "deleteClone": ("E.delete_clone", "готово"),
    "broadcastFunction": ("E.broadcast", "готово (получателей в проекте нет)"),
    "broadcastFun>object": ("E.broadcast_to_object", "готово (получателей нет)"),
    "broadcastFun>allObjects": ("E.broadcast", "готово (получателей нет)"),
    "function": ("E.call_function", "готово (получателей нет)"),
    "showToast": ("E.toast", "готово"),
    "openLink": ("E.open_link (журнал)", "готово"),
    "exitGame": ("E.exit_game", "готово"),
    "ask": ("E.ask", "готово: поле ввода + ожидание ответа"),
    "createTextField": ("E.text_create", "готово: ввод с клавиатуры"),
    "deleteTextField": ("E.text_delete", "готово"),
    "setPositionTextField": ("E.text_set('pos')", "готово"),
    "setAlignTextField": ("E.text_set('align')", "готово"),
    "placeholderTextField": ("E.text_set('placeholder')", "готово"),
    "showVariable2": ("E.text_show_var", "готово"),
    "fbSetValue": ("E.fb_set (локальная эмуляция)", "готово"),
    "fbGetValue": ("E.fb_get (локальная эмуляция)", "готово"),
    "removeAdaptiveSizeDevice": ("E.adaptive_off", "готово"),
    "removeObjectCamera": ("E.camera_off", "готово"),
    "focusCameraToObject": ("E.camera_focus (журнал)", "заглушка"),
    "setGravityAllObjects": ("E.set_gravity (журнал)", "заглушка: нет физики"),
    "addBody": ("E.add_body (журнал)", "заглушка: нет физики"),
    "isSensor": ("E.set_sensor (журнал)", "заглушка: нет физики"),
    "setQuareWHHitbox": ("E.set_hitbox (журнал)", "заглушка: нет физики"),
    "createRadialParticle": ("E.particles_create", "заглушка: нет частиц"),
    "setPositionParticle": ("E.particles_move", "заглушка: нет частиц"),
}

EXPRESSIONS = {
    "number": "числовой литерал (склейка кусков: 3,5,0 → 350)",
    "text": "строковый литерал",
    "globalVariable": "E.gv(id)",
    "localVariable": "E.lv(o, id)",
    "globalArray": "E.array(id)",
    "function: =, ≠, <, >, ≤, ≥": "E.op_eq / op_ne / op_lt / op_gt / op_le / op_ge",
    "function: +, -, ×, ÷": "E.op_add / op_sub / op_mul / op_div",
    "function: and, or, not": "E.truthy(...)",
    "function: positionX / positionY": "E.pos_x(o) / E.pos_y(o)",
    "function: rotation / size / brightness / alpha": "геттеры спрайта",
    "function: numberImage / nameImage": "номер и имя текущей картинки",
    "function: touchDisplayX/Y/Id": "координаты касания",
    "function: touchesObject2(id)": "E.touching_object(o, id)",
    "function: random(a,b)": "E.random",
    "function: connect2(a,b,c)": "E.fb_key (путь в локальном хранилище)",
    "ref: scenes/objects/images/sounds": "числовой id из проекта",
    "alignText / GL / typeBody / onOrOff": "строковый/служебный токен",
}


def main() -> int:
    project = load_project("original")
    counts = collections.Counter()
    for scene in project.scenes:
        for obj in scene.objects:
            for script in split_scripts(obj.scripts):
                for brick in script["body"]:
                    counts[brick["name"]] += 1
                counts[f"trigger:{script['trigger']}"] += 1

    lines = [
        "# Покрытие блоков Pocket Code",
        "",
        "Сгенерировано `tools/report.py` по данным `original/`.",
        "",
        f"Блоков в проекте: **{sum(v for k, v in counts.items() if not k.startswith('trigger:'))}**, "
        f"различных видов: **{len(counts)}**.",
        "",
        "## Блоки",
        "",
        "| блок | раз | во что превращается | статус |",
        "|---|---:|---|---|",
    ]
    unknown = []
    for name, count in sorted(counts.items(), key=lambda kv: -kv[1]):
        if name.startswith("trigger:"):
            continue
        call, status = BRICKS.get(name, ("—", "НЕ ОБРАБОТАН"))
        if status == "НЕ ОБРАБОТАН":
            unknown.append(name)
        lines.append(f"| `{name}` | {count} | {call} | {status} |")

    lines += [
        "",
        "## Выражения и сенсоры",
        "",
        "| конструкция | реализация |",
        "|---|---|",
    ]
    for item, mapping in EXPRESSIONS.items():
        lines.append(f"| {item} | {mapping} |")

    lines += [
        "",
        "## Итог",
        "",
        f"* блоков без реализации: **{len(unknown)}**"
        + (f" ({', '.join(unknown)})" if unknown else ""),
        "* заглушки (пишут в журнал, но не моделируют физику/частицы): "
        "`addBody`, `isSensor`, `setGravityAllObjects`, `setQuareWHHitbox`, "
        "`createRadialParticle`, `setPositionParticle`, `focusCameraToObject`.",
        "* звук: выбор и запуск звука перенесены, воспроизведения нет (нет MP3-декодера).",
        "",
    ]

    os.makedirs("docs", exist_ok=True)
    with open("docs/BRICKS.md", "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines))
    print(f"docs/BRICKS.md: блоков {len(counts)}, без реализации {len(unknown)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
