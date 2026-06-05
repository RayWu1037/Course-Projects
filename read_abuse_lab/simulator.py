from __future__ import annotations

import argparse
import json
import random
import time
import uuid
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from urllib import request
from urllib.parse import urlparse


ROOT = Path(__file__).resolve().parent
CONFIG = json.loads((ROOT / "lab_config.json").read_text(encoding="utf-8"))
PAGES_PER_CHAPTER = int(CONFIG["pages_per_chapter"])
MIN_GAP_MS = int(CONFIG["min_request_gap_ms"])


class TargetBlocked(Exception):
    pass


def assert_allowed_target(target: str) -> str:
    target = target.rstrip("/")
    parsed = urlparse(target)
    if parsed.scheme not in {"http", "https"}:
        raise TargetBlocked("Target must be http or https.")
    if target not in {item.rstrip("/") for item in CONFIG["allowed_targets"]}:
        allowed = ", ".join(CONFIG["allowed_targets"])
        raise TargetBlocked(f"Target is not in lab_config.json allowed_targets: {allowed}")
    return target


def post_json(target: str, path: str, payload: dict) -> dict:
    data = json.dumps(payload).encode("utf-8")
    req = request.Request(
        target + path,
        data=data,
        headers={"Content-Type": "application/json; charset=utf-8"},
        method="POST",
    )
    with request.urlopen(req, timeout=10) as resp:
        return json.loads(resp.read().decode("utf-8"))


def delay_ms(mode: str) -> int:
    if mode == "normal":
        return random.randint(1800, 6500)
    if mode == "fixed_fast":
        return 180
    if mode == "jump_read":
        return random.randint(120, 260)
    if mode == "human_like_bot":
        return max(150, int(random.gauss(850, 80)))
    return random.randint(300, 900)


def run_user(target: str, user_id: str, mode: str, chapters: int, book_id: str) -> None:
    session_id = str(uuid.uuid4())
    base = {
        "session_id": session_id,
        "user_id": user_id,
        "book_id": book_id,
        "mode": mode,
        "is_bot_simulation": mode != "normal",
    }
    post_json(target, "/api/session/start", base)
    time.sleep(MIN_GAP_MS / 1000)

    for chapter_no in range(1, chapters + 1):
        chapter_id = f"chapter_{chapter_no:04d}"
        chapter_start = time.time()

        pages = PAGES_PER_CHAPTER
        if mode == "jump_read":
            pages = random.choice([1, 2])

        for page_index in range(1, pages + 1):
            duration = delay_ms(mode)
            payload = {
                **base,
                "chapter_id": chapter_id,
                "page_index": page_index,
                "duration_ms": duration,
            }
            post_json(target, "/api/read/page", payload)
            time.sleep(max(duration, MIN_GAP_MS) / 1000)

        elapsed_ms = int((time.time() - chapter_start) * 1000)
        post_json(
            target,
            "/api/chapter/complete",
            {**base, "chapter_id": chapter_id, "duration_ms": elapsed_ms},
        )
        time.sleep(MIN_GAP_MS / 1000)


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate reading behavior against the local lab target.")
    parser.add_argument("--target", default=CONFIG["default_target"])
    parser.add_argument("--mode", choices=["normal", "fixed_fast", "jump_read", "human_like_bot", "batch"], default="normal")
    parser.add_argument("--user", default="alice")
    parser.add_argument("--users", type=int, default=3)
    parser.add_argument("--chapters", type=int, default=3)
    parser.add_argument("--book", default="book_demo")
    args = parser.parse_args()

    target = assert_allowed_target(args.target)
    print(f"Target allowed: {target}")

    if args.mode == "batch":
        modes = ["fixed_fast", "jump_read", "human_like_bot"]
        with ThreadPoolExecutor(max_workers=min(args.users, 8)) as pool:
            for index in range(args.users):
                mode = modes[index % len(modes)]
                pool.submit(run_user, target, f"batch_user_{index + 1}", mode, args.chapters, args.book)
        return

    run_user(target, args.user, args.mode, args.chapters, args.book)


if __name__ == "__main__":
    main()
