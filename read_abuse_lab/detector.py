from __future__ import annotations

import argparse
import json
import statistics
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parent
CONFIG = json.loads((ROOT / "lab_config.json").read_text(encoding="utf-8"))
EVENTS_PATH = ROOT / CONFIG["data_dir"] / CONFIG["events_file"]


def load_events(path: Path) -> list[dict]:
    if not path.exists():
        return []
    events = []
    with path.open("r", encoding="utf-8") as f:
        for line in f:
            if line.strip():
                events.append(json.loads(line))
    return events


def coefficient_of_variation(values: list[int]) -> float:
    if len(values) < 2:
        return 0.0
    mean = statistics.mean(values)
    if mean == 0:
        return 0.0
    return statistics.pstdev(values) / mean


def score_session(events: list[dict]) -> dict:
    page_events = [e for e in events if e["action"] == "read_page"]
    complete_events = [e for e in events if e["action"] == "chapter_complete"]
    durations = [int(e.get("duration_ms", 0)) for e in page_events]
    pages_per_chapter = defaultdict(int)
    reasons = []
    score = 0

    for event in page_events:
        pages_per_chapter[event.get("chapter_id")] += 1

    avg_page_ms = statistics.mean(durations) if durations else 0
    cv = coefficient_of_variation(durations)
    short_chapter_count = sum(1 for e in complete_events if int(e.get("duration_ms", 0)) < 6000)
    sparse_complete_count = sum(1 for count in pages_per_chapter.values() if count <= 2)

    if avg_page_ms and avg_page_ms < 700:
        score += 35
        reasons.append("平均翻页停留时间低于人类阅读常见下限")
    if len(durations) >= 5 and cv < 0.12:
        score += 25
        reasons.append("翻页间隔方差过低，呈现机械固定节奏")
    if short_chapter_count:
        score += min(30, short_chapter_count * 10)
        reasons.append("存在异常短时间完成章节")
    if sparse_complete_count:
        score += min(25, sparse_complete_count * 8)
        reasons.append("存在少量页面后直接完读的跳读行为")
    if len(complete_events) >= 5 and avg_page_ms < 1200:
        score += 15
        reasons.append("多章节连续快速完读")

    score = min(score, 100)
    if score >= 70:
        level = "high"
    elif score >= 35:
        level = "medium"
    else:
        level = "low"

    first = events[0]
    return {
        "session_id": first.get("session_id"),
        "user_id": first.get("user_id"),
        "mode": first.get("mode"),
        "events": len(events),
        "pages": len(page_events),
        "completed_chapters": len(complete_events),
        "avg_page_ms": round(avg_page_ms, 2),
        "page_interval_cv": round(cv, 4),
        "risk_score": score,
        "risk_level": level,
        "reasons": reasons or ["未触发主要异常规则"],
        "label": "bot_simulation" if any(e.get("is_bot_simulation") for e in events) else "normal",
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Detect suspicious complete-reading behavior.")
    parser.add_argument("--events", type=Path, default=EVENTS_PATH)
    parser.add_argument("--json", action="store_true", help="Print JSON lines instead of a table.")
    args = parser.parse_args()

    events = load_events(args.events)
    if not events:
        print(f"No events found at {args.events}")
        return

    reading_actions = {"session_start", "read_page", "chapter_complete"}
    sessions = defaultdict(list)
    for event in events:
        if event.get("action") not in reading_actions or not event.get("session_id"):
            continue
        sessions[event.get("session_id")].append(event)

    results = [score_session(sorted(items, key=lambda e: e["timestamp"])) for items in sessions.values()]
    results.sort(key=lambda item: item["risk_score"], reverse=True)

    if args.json:
        for result in results:
            print(json.dumps(result, ensure_ascii=False, sort_keys=True))
        return

    print("risk  level   user_id             mode            pages chapters avg_ms   cv     reasons")
    print("-" * 108)
    for r in results:
        reasons = "; ".join(r["reasons"])
        print(
            f"{r['risk_score']:>4}  {r['risk_level']:<6}  {r['user_id']:<18} "
            f"{r['mode']:<14} {r['pages']:>5} {r['completed_chapters']:>8} "
            f"{r['avg_page_ms']:>6} {r['page_interval_cv']:>6}  {reasons}"
        )


if __name__ == "__main__":
    main()
