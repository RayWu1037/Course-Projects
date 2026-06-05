from __future__ import annotations

import argparse
import json
import random
import time
import uuid
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from simulator import CONFIG, MIN_GAP_MS, assert_allowed_target, post_json


ROOT = Path(__file__).resolve().parent


def nickname(index: int, mode: str) -> str:
    if mode == "normal":
        return f"reader_{uuid.uuid4().hex[:6]}"
    return f"reader_{index:04d}"


def profile(index: int, mode: str) -> dict:
    shared_device = "device_shared_lab"
    device_id = f"device_{uuid.uuid4().hex[:10]}" if mode == "normal" else shared_device
    ip_hint = f"10.0.0.{random.randint(2, 200)}" if mode == "normal" else "10.0.0.9"
    email_domain = random.choice(["example.edu", "lab.local"]) if mode == "normal" else "bulk.test"
    return {
        "user_id": f"{mode}_user_{index:04d}_{uuid.uuid4().hex[:4]}",
        "device_id": device_id,
        "ip_hint": ip_hint,
        "nickname": nickname(index, mode),
        "email_domain": email_domain,
        "mode": mode,
        "is_bot_simulation": mode != "normal",
    }


def register_one(target: str, index: int, mode: str) -> None:
    payload = profile(index, mode)
    post_json(target, "/api/register", payload)
    gap = random.randint(1500, 5000) if mode == "normal" else random.randint(MIN_GAP_MS, 260)
    time.sleep(gap / 1000)


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate account registration behavior against the local lab target.")
    parser.add_argument("--target", default=CONFIG["default_target"])
    parser.add_argument("--mode", choices=["normal", "bulk_same_device", "burst"], default="bulk_same_device")
    parser.add_argument("--count", type=int, default=10)
    parser.add_argument("--workers", type=int, default=4)
    args = parser.parse_args()

    target = assert_allowed_target(args.target)
    print(f"Target allowed: {target}")

    workers = 1 if args.mode == "normal" else min(args.workers, 8)
    with ThreadPoolExecutor(max_workers=workers) as pool:
        for index in range(1, args.count + 1):
            pool.submit(register_one, target, index, args.mode)


if __name__ == "__main__":
    main()
