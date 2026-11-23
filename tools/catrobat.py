"""Минимальный парсер проекта Pocket Code (Catrobat), экспортированного в текстовые файлы.

Структура проекта (как в релизе `YOU_DON'T_NEED_IT`):

    icon.png
    options.txt          {"displayWidth":720, ...}
    scenes.txt           [[<имя>, <id>], ...]
    variables.txt        [[<id>, <имя>[, <значение>]], ...]      глобальные переменные
    arrays.txt           [[<id>, <имя>], ...]                     глобальные массивы
    counter.txt          [<nextId>, <nextImageId>, ...]
    scene_<id>/objects.txt   [[<имя>, <id>, {"rotation":..,"path":..,"size":..}], ...]
    scene_<id>/functions.txt [[<id>, <имя>, <арность>], ...]      "функции" (broadcast-сообщения)
    scene_<id>/object_<id>/scripts.txt   плоский список блоков [имя, аргументы, "on"|"off"]
    scene_<id>/object_<id>/images.txt    [[<имя>, <id>], ...]     файлы image_<id>.png
    scene_<id>/object_<id>/sounds.txt    [[<имя>, <id>], ...]     файлы sound_<id>.mp3
    scene_<id>/object_<id>/variables.txt локальные переменные объекта

Формулы сериализованы как *плоский список токенов*, который при вычислении склеивается в
выражение, например:

    [["number", 3], ["number", 5], ["number", 0]]      -> "350"   (числовые куски)
    [["number", 0], ["number", "."], ["number", 5]]    -> "0.5"
    [["-", "-"], ["number", 2], ["number", 1]]         -> "-21"
    [["function", "random"], ["function", "("],
     ["number", 0], ["function", ","], ["number", 9],
     ["function", ")"]]                                -> "random(0,9)"
    [["globalVariable", 47], ["function", "="],
     ["number", 2]]                                    -> "var47 = 2"

Парсер ниже разбирает это в дерево выражений (см. `parse_formula`).
"""

from __future__ import annotations

import json
import os
import re
from dataclasses import dataclass, field
from typing import Any, Iterable

# --------------------------------------------------------------------------------------
# токены формул
# --------------------------------------------------------------------------------------

#: токены-операторы: ("function", "=") -> "="
OPERATORS = {
    "=": "eq", "≠": "ne", "!=": "ne",
    "<": "lt", ">": "gt", "≤": "le", "≥": "ge",
    "+": "add", "-": "sub", "×": "mul", "÷": "div", "*": "mul", "/": "div",
    "and": "and", "or": "or", "not": "not",
    "&": "and", "|": "or",
}

#: встроенные функции Pocket Code, у которых аргументы пишутся в скобках
CALL_FUNCTIONS = {
    "random", "join", "letter", "length", "contains", "subText", "abs", "ceil",
    "floor", "round", "sqrt", "sin", "cos", "tan", "asin", "acos", "atan", "ln",
    "log", "exp", "mod", "max", "min", "numberOfItems", "item", "indexOfItem",
    "regex", "truncate", "format", "color", "concatenate",
}

#: "нуль-арные" сенсоры среды (в сериализации выглядят как function без скобок)
ZERO_ARG_SENSORS = {
    "positionX", "positionY", "rotation", "size", "brightness", "alpha",
    "layer", "time", "tSecond", "tMinute", "tHour", "tDay", "tMonth", "tYear",
    "touchDisplayX", "touchDisplayY", "touchDisplayId", "touchesFinger", "touch",
    "touchDisplayXId", "touchDisplayYId", "touchDisplayIdId", "countTouchDisplay",
    "direction", "transparency", "arctangent2", "containsArray",
    "numberImage", "numberSounds", "nameImage", "name", "visible", "globalVolume",
    "loudness", "background", "currentBackground", "sceneName", "objectName",
}

#: сенсоры/функции, у которых аргумент закодирован в самом токене
INLINE_ARG_SENSORS = {
    "touchesObject2", "noRotate", "glideTo", "goto", "object", "scene", "image",
    "sound", "variable", "localVariable", "globalArray", "touchesPoint",
    "distanceTo", "objectExists", "typeBody",
}

