#!/usr/bin/env python3
"""Мини-эмулятор Firebase Realtime Database (REST) для тестов сети.

Повторяет то, чем пользуется игра: GET/PUT/PATCH/DELETE по адресу
<путь>.json, значения — числа и строки, PATCH сливает объект с текущим узлом.

    python3 tests/firebase_mock.py --port 8899 --seed seed.json --dump dump.json

--dump пишет всё содержимое «базы» после каждой записи — тест читает файл и
проверяет, что игра действительно отправила значение на сервер.
"""
import argparse
import json
import os
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import unquote

LOCK = threading.Lock()
DB = {}
DUMP = None
SEED = None


def load_seed(path):
    if not path:
        return {}
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def dump():
    if not DUMP:
        return
    tmp = DUMP + ".tmp"
    with open(tmp, "w", encoding="utf-8") as handle:
        json.dump(DB, handle, ensure_ascii=False, indent=1, sort_keys=True)
    os.replace(tmp, DUMP)


def split_path(raw):
    path = raw.split("?")[0]
    if path.endswith(".json"):
        path = path[: -len(".json")]
    return [unquote(segment) for segment in path.split("/") if segment]


def get_node(parts):
    node = DB
    for part in parts:
        if not isinstance(node, dict) or part not in node:
            return None
        node = node[part]
    return node


def set_node(parts, value):
    if not parts:
        return value
    node = DB
    for part in parts[:-1]:
        if not isinstance(node.get(part), dict):
            node[part] = {}
        node = node[part]
    if value is None:
        node.pop(parts[-1], None)
    else:
        node[parts[-1]] = value
    return value


def patch_node(parts, value):
    current = get_node(parts)
    if not isinstance(current, dict) or not isinstance(value, dict):
        return set_node(parts, value)
    for key, item in value.items():
        current[key] = item
    return current


class Handler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def log_message(self, *_args):
        pass

    def _reply(self, code, payload):
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _read_body(self):
        length = int(self.headers.get("Content-Length") or 0)
        if length <= 0:
            return None
        raw = self.rfile.read(length).decode("utf-8")
        return json.loads(raw) if raw.strip() else None

    def do_GET(self):
        parts = split_path(self.path)
        with LOCK:
            node = get_node(parts)
        self._reply(200, node)

    def do_PUT(self):
        parts = split_path(self.path)
        value = self._read_body()
        with LOCK:
            set_node(parts, value)
            dump()
        self._reply(200, value)

    def do_PATCH(self):
        parts = split_path(self.path)
        value = self._read_body()
        with LOCK:
            result = patch_node(parts, value)
            dump()
        self._reply(200, result)

    def do_DELETE(self):
        parts = split_path(self.path)
        with LOCK:
            set_node(parts, None)
            dump()
        self._reply(200, None)

    def do_POST(self):
        self._reply(405, {"error": "не поддерживается"})


def main():
    global DB, DUMP
    parser = argparse.ArgumentParser(description="Эмулятор Firebase RTDB для тестов")
    parser.add_argument("--port", type=int, default=8899)
    parser.add_argument("--seed", help="файл с начальным содержимым базы")
    parser.add_argument("--dump", help="куда писать содержимое базы после каждой записи")
    args = parser.parse_args()
    DB = load_seed(args.seed)
    DUMP = args.dump
    if DUMP:
        with LOCK:
            dump()
    server = ThreadingHTTPServer(("127.0.0.1", args.port), Handler)
    server.daemon_threads = True
    print("firebase_mock: http://127.0.0.1:%d/" % args.port, flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
