from __future__ import annotations

import argparse
import json
from collections import Counter, defaultdict
from pathlib import Path

from detector import EVENTS_PATH, load_events


def score_registrations(events: list[dict]) -> list[dict]:
    regs = [e for e in events if e.get("action") == "account_register"]
    by_device = Counter(e.get("device_id") for e in regs if e.get("device_id"))
    by_ip = Counter(e.get("ip_hint") for e in regs if e.get("ip_hint"))
    by_domain = Counter(e.get("email_domain") for e in regs if e.get("email_domain"))

    grouped = defaultdict(list)
    for event in regs:
        key = event.get("device_id") or event.get("ip_hint") or "unknown"
        grouped[key].append(event)

    results = []
    for key, items in grouped.items():
        score = 0
        reasons = []
        devices = {e.get("device_id") for e in items if e.get("device_id")}
        ips = {e.get("ip_hint") for e in items if e.get("ip_hint")}
        domains = {e.get("email_domain") for e in items if e.get("email_domain")}
        timestamps = sorted(int(e.get("timestamp", 0)) for e in items)

        max_device_count = max((by_device[d] for d in devices), default=0)
        max_ip_count = max((by_ip[ip] for ip in ips), default=0)
        max_domain_count = max((by_domain[d] for d in domains), default=0)

        if max_device_count >= 5:
            score += 35
            reasons.append("同一设备短时间注册多个账号")
        if max_ip_count >= 8:
            score += 25
            reasons.append("同一 IP 注册频率异常")
        if max_domain_count >= 8:
            score += 15
            reasons.append("邮箱域名高度集中")
        if len(timestamps) >= 5 and timestamps[-1] - timestamps[0] < 10000:
            score += 25
            reasons.append("注册请求呈现突发批量模式")
        if any(e.get("is_bot_simulation") for e in items):
            score += 10
            reasons.append("样本标记为注册滥用模拟")

        score = min(score, 100)
        level = "high" if score >= 70 else "medium" if score >= 35 else "low"
        results.append(
            {
                "group": key,
                "accounts": len(items),
                "risk_score": score,
                "risk_level": level,
                "reasons": reasons or ["未触发主要注册异常规则"],
            }
        )

    return sorted(results, key=lambda item: item["risk_score"], reverse=True)


def main() -> None:
    parser = argparse.ArgumentParser(description="Detect suspicious account registration behavior.")
    parser.add_argument("--events", type=Path, default=EVENTS_PATH)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    results = score_registrations(load_events(args.events))
    if not results:
        print(f"No account registration events found at {args.events}")
        return

    if args.json:
        for result in results:
            print(json.dumps(result, ensure_ascii=False, sort_keys=True))
        return

    print("risk  level   group                 accounts  reasons")
    print("-" * 86)
    for r in results:
        print(f"{r['risk_score']:>4}  {r['risk_level']:<6}  {r['group']:<20} {r['accounts']:>8}  {'; '.join(r['reasons'])}")


if __name__ == "__main__":
    main()
