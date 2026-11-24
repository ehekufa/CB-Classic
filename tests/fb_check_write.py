#!/usr/bin/env python3
"""Проверка, что игра действительно записала значение в «базу».

    python3 tests/fb_check_write.py dump.json "Гость/123/В" 20
"""
import json
import sys


def main():
    path, key, expected = sys.argv[1], sys.argv[2], sys.argv[3]
    with open(path, "r", encoding="utf-8") as handle:
        node = json.load(handle)
    for part in key.split("/"):
        if not isinstance(node, dict) or part not in node:
            print("%s: нет значения %s" % (path, key))
            return 1
        node = node[part]
    ok = str(node) == expected
    print("в базе %s = %s (ожидалось %s)" % (key, node, expected))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
