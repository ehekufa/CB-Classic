#!/usr/bin/env python3
"""Конвертер скриптов Pocket Code (Catrobat) в Lua.

    python3 tools/catrobat2lua.py --src original --out generated

Каждый объект превращается в самостоятельный Lua-модуль:

    generated/lua/scenes/<scene>/obj_<id>.lua

Модуль возвращает таблицу с триггерами и телами скриптов. Тела — обычный Lua-код,
который вызывает рантайм движка (`src/lua/runtime/*.lua`). Это «порт», а не эмуляция
исходного формата: результат можно читать и править руками.

Формулы разбираются `tools/catrobat.py` в AST и печатаются как Lua-выражения
(`E.op_add`, `E.gv`, ...), поэтому сравнения/склейка строк ведут себя как в Pocket Code.
"""

from __future__ import annotations

import argparse
import json
import os
import sys
from typing import Any

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from catrobat import Expr, Project, SceneObject, load_project, parse_formula, split_scripts  # noqa: E402

# --------------------------------------------------------------------------------------
# выражения
# --------------------------------------------------------------------------------------

SENSORS = {
    "positionX": "E.pos_x(o)",
    "positionY": "E.pos_y(o)",
    "rotation": "E.rot(o)",
    "size": "E.size(o)",
    "brightness": "E.bright(o)",
    "alpha": "E.alpha(o)",
    "layer": "E.layer(o)",
    "visible": "E.visible(o)",
    "name": "E.obj_name(o)",
    "objectName": "E.obj_name(o)",
    "sceneName": "E.scene_name()",
    "numberImage": "E.image_number(o)",
    "numberSounds": "E.sound_count(o)",
    "nameImage": "E.image_name(o)",
    "time": "E.time_ms()",
    "tSecond": "E.clock_second()",
    "tMinute": "E.clock_minute()",
    "tHour": "E.clock_hour()",
    "tDay": "E.clock_day()",
    "tMonth": "E.clock_month()",
    "tYear": "E.clock_year()",
    "touchDisplayX": "E.touch_x()",
    "touchDisplayY": "E.touch_y()",
    "touchDisplayId": "E.touch_id()",
    "touchDisplay": "E.touch_active()",          # «экран нажат» (waitIfTrue2)
    "displayActualWidth": "E.display_width()",   # ширина экрана устройства
    "displayActualHeight": "E.display_height()", # высота экрана устройства
    "touchesFinger": "E.touch_active()",
    "loudness": "E.loudness()",
    "globalVolume": "E.volume()",
    "touch": "E.touch_active()",
    "countTouchDisplay": "E.touch_count()",
    "direction": "E.rot(o)",
    "transparency": "E.op_sub(100, E.alpha(o))",
}

MATH = {
    "abs": "math.abs", "ceil": "math.ceil", "floor": "math.floor", "round": "E.round",
    "sqrt": "math.sqrt", "sin": "math.sin", "cos": "math.cos", "tan": "math.tan",
    "asin": "math.asin", "acos": "math.acos", "atan": "math.atan",
    "ln": "math.log", "log": "E.log10", "exp": "math.exp", "mod": "E.mod",
    "max": "math.max", "min": "math.min",
    "truncate": "E.truncate",
}

BINARY = {"add": "E.op_add", "sub": "E.op_sub", "mul": "E.op_mul", "div": "E.op_div"}
COMPARE = {"eq": "E.op_eq", "ne": "E.op_ne", "lt": "E.op_lt", "gt": "E.op_gt",
           "le": "E.op_le", "ge": "E.op_ge"}


def num_lit(value: float) -> str:
    if value == int(value):
        return str(int(value))
    return repr(float(value))


class Unsupported:
    """Сборщик неподдержанных конструкций для отчёта."""

    def __init__(self) -> None:
        self.bricks: dict[str, int] = {}
        self.expressions: dict[str, int] = {}

    def brick(self, name: str) -> None:
        self.bricks[name] = self.bricks.get(name, 0) + 1

    def expr(self, name: str) -> None:
        self.expressions[name] = self.expressions.get(name, 0) + 1


