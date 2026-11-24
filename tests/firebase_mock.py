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


# Аккаунт админа, который «знает» эмулятор (тесты входа в Firebase)
AUTH_EMAIL = "admin@cbclassic.local"
AUTH_PASSWORD = "Kenta2026"


def load_seed(path):
    if not path:
        return {}
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


REQUIRE_AUTH = False    # писать в админские узлы можно только с токеном
DUMP_AUTH = None


def dump_auth():
    if not DUMP_AUTH:
        return
    tmp = DUMP_AUTH + ".tmp"
    with open(tmp, "w", encoding="utf-8") as handle:
        json.dump(AUTH_SEEN, handle)
    os.replace(tmp, DUMP_AUTH)


def dump():
    if not DUMP:
        return
    tmp = DUMP + ".tmp"
    with open(tmp, "w", encoding="utf-8") as handle:
        json.dump(DB, handle, ensure_ascii=False, indent=1, sort_keys=True)
    os.replace(tmp, DUMP)


AUTH_SEEN = []          # query-параметры auth у пришедших запросов

ADMIN_NODES = ("Ивент", "Админское сообщение")


def needs_auth(parts, body=None):
    """Узлы, которые в «строгих» правилах пишет только админ.

    Ключи без «/» игра кладёт в world/, а пачку пишет одним PATCH в /world —
    поэтому смотрим и путь, и ключи в теле."""
    node = parts[1] if len(parts) > 1 and parts[0] == "world" else (parts[0] if parts else "")
    if node in ADMIN_NODES:
        return True
    if isinstance(body, dict) and parts and parts[-1] == "world":
        return any(key in ADMIN_NODES for key in body)
    return False


def note_auth(raw):
    query = raw.split("?", 1)
    if len(query) < 2:
        return
    for pair in query[1].split("&"):
        if pair.startswith("auth="):
            AUTH_SEEN.append(pair[5:] if len(pair) > 5 else "")


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

    def do_POST(self):
        # Эмулятор входа в Firebase: POST .../accounts:signInWithPassword?key=...
        body = self._read_body() or {}
        if "signInWithPassword" in self.path:
            email = str(body.get("email", ""))
            password = str(body.get("password", ""))
            if email == AUTH_EMAIL and password == AUTH_PASSWORD:
                self._reply(200, {"idToken": "test-id-token", "localId": "uid-admin",
                                  "email": email})
            else:
                self._reply(400, {"error": {"code": 400, "message": "INVALID_LOGIN_CREDENTIALS"}})
            return
        self._reply(404, {"error": "unknown endpoint"})

    def do_GET(self):
        note_auth(self.path)
        parts = split_path(self.path)
        with LOCK:
            node = get_node(parts)
        self._reply(200, node)

    def do_PUT(self):
        note_auth(self.path)
        dump_auth()
        parts = split_path(self.path)
        value = self._read_body()
        if REQUIRE_AUTH and needs_auth(parts, value) and "auth=test-id-token" not in self.path:
            self._reply(401, {"error": "Permission denied"})
            return
        with LOCK:
            set_node(parts, value)
            dump()
        self._reply(200, value)

    def do_PATCH(self):
        note_auth(self.path)
        dump_auth()
        parts = split_path(self.path)
        value = self._read_body()
        if REQUIRE_AUTH and needs_auth(parts, value) and "auth=test-id-token" not in self.path:
            self._reply(401, {"error": "Permission denied"})
            return
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

def main():
    global DB, DUMP, REQUIRE_AUTH, DUMP_AUTH
    parser = argparse.ArgumentParser(description="Эмулятор Firebase RTDB для тестов")
    parser.add_argument("--port", type=int, default=8899)
    parser.add_argument("--seed", help="файл с начальным содержимым базы")
    parser.add_argument("--dump", help="куда писать содержимое базы после каждой записи")
    parser.add_argument("--dump-auth", help="куда писать пришедшие токены авторизации")
    parser.add_argument("--require-auth", action="store_true",
                        help="писать в админские узлы можно только с токеном")
    args = parser.parse_args()
    DB = load_seed(args.seed)
    DUMP = args.dump
    global REQUIRE_AUTH, DUMP_AUTH
    REQUIRE_AUTH = args.require_auth
    DUMP_AUTH = args.dump_auth
    if DUMP:
        with LOCK:
            dump()
    server = ThreadingHTTPServer(("127.0.0.1", args.port), Handler)
    server.daemon_threads = True
    print("firebase_mock: http://127.0.0.1:%d/" % args.port, flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
