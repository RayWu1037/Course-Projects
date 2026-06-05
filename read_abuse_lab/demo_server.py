from __future__ import annotations

import argparse
import json
import time
import uuid
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlparse


ROOT = Path(__file__).resolve().parent
CONFIG = json.loads((ROOT / "lab_config.json").read_text(encoding="utf-8"))
DATA_DIR = ROOT / CONFIG["data_dir"]
EVENTS_PATH = DATA_DIR / CONFIG["events_file"]
PAGES_PER_CHAPTER = int(CONFIG["pages_per_chapter"])
ACCOUNTS: set[str] = set()


def now_ms() -> int:
    return int(time.time() * 1000)


def append_event(event: dict) -> None:
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    with EVENTS_PATH.open("a", encoding="utf-8") as f:
        f.write(json.dumps(event, ensure_ascii=False, sort_keys=True) + "\n")


class LabHandler(BaseHTTPRequestHandler):
    server_version = "ReadAbuseLab/1.0"

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path == "/health":
            self.send_json({"ok": True, "pages_per_chapter": PAGES_PER_CHAPTER})
            return
        if parsed.path == "/":
            self.send_json(
                {
                    "name": "Read Abuse Lab",
                    "endpoints": [
                        "POST /api/register",
                        "POST /api/session/start",
                        "POST /api/read/page",
                        "POST /api/chapter/complete",
                    ],
                }
            )
            return
        self.send_error(404, "Not found")

    def do_POST(self) -> None:
        parsed = urlparse(self.path)
        body = self.read_json()

        if parsed.path == "/api/register":
            user_id = body.get("user_id") or f"user_{uuid.uuid4().hex[:8]}"
            duplicate = user_id in ACCOUNTS
            ACCOUNTS.add(user_id)
            event = self.base_event("account_register", body, body.get("session_id"))
            event["user_id"] = user_id
            event["device_id"] = body.get("device_id")
            event["ip_hint"] = body.get("ip_hint")
            event["nickname"] = body.get("nickname")
            event["email_domain"] = body.get("email_domain")
            event["duplicate"] = duplicate
            append_event(event)
            self.send_json({"ok": not duplicate, "user_id": user_id, "duplicate": duplicate})
            return

        if parsed.path == "/api/session/start":
            session_id = body.get("session_id") or str(uuid.uuid4())
            event = self.base_event("session_start", body, session_id)
            append_event(event)
            self.send_json({"ok": True, "session_id": session_id})
            return

        if parsed.path == "/api/read/page":
            event = self.base_event("read_page", body, body.get("session_id"))
            event["page_index"] = int(body.get("page_index", 0))
            event["duration_ms"] = int(body.get("duration_ms", 0))
            append_event(event)
            self.send_json({"ok": True, "recorded": event["action"]})
            return

        if parsed.path == "/api/chapter/complete":
            event = self.base_event("chapter_complete", body, body.get("session_id"))
            event["duration_ms"] = int(body.get("duration_ms", 0))
            append_event(event)
            reward = 1 if event["duration_ms"] >= 8000 else 0
            self.send_json({"ok": True, "reward_points": reward})
            return

        self.send_error(404, "Not found")

    def base_event(self, action: str, body: dict, session_id: str | None) -> dict:
        return {
            "timestamp": now_ms(),
            "session_id": session_id,
            "user_id": body.get("user_id", "anonymous"),
            "book_id": body.get("book_id", "book_demo"),
            "chapter_id": body.get("chapter_id"),
            "action": action,
            "mode": body.get("mode", "unknown"),
            "is_bot_simulation": bool(body.get("is_bot_simulation", False)),
        }

    def read_json(self) -> dict:
        length = int(self.headers.get("Content-Length", "0"))
        if length <= 0:
            return {}
        raw = self.rfile.read(length)
        try:
            return json.loads(raw.decode("utf-8"))
        except json.JSONDecodeError:
            self.send_error(400, "Invalid JSON")
            return {}

    def send_json(self, payload: dict, status: int = 200) -> None:
        encoded = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(encoded)))
        self.end_headers()
        self.wfile.write(encoded)

    def log_message(self, fmt: str, *args) -> None:
        return


def main() -> None:
    parser = argparse.ArgumentParser(description="Run the local reading abuse lab server.")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8787)
    args = parser.parse_args()

    server = ThreadingHTTPServer((args.host, args.port), LabHandler)
    print(f"Read Abuse Lab server running at http://{args.host}:{args.port}")
    print(f"Writing events to {EVENTS_PATH}")
    server.serve_forever()


if __name__ == "__main__":
    main()