_TRUE = {"true", "TRUE", "True"}
_FALSE = {"false", "FALSE", "False"}


@dataclass
class Tok:
    kind: str            # number | text | op | func | lparen | rparen | comma | var | array | ref | bool | unknown
    value: Any = None
    raw: Any = None


_NUM_CHUNK = re.compile(r"^-?[0-9.]+$")


def formula_tokens(formula: Any) -> list[Tok]:
    """Разбирает сериализованную формулу (список токенов) в плоский поток.

    Соседние числовые куски склеиваются в один числовой литерал: 3,5,0 -> 350.
    """
    if formula is None:
        return [Tok("number", 0.0)]
    if not isinstance(formula, list):
        return [Tok("number", formula)]

    toks: list[Tok] = []
    num_buf: str | None = None

    def flush() -> None:
        nonlocal num_buf
        if num_buf is not None:
            text = num_buf
            num_buf = None
            try:
                toks.append(Tok("number", float(text), text))
            except ValueError:
                toks.append(Tok("unknown", text, text))

    for item in formula:
        if isinstance(item, str):
            # встречаются «плоские» строковые токены, напр. ["alignText", "center"]
            flush()
            toks.append(Tok("text", item))
            continue
        if not isinstance(item, list) or not item:
            flush()
            toks.append(Tok("unknown", item, item))
            continue

        head = item[0]
        arg = item[1] if len(item) > 1 else None

        if head in ("(", ")", ","):
            flush()
            toks.append(Tok({"(": "lparen", ")": "rparen", ",": "comma"}[head]))
            continue
        if head in ("left", "center", "right"):
            flush()
            toks.append(Tok("text", str(head)))
            continue

        if head == "number":
            # числовой кусок: "3", ".", "5" ... склеиваем
            chunk = "" if arg is None else str(arg)
            num_buf = chunk if num_buf is None else num_buf + chunk
            continue
        if head == "text":
            flush()
            toks.append(Tok("text", "" if arg is None else str(arg)))
            continue
        if head == "-":
            # "-" всегда оператор: знак числа получается унарным минусом на разборе
            flush()
            toks.append(Tok("op", "sub"))
            continue
        if head == "function":
            name = str(arg)
            if name in ("(",):
                flush()
                toks.append(Tok("lparen"))
                continue
            if name in (")",):
                flush()
                toks.append(Tok("rparen"))
                continue
            if name in (",",):
                flush()
                toks.append(Tok("comma"))
                continue
            if name in _TRUE:
                flush()
                toks.append(Tok("bool", True))
                continue
            if name in _FALSE:
                flush()
                toks.append(Tok("bool", False))
                continue
            if name in ("left", "center", "right"):
                flush()
                toks.append(Tok("text", name))
                continue
            if name in OPERATORS:
                flush()
                toks.append(Tok("op", OPERATORS[name]))
                continue
            flush()
            if len(item) > 2:                      # сенсор с «зашитым» аргументом
                toks.append(Tok("func", name, item[2]))
            elif name in CALL_FUNCTIONS:
                toks.append(Tok("func", name))
            elif name in ZERO_ARG_SENSORS:
                toks.append(Tok("func", name))
            else:
                toks.append(Tok("func", name))
            continue
        if head == "globalVariable":
            flush()
            toks.append(Tok("var", None, int(arg)))
            continue
        if head == "localVariable":
            flush()
            toks.append(Tok("var", "local", int(arg)))
            continue
        if head == "globalArray":
            flush()
            toks.append(Tok("array", None, int(arg)))
            continue
        if head in ("scenes", "objects", "images", "sounds"):
            flush()
            toks.append(Tok("ref", head, int(arg)))
            continue
        if head in OPERATORS:                      # оператор вне "function"
            flush()
            toks.append(Tok("op", OPERATORS[head]))
            continue
        if head in ("alignText",):
            flush()
            toks.append(Tok("text", str(arg)))
            continue
        if head in ("onOrOff", "typeBody", "GL"):
            flush()
            toks.append(Tok("ref", head, arg))
            continue

        flush()
        if len(item) == 2 and isinstance(head, str):
            toks.append(Tok("func", head, arg))     # напр. ["goTo", 21]
        else:
            toks.append(Tok("unknown", head, item))

    flush()
    return toks