def gen_expr(node: Expr, unsup: Unsupported) -> str:
    k = node.kind
    if k == "num":
        return num_lit(float(node.value or 0))
    if k == "str":
        return json.dumps(str(node.value), ensure_ascii=False)
    if k == "bool":
        return "true" if node.value else "false"
    if k == "gvar":
        return f"E.gv({node.value})"
    if k == "lvar":
        return f"E.lv(o, {node.value})"
    if k == "array":
        return f"E.array({node.value})"
    if k == "ref":
        kind, rid = node.value
        return str(rid)
    if k in ("add", "sub", "mul", "div"):
        return f"{BINARY[k]}({gen_expr(node.args[0], unsup)}, {gen_expr(node.args[1], unsup)})"
    if k in COMPARE:
        return f"{COMPARE[k]}({gen_expr(node.args[0], unsup)}, {gen_expr(node.args[1], unsup)})"
    if k == "and":
        return (f"E.truthy({gen_expr(node.args[0], unsup)}) and "
                f"E.truthy({gen_expr(node.args[1], unsup)})")
    if k == "or":
        return (f"E.truthy({gen_expr(node.args[0], unsup)}) or "
                f"E.truthy({gen_expr(node.args[1], unsup)})")
    if k == "not":
        return f"(not E.truthy({gen_expr(node.args[0], unsup)}))"
    if k == "neg":
        return f"E.op_neg({gen_expr(node.args[0], unsup)})"
    if k == "join":
        return "E.op_join(" + ", ".join(gen_expr(a, unsup) for a in node.args) + ")"
    if k == "call":
        name = node.value
        args = [gen_expr(a, unsup) for a in node.args]
        if name in SENSORS and not args:
            return SENSORS[name]
        if name in MATH:
            return f"{MATH[name]}({', '.join(args)})"
        if name == "random":
            return f"E.random({', '.join(args) if args else '0, 1'})"
        if name in ("join", "concatenate"):
            return "E.op_join(" + ", ".join(args) + ")"
        if name == "letter":
            return f"E.str_letter({', '.join(args)})"
        if name == "length":
            return f"E.str_length({', '.join(args)})"
        if name == "contains":
            return f"E.str_contains({', '.join(args)})"
        if name == "subText":
            return f"E.str_sub({', '.join(args)})"
        if name == "touchesObject2":
            return f"E.touching_object(o, {', '.join(args)})"
        if name == "touchDisplayXId":
            return f"E.touch_x_by_id({', '.join(args)})"
        if name == "touchDisplayYId":
            return f"E.touch_y_by_id({', '.join(args)})"
        if name in ("touchDisplayId", "touchDisplayIdId"):
            return f"E.touch_id_by_index({', '.join(args)})"
        if name == "arctangent2":
            return f"E.atan2deg({', '.join(args)})"
        if name == "containsArray":
            return f"E.array_contains({', '.join(args)})"
        if name == "goTo":
            return f"E.obj_pos_x({', '.join(args)})"
        if name in ("connect2", "connect"):
            return f"E.fb_key({', '.join(args)})"
        if name == "typeBody":
            return "E.body_type(o)"
        if name == "GL":
            return f"E.blend({', '.join(args)})"
        if name in ("object", "scene", "image", "sound"):
            return args[0] if args else "0"
        unsup.expr(name)
        return f"E.unknown_expr({json.dumps(name, ensure_ascii=False)})"
    if k == "unknown":
        unsup.expr(str(node.value))
        return f"E.unknown_expr({json.dumps(str(node.value), ensure_ascii=False)})"
    unsup.expr(k)
    return f"E.unknown_expr({json.dumps(k, ensure_ascii=False)})"


def formula(value: Any, unsup: Unsupported) -> str:
    return gen_expr(parse_formula(value), unsup)


# --------------------------------------------------------------------------------------
# блоки
# --------------------------------------------------------------------------------------

TRIGGERS = {
    "start": "start",
    "whenTheTruth": "when_truth",
    # В выгрузке Pocket Code пара brick'ов «кнопка» выглядит так:
    #   touchObject   — палец коснулся (подсветить кнопку)
    #   onTouchObject — палец отпущен (само нажатие: звук, переход, выстрел)
    # Если запускать оба на касание, прицел в игре гас сразу же, а попадание
    # считалось в момент нажатия, а не отпускания.
    "onTouchObject": "touch_up",
    "touchObject": "touch_down",
    "startClone": "clone",
    "touchBack": "back",
}


