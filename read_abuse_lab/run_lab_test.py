from __future__ import annotations

import argparse
import threading
import time

from demo_server import LabHandler, ThreadingHTTPServer
from detector import EVENTS_PATH, main as detector_main
from register_detector import main as register_detector_main
from register_simulator import register_one
from simulator import CONFIG, run_user


def main() -> None:
    parser = argparse.ArgumentParser(description="Run a self-contained lab smoke test.")
    parser.add_argument("--keep-events", action="store_true", help="Keep existing events.jsonl before generating samples.")
    args = parser.parse_args()

    if not args.keep_events and EVENTS_PATH.exists():
        EVENTS_PATH.unlink()

    server = ThreadingHTTPServer(("127.0.0.1", 8787), LabHandler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    time.sleep(0.3)

    try:
        target = CONFIG["default_target"]
        for index in range(1, 9):
            register_one(target, index, "bulk_same_device")
        run_user(target, "bot_fixed_demo", "fixed_fast", 2, "book_demo")
        run_user(target, "bot_jump_demo", "jump_read", 2, "book_demo")
    finally:
        server.shutdown()
        thread.join(timeout=3)

    print("\nAccount registration risk")
    register_detector_main()
    print("\nComplete-reading risk")
    detector_main()


if __name__ == "__main__":
    main()