# --------------------------------------------------------------------------------------
# AST выражений
# --------------------------------------------------------------------------------------

@dataclass
class Expr:
    kind: str
    value: Any = None
    args: list["Expr"] = field(default_factory=list)


_PREC = {
    "or": 1, "and": 2,
    "eq": 3, "ne": 3, "lt": 3, "gt": 3, "le": 3, "ge": 3,
    "add": 4, "sub": 4,
    "mul": 5, "div": 5,
    "not": 6, "neg": 6,
}


class _Parser:
    def __init__(self, toks: list[Tok]):
        self.toks = toks
        self.i = 0

    def peek(self) -> Tok | None:
        return self.toks[self.i] if self.i < len(self.toks) else None

    def next(self) -> Tok | None:
        t = self.peek()
        if t is not None:
            self.i += 1
        return t

    # выражение целиком: неявная конкатенация "a b" трактуется как join(a, b)
    def parse_all(self) -> Expr:
        parts: list[Expr] = []
        while self.peek() is not None:
            parts.append(self.parse_expr(0))
        if not parts:
            return Expr("num", 0.0)
        if len(parts) == 1:
            return parts[0]
        return Expr("join", args=parts)

    def parse_expr(self, min_prec: int) -> Expr:
        left = self.parse_unary()
        while True:
            t = self.peek()
            if t is None or t.kind != "op":
                break
            prec = _PREC.get(t.value, 0)
            if prec < min_prec or prec == 0:
                break
            op = t.value
            self.next()
            if op == "not":
                left = Expr("not", args=[left])
                continue
            right = self.parse_expr(prec + 1)
            left = Expr(op, args=[left, right])
        return left

    def parse_unary(self) -> Expr:
        t = self.peek()
        if t is None:
            return Expr("num", 0.0)
        if t.kind == "op" and t.value == "sub":
            self.next()
            return Expr("neg", args=[self.parse_expr(_PREC["mul"])])
        if t.kind == "op" and t.value == "not":
            self.next()
            return Expr("not", args=[self.parse_unary()])
        if t.kind == "number":
            self.next()
            return Expr("num", t.value)
        if t.kind == "text":
            self.next()
            return Expr("str", t.value)
        if t.kind == "bool":
            self.next()
            return Expr("bool", t.value)
        if t.kind == "var":
            self.next()
            if t.value == "local":
                return Expr("lvar", t.raw)
            return Expr("gvar", t.raw)
        if t.kind == "array":
            self.next()
            return Expr("array", t.raw)
        if t.kind == "ref":
            self.next()
            return Expr("ref", (t.value, t.raw))
        if t.kind == "lparen":
            self.next()
            inner = self.parse_expr(0)
            if self.peek() and self.peek().kind == "rparen":
                self.next()
            return inner
        if t.kind == "func":
            self.next()
            name = t.value
            inline = t.raw
            args: list[Expr] = []
            if self.peek() and self.peek().kind == "lparen":
                self.next()
                while self.peek() is not None and self.peek().kind != "rparen":
                    if self.peek().kind == "comma":
                        self.next()
                        continue
                    args.append(self.parse_expr(0))
                if self.peek() and self.peek().kind == "rparen":
                    self.next()
            node = Expr("call", name, args)
            if inline is not None:
                node.value = name
                node.args = args + [Expr("num", inline)]
            return node
        if t.kind == "comma":
            self.next()
            return Expr("num", 0.0)
        self.next()
        return Expr("unknown", t.raw if t.raw is not None else t.value)


def parse_formula(formula: Any) -> Expr:
    return _Parser(formula_tokens(formula)).parse_all()


# --------------------------------------------------------------------------------------
# проект
# --------------------------------------------------------------------------------------

@dataclass
class SceneObject:
    scene_id: int
    object_id: int
    name: str
    rotation: float
    size: float
    default_image: str | None
    images: list[tuple[str, int]]
    sounds: list[tuple[str, int]]
    variables: list[tuple[int, str, Any]]
    scripts: list[dict]
    path: str

    @property
    def dir(self) -> str:
        return self.path