class BrickGen:
    def __init__(self, unsup: Unsupported):
        self.unsup = unsup

    # --- helpers -------------------------------------------------------------
    def target(self, arg: Any) -> str:
        """arg = ["globalVariable", id] | ["localVariable", id]"""
        if isinstance(arg, list) and arg and arg[0] == "globalVariable":
            return f"E.gv({arg[1]})", f"E.set_gv({arg[1]}, %s)"
        if isinstance(arg, list) and arg and arg[0] == "localVariable":
            return f"E.lv(o, {arg[1]})", f"E.set_lv(o, {arg[1]}, %s)"
        return formula(arg, self.unsup), "E.set_gv(0, %s)"

    def ref_id(self, arg: Any) -> str:
        if isinstance(arg, list) and len(arg) > 1 and arg[0] in (
                "scenes", "objects", "images", "sounds", "function", "globalArray"):
            return str(arg[1])
        if isinstance(arg, list) and len(arg) == 1 and arg[0] == "objects":
            return "-1"                              # ["objects"] = сам объект
        return formula(arg, self.unsup)

    def goto(self, arg: Any) -> str:
        if isinstance(arg, list) and len(arg) == 2 and arg[0] == "goTo":
            if isinstance(arg[1], str):
                return "E.go_to_touch(o)"
            return f"E.go_to(o, {arg[1]})"
        if isinstance(arg, list) and len(arg) == 1 and arg[0] == "objects":
            return "E.go_to_self(o)"
        return f"E.go_to(o, {self.ref_id(arg)})"

    def option(self, arg: Any) -> str:
        # вариант может прийти как ["onOrOff", "off"] или как [[...]] (обёртка)
        while isinstance(arg, list) and len(arg) == 1 and isinstance(arg[0], list):
            arg = arg[0]
        if isinstance(arg, list) and len(arg) > 1 and isinstance(arg[1], str):
            return json.dumps(arg[1], ensure_ascii=False)
        if isinstance(arg, list) and arg and isinstance(arg[0], str):
            return json.dumps(arg[0], ensure_ascii=False)
        return json.dumps(str(arg), ensure_ascii=False)

    # --- главный генератор ---------------------------------------------------
    def block(self, bricks: list[dict], depth: int) -> list[str]:
        """Рекурсивно печатает последовательность блоков; возвращает строки кода."""
        lines: list[str] = []
        pad = "  " * depth
        i = 0
        while i < len(bricks):
            brick = bricks[i]
            name = brick["name"]
            args = brick["args"] or []

            if name == "if":
                cond = formula(args[0], self.unsup) if args else "false"
                # ищем соответствующий else/endIf
                body, else_body, nxt = self._split_block(bricks, i + 1)
                lines.append(f"{pad}if E.truthy({cond}) then")
                lines.extend(self.block(body, depth + 1))
                if else_body is not None:
                    lines.append(f"{pad}else")
                    lines.extend(self.block(else_body, depth + 1))
                lines.append(f"{pad}end")
                i = nxt
                continue

            if name == "repeat":
                count = formula(args[0], self.unsup) if args else "0"
                body, _, nxt = self._split_block(bricks, i + 1)
                lines.append(f"{pad}for _ = 1, math.floor(E.num({count})) do")
                lines.extend(self.block(body, depth + 1))
                lines.append(f"{pad}  E.frame()")
                lines.append(f"{pad}end")
                i = nxt
                continue

            if name == "cycleForever":
                body, _, nxt = self._split_block(bricks, i + 1)
                # «повторять всегда { установить яркость N }» — не цикл, а способ
                # автора «держать» кнопку серой. Такой вечный поток перебивал
                # возврат яркости, и кнопка способности не перезаряжалась,
                # поэтому сворачиваем его в одну установку.
                if self._only_brightness(body):
                    lines.append(f"{pad}E.set_bright(o, {self._brightness_value(body[0])})")
                    i = nxt
                    continue
                # остальные «повторять всегда» уходят в отдельный поток: иначе
                # блоки, стоящие после него в том же скрипте, никогда не
                # выполнились бы (в выгрузке Pocket Code так и есть — на этом
                # ломался джойстик)
                lines.append(f"{pad}E.forever(function()")
                lines.extend(self.block(body, depth + 1))
                lines.append(f"{pad}end)")
                i = nxt
                continue

            if name == "repeatIsTrue":
                cond = formula(args[0], self.unsup) if args else "false"
                body, _, nxt = self._split_block(bricks, i + 1)
                lines.append(f"{pad}while E.truthy({cond}) do")
                lines.extend(self.block(body, depth + 1))
                lines.append(f"{pad}  E.frame()")
                lines.append(f"{pad}end")
                i = nxt
                continue

            if name in ("endIf", "endRepeat", "endCycleForever", "else"):
                # структурные блоки обрабатываются в _split_block
                i += 1
                continue

            stmt = self.simple(brick, pad)
            if stmt:
                lines.append(pad + stmt)
            i += 1
        return lines or [f"{pad}-- (пусто)"]

    @staticmethod
    def _only_brightness(body: list[dict]) -> bool:
        """Цикл «повторять всегда» из одних «установить яркость»."""
        return bool(body) and all(b.get("name") == "setBrightness" for b in body)

    def _brightness_value(self, brick: dict) -> str:
        args = brick.get("args") or []
        return formula(args[0], self.unsup) if args else "100"

    def _split_block(self, bricks: list[dict], start: int):
        """Возвращает (тело, тело_else|None, индекс_после_закрывающего_блока)."""
        opens = {"if", "cycleForever", "repeat", "repeatIsTrue"}
        closes = {"endIf", "endCycleForever", "endRepeat"}
        body: list[dict] = []
        else_body: list[dict] | None = None
        depth = 0
        i = start
        while i < len(bricks):
            brick = bricks[i]
            name = brick["name"]
            if name in opens:
                depth += 1
            elif name in closes:
                if depth == 0:
                    return body, else_body, i + 1
                depth -= 1
            elif name == "else" and depth == 0 and else_body is None:
                else_body = []
                i += 1
                continue
            if else_body is not None:
                else_body.append(brick)
            else:
                body.append(brick)
            i += 1
        return body, else_body, i

    # --- отдельные блоки -----------------------------------------------------
    def simple(self, brick: dict, pad: str) -> str | None:
        name = brick["name"]
        args = brick["args"] or []
        f = lambda idx: formula(args[idx], self.unsup) if idx < len(args) else "0"

        table = {
            "wait": lambda: f"E.wait(E.num({f(0)}))",
            "waitIfTrue2": lambda: f"E.wait_until(function() return E.truthy({f(0)}) end, o)",
            "show": lambda: "E.show(o)",
            "hide": lambda: "E.hide(o)",
            "setPosition": lambda: f"E.set_pos(o, {f(0)}, {f(1)})",
            "setPositionX": lambda: f"E.set_x(o, {f(0)})",
            "setPositionY": lambda: f"E.set_y(o, {f(0)})",
            "editPositionX": lambda: f"E.add_x(o, {f(0)})",
            "editPositionY": lambda: f"E.add_y(o, {f(0)})",
            "setSize": lambda: f"E.set_size(o, {f(0)})",
            "editSize": lambda: f"E.add_size(o, {f(0)})",
            "setRotate": lambda: f"E.set_rot(o, {f(0)})",
            "editRotateRight": lambda: f"E.add_rot(o, {f(0)})",
            "setRotateToObject": lambda: f"E.face(o, {self.ref_id(args[0])})",
            "goSteps": lambda: f"E.go_steps(o, {f(0)})",
            "goTo": lambda: self.goto(args[0]),
            "setBrightness": lambda: f"E.set_bright(o, {f(0)})",
            "setAlpha": lambda: f"E.set_alpha(o, {f(0)})",
            "editAlpha": lambda: f"E.add_alpha(o, {f(0)})",
            "setLayer": lambda: f"E.set_layer(o, {f(0)})",
            "setImageToName": lambda: f"E.set_image_by_id(o, {self.ref_id(args[0])})",
            "setImageToId": lambda: f"E.set_image_by_id(o, {f(0)})",
            "nextImage": lambda: "E.next_image(o)",
            "setBackgroundColorHex": lambda: f"E.set_bg_color({f(0)})",
            "playSound": lambda: f"E.play_sound(o, {self.ref_id(args[0])})",
            "playSoundAndWait": lambda: f"E.play_sound_wait(o, {self.ref_id(args[0])})",
            "showToast": lambda: f"E.toast({f(0)})",
            "openLink": lambda: f"E.open_link({f(0)})",
            "exitGame": lambda: "E.exit_game()",
            "runScene": lambda: f"E.run_scene({self.ref_id(args[0])}) return",
            "broadcastFunction": lambda: f"E.broadcast({self.ref_id(args[0])})",
            "broadcastFun>allObjects": lambda: f"E.broadcast({self.ref_id(args[0])})",
            "broadcastFun>object": lambda: (
                f"E.broadcast_to_object({self.ref_id(args[0])}, {self.ref_id(args[1])})"),
            "clone": lambda: ("E.clone_self(o)" if isinstance(args[0], list) and len(args[0]) == 1
                              else f"E.clone({self.ref_id(args[0])})"),
            "deleteClone": lambda: "E.delete_clone(o) return",
            "saveVariable": lambda: "E.store_var(...)",
            "readVariable": lambda: "E.load_var(...)",
            "removeAdaptiveSizeDevice": lambda: "E.adaptive_off()",
            # камера привязана к объекту: «выключить камеру» = этот объект не
            # едет вместе с камерой (кнопки и стрелки остаются на месте)
            "removeObjectCamera": lambda: "E.camera_off(o)",
            "focusCameraToObject": lambda: f"E.camera_focus(o, {f(0)}, {f(1)})",
            "setGravityAllObjects": lambda: f"E.set_gravity({f(0)}, {f(1)})",
            "addBody": lambda: "E.add_body(o, " + (self.option(args[0]) if args else "'dynamic'") + ")",
            "isSensor": lambda: "E.set_sensor(o, " + (self.option(args[0]) if args else "'off'") + ")",
            "setQuareWHHitbox": lambda: f"E.set_hitbox(o, {f(0)}, {f(1)})",
            "setAlignTextField": lambda: "E.text_set(%s, 'align', %s)" % (f(0), self.option(args[1])),
            "setPositionTextField": lambda: "E.text_set(%s, 'pos', %s, %s)" % (f(0), f(1), f(2)),
            "placeholderTextField": lambda: "E.text_set(%s, 'placeholder', %s)" % (f(0), f(1)),
            "deleteTextField": lambda: f"E.text_delete({f(0)})",
            "function": lambda: f"E.call_function({self.ref_id(args[0])})",
            "fbSetValue": lambda: f"E.fb_set({f(1)}, {f(0)}, {f(2)})",
            "fbGetValue": lambda: (
                "E.fb_get(" + f(0) + ", " + f(1) + ", function(v) "
                + ("E.set_gv(%d, v)" % args[2][1]
                   if len(args) > 2 and isinstance(args[2], list) and len(args[2]) > 1
                   and args[2][0] == "globalVariable" else "E.noop()")
                + " end)"),
            "transitionPosition2": lambda: f"E.transition(o, {f(0)}, {f(1)}, {f(2)})",
        }

        if name == "setVariable" or name == "editVariable":
            use, setter = self.target(args[0] if args else [])
            if name == "setVariable":
                value = formula(args[1], self.unsup) if len(args) > 1 else "0"
                return f"{setter % value}"
            delta = formula(args[1], self.unsup) if len(args) > 1 else "0"
            if use.startswith("E.gv"):
                gid = args[0][1]
                return f"E.add_gv({gid}, {delta})"
            lid = args[0][1]
            return f"E.add_lv(o, {lid}, {delta})"

        if name in ("saveVariable", "readVariable"):
            # «сохранить переменную 93» / «прочитать сохранённую 93»: ключ
            # хранилища — сама переменная, а не её значение (раньше ключ
            # терялся и все сохранения ложились в «var:nil»)
            arg = args[0] if args else []
            if isinstance(arg, list) and len(arg) > 1 and arg[0] == "globalVariable":
                gid = arg[1]
                if name == "saveVariable":
                    return f"E.store_var(o, E.gv({gid}), {gid})"
                return f"E.load_var(o, {gid})"
            use, _ = self.target(arg)
            return f"E.{'store_var' if name == 'saveVariable' else 'load_var'}(o, {use})"

        if name == "addElementArray":
            value = formula(args[0], self.unsup) if args else "0"
            aid = self.ref_id(args[1]) if len(args) > 1 else "0"
            return f"E.array_add({aid}, {value})"

        if name == "ask":
            question = formula(args[0], self.unsup) if args else '""'
            use, _ = self.target(args[1]) if len(args) > 1 else ('""', "")
            return f"E.ask(o, {question}, function(answer) E.set_gv({args[1][1] if len(args) > 1 and isinstance(args[1], list) and args[1][0] == 'globalVariable' else 0}, answer) end)"

        if name == "showVariable2":
            varid = args[0][1] if args and isinstance(args[0], list) and args[0][0] == "globalVariable" else 0
            rest = [formula(a, self.unsup) for a in args[1:]]
            while len(rest) < 4:
                rest.append("0")
            return (f"E.text_show_var({varid}, {rest[0]}, {rest[1]}, {rest[2]}, "
                    f"{rest[3]}, {self.option(args[5]) if len(args) > 5 else chr(34)*2})")

        if name in ("saveArray", "readArray", "deleteAllElementsArray"):
            # массивы проекта: saveArray/readArray кладут массив в хранилище
            # устройства и забирают обратно (в оригинале — достижения,
            # введённые промокоды и подобное)
            arg = args[0] if args else []
            aid = arg[1] if isinstance(arg, list) and len(arg) > 1 else 0
            fn = {"saveArray": "array_save", "readArray": "array_load",
                  "deleteAllElementsArray": "array_clear"}[name]
            return f"E.{fn}({aid})"

        if name == "hideVariable":
            arg = args[0] if args else []
            gid = arg[1] if isinstance(arg, list) and len(arg) > 1 else 0
            return f'E.cb.text_set("var:{gid}", "visible", 0)'

        if name == "showVariable":
            arg = args[0] if args else []
            gid = arg[1] if isinstance(arg, list) and len(arg) > 1 else 0
            return f'E.cb.text_set("var:{gid}", "visible", 1)'

        if name == "setFontSizeTextField":
            label = formula(args[0], self.unsup) if args else '""'
            size = formula(args[1], self.unsup) if len(args) > 1 else "40"
            return f"E.text_set({label}, 'size', {size})"

        if name in ("createVideo", "deleteVideo"):
            # видео-интро боя: кадры ролика заранее нарезаны tools/extract_videos.py,
            # движок листает их поверх сцены (E.video_play / E.video_stop)
            vid = "0"
            if args and isinstance(args[0], list) and len(args[0]) > 1:
                vid = str(args[0][1])
            call = "E.video_play" if name == "createVideo" else "E.video_stop"
            return f"{call}({vid})"

        if name == "createTextField":
            label = formula(args[0], self.unsup) if args else '""'
            editable = self.option(args[1]) if len(args) > 1 else '"off"'
            w = formula(args[2], self.unsup) if len(args) > 2 else "0"
            h = formula(args[3], self.unsup) if len(args) > 3 else "0"
            varid = args[4][1] if len(args) > 4 and isinstance(args[4], list) else 0
            return f"E.text_create({label}, {editable}, {w}, {h}, {varid})"

        if name == "createRadialParticle":
            label = formula(args[0], self.unsup) if args else '""'
            image = self.ref_id(args[1]) if len(args) > 1 else "0"
            return f"E.particles_create(o, {label}, {image})"

        if name == "setPositionParticle":
            label = formula(args[0], self.unsup) if args else '""'
            return f"E.particles_move(o, {label}, {f(1)}, {f(2)})"

        if name in table:
            return table[name]()

        self.unsup.brick(name)
        return f"E.unsupported({json.dumps(name, ensure_ascii=False)})"


