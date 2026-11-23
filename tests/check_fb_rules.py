#!/usr/bin/env python3
"""Проверка правил Firebase: firebase-rules.json — валидный JSON с закрытым корнем.

Если рядом лежит FIREBASE_RULES.md, его блок ```json сверяется с файлом, чтобы
текст в документации не разошёлся с тем, что вставляют в консоль.
"""
import json
import os
import re
import sys


def main():
    path = "firebase-rules.json"
    if not os.path.exists(path):
        print("нет %s — проверка пропущена" % path)
        return 0
    with open(path, encoding="utf-8") as handle:
        rules = json.load(handle)
    root = rules.get("rules", {})
    ok = root.get(".read") is False and root.get(".write") is False and "world" in root

    doc = "FIREBASE_RULES.md"
    if os.path.exists(doc):
        text = open(doc, encoding="utf-8").read()
        blocks = re.findall(r"```json\n(.*?)```", text, re.S)
        if not blocks:
            print("в %s нет блока ```json" % doc)
            ok = False
        elif json.loads(blocks[0]) != rules:
            print("%s расходится с %s" % (doc, path))
            ok = False

    print("узлов в правилах: %d, корень закрыт: %s" % (len(root), ok))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