@dataclass
class Scene:
    scene_id: int
    name: str
    objects: list[SceneObject]
    functions: list[tuple[int, str, int]]
    path: str


@dataclass
class Project:
    root: str
    name: str
    stage_w: int
    stage_h: int
    scenes: list[Scene]
    variables: list[tuple[int, str, Any]]
    arrays: list[tuple[int, str]]
    counter: list[int]
    orientation: str = "horizontal"

    def scene_by_id(self, sid: int) -> Scene | None:
        return next((s for s in self.scenes if s.scene_id == sid), None)


def _read_json(path: str, default: Any) -> Any:
    if not os.path.exists(path):
        return default
    with open(path, "r", encoding="utf-8") as fh:
        text = fh.read().strip()
    if not text:
        return default
    return json.loads(text)


def _bricks(path: str) -> list[dict]:
    data = _read_json(path, [])
    out: list[dict] = []
    for brick in data or []:
        if isinstance(brick, list) and len(brick) >= 3 and isinstance(brick[0], str):
            out.append({"name": brick[0], "args": brick[1], "status": brick[2]})
    return out


def load_project(root: str) -> Project:
    opts = _read_json(os.path.join(root, "options.txt"), {})
    scenes_raw = _read_json(os.path.join(root, "scenes.txt"), [])
    variables = _read_json(os.path.join(root, "variables.txt"), [])
    arrays = _read_json(os.path.join(root, "arrays.txt"), [])
    counter = _read_json(os.path.join(root, "counter.txt"), [])

    scenes: list[Scene] = []
    for name, sid in scenes_raw:
        sdir = os.path.join(root, f"scene_{sid}")
        objs_raw = _read_json(os.path.join(sdir, "objects.txt"), [])
        functions = _read_json(os.path.join(sdir, "functions.txt"), [])

        objects: list[SceneObject] = []
        for entry in objs_raw:
            oname, oid = entry[0], entry[1]
            meta = entry[2] if len(entry) > 2 and isinstance(entry[2], dict) else {}
            odir = os.path.join(sdir, f"object_{oid}")
            images = [tuple(x) for x in _read_json(os.path.join(odir, "images.txt"), [])]
            sounds = [tuple(x) for x in _read_json(os.path.join(odir, "sounds.txt"), [])]
            ovars = _read_json(os.path.join(odir, "variables.txt"), [])
            scripts = _bricks(os.path.join(odir, "scripts.txt"))
            default_image = os.path.basename(meta.get("path") or "") or None
            objects.append(SceneObject(
                scene_id=sid, object_id=oid, name=oname,
                rotation=float(meta.get("rotation") or 0.0),
                size=float(meta.get("size") or 1.0),
                default_image=default_image,
                images=images, sounds=sounds, variables=ovars,
                scripts=scripts, path=odir,
            ))
        scenes.append(Scene(scene_id=sid, name=name, objects=objects,
                            functions=[tuple(f) for f in functions], path=sdir))

    return Project(
        root=root,
        name=str(opts.get("name") or "Pocket Code project"),
        stage_w=int(opts.get("displayWidth") or 720),
        stage_h=int(opts.get("displayHeight") or 1348),
        scenes=scenes, variables=variables, arrays=arrays, counter=counter,
        orientation=str(opts.get("orientation") or "horizontal"),
    )


def split_scripts(bricks: Iterable[dict]) -> list[dict]:
    """Режет плоский поток блоков на отдельные скрипты по триггерам.

    Триггеры Pocket Code: `start`, `whenTheTruth`, `onTouchObject`.
    """
    TRIGGERS = {"start", "whenTheTruth", "onTouchObject", "touchObject",
                "startClone", "touchBack"}
    scripts: list[dict] = []
    current: dict | None = None
    for brick in bricks:
        if brick["name"] in TRIGGERS:
            current = {"trigger": brick["name"], "args": brick["args"], "body": []}
            scripts.append(current)
            continue
        if current is None:                     # проект без триггера — не должен встречаться
            current = {"trigger": "start", "args": [], "body": []}
            scripts.append(current)
        current["body"].append(brick)
    return scripts