# --------------------------------------------------------------------------------------
# модули объектов
# --------------------------------------------------------------------------------------

def lua_str(text: str) -> str:
    return json.dumps(str(text), ensure_ascii=False)


def object_module(obj: SceneObject, scene_name: str, unsup: Unsupported) -> tuple[str, int]:
    gen = BrickGen(unsup)
    scripts = split_scripts(obj.scripts)
    out = [
        "-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.",
        f"-- Сцена {obj.scene_id} {lua_str(scene_name)} / объект {obj.object_id} {lua_str(obj.name)}",
        'local E = require("runtime.engine")',
        "",
        "return {",
        f"  scene_id = {obj.scene_id},",
        f"  object_id = {obj.object_id},",
        f"  name = {lua_str(obj.name)},",
        "  scripts = {",
    ]
    for idx, script in enumerate(scripts, 1):
        raw_trigger = script["trigger"]
        if isinstance(raw_trigger, tuple) and raw_trigger[0] == "function":
            trig = f"function:{raw_trigger[1]}"
        else:
            trig = TRIGGERS.get(raw_trigger, "start")
        args = script["args"] or []
        cond_line = None
        if trig == "when_truth":
            cond_line = formula(args[0], unsup) if args else "false"
        out.append(f"    {{ -- скрипт {idx}: {trig}")
        out.append(f"      trigger = {lua_str(trig)},")
        if cond_line:
            out.append("      cond = function(o) return E.truthy(%s) end," % cond_line)
        out.append("      body = function(o)")
        out.extend(gen.block(script["body"], 4))
        out.append("      end,")
        out.append("    },")
    out.append("  },")
    out.append("}")
    return "\n".join(out) + "\n", len(scripts)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", default="original")
    ap.add_argument("--out", default="generated")
    args = ap.parse_args()

    project: Project = load_project(args.src)
    unsup = Unsupported()
    os.makedirs(os.path.join(args.out, "lua", "scenes"), exist_ok=True)

    index = {"project": project.name, "scenes": []}
    total_scripts = 0
    for scene in project.scenes:
        sdir = os.path.join(args.out, "lua", "scenes", f"{scene.scene_id:02d}")
        os.makedirs(sdir, exist_ok=True)
        modules = []
        for obj in scene.objects:
            code, count = object_module(obj, scene.name, unsup)
            fname = f"obj_{obj.object_id:03d}.lua"
            with open(os.path.join(sdir, fname), "w", encoding="utf-8") as fh:
                fh.write(code)
            modules.append({"object_id": obj.object_id, "name": obj.name, "file": fname,
                            "scripts": count})
            total_scripts += count
        with open(os.path.join(sdir, "index.lua"), "w", encoding="utf-8") as fh:
            fh.write("-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py\nreturn {\n")
            fh.write(f"  id = {scene.scene_id}, name = {lua_str(scene.name)},\n  objects = {{\n")
            for m in modules:
                fh.write(f"    {{ id = {m['object_id']}, name = {lua_str(m['name'])}, "
                         f"file = {lua_str(m['file'])}, scripts = {m['scripts']} }},\n")
            fh.write("  },\n}\n")
        index["scenes"].append({"id": scene.scene_id, "name": scene.name,
                                "dir": f"{scene.scene_id:02d}", "objects": modules})

    with open(os.path.join(args.out, "index.json"), "w", encoding="utf-8") as fh:
        json.dump(index, fh, ensure_ascii=False, indent=1)

    with open(os.path.join(args.out, "coverage.json"), "w", encoding="utf-8") as fh:
        json.dump({"unsupported_bricks": unsup.bricks,
                   "unsupported_expressions": unsup.expressions}, fh, ensure_ascii=False, indent=1)

    print(f"объектов: {sum(len(s['objects']) for s in index['scenes'])}, скриптов: {total_scripts}")
    print(f"неподдержанных блоков: {sum(unsup.bricks.values())} ({len(unsup.bricks)} видов), "
          f"выражений: {sum(unsup.expressions.values())} ({len(unsup.expressions)} видов)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
