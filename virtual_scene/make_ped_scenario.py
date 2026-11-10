#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""按 st5 车道生成"运动行人"官方场景 json（给 sim_obstacle 用）。

- 从 base_map.txt 读车道中心线，把当前车位置投影到车道
- 在车前方 N 米处放一个横穿/迎面行走的行人（TRACKED + TIME 触发）
- start/end 用真实车道点，保证 routing 能算出来
"""

import argparse
import json
import math
import os
import re
import sys
import time

MAP_DIR = "/apollo/modules/map/data/st5"
MAP_TXT = MAP_DIR + "/base_map.txt"


def load_lane(path=MAP_TXT):
    txt = open(path, errors="ignore").read()
    pts = [(float(x), float(y)) for x, y in
           re.findall(r"point \{\s*x: ([\d.]+)\s*y: ([\d.]+)", txt)]
    return pts


def heading_of(pts, i):
    a = pts[max(0, i - 1)]
    b = pts[min(len(pts) - 1, i + 1)]
    return math.atan2(b[1] - a[1], b[0] - a[0])


def nearest_index(pts, x, y):
    best, bi = 1e18, 0
    for i, (px, py) in enumerate(pts):
        d = (px - x) ** 2 + (py - y) ** 2
        if d < best:
            best, bi = d, i
    return bi


def point_at_distance(pts, start_idx, dist):
    """沿车道从 start_idx 走 dist 米，返回 (index, 剩余不足的距离)"""
    acc = 0.0
    for i in range(start_idx, len(pts) - 1):
        seg = math.hypot(pts[i + 1][0] - pts[i][0], pts[i + 1][1] - pts[i][1])
        acc += seg
        if acc >= dist:
            return i + 1
    return len(pts) - 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ahead", type=float, default=40.0, help="行人在车前方多少米")
    ap.add_argument("--mode", default="cross", choices=["cross", "toward", "static"])
    ap.add_argument("--speed", type=float, default=1.2)
    ap.add_argument("--delay", type=float, default=5.0, help="第几秒开始动（TIME 触发）")
    ap.add_argument("--lateral", type=float, default=4.0, help="横穿起点横向偏移")
    ap.add_argument("--ego-x", type=float, required=True)
    ap.add_argument("--ego-y", type=float, required=True)
    ap.add_argument("--ego-heading", type=float, default=0.0)
    ap.add_argument("--sim-time", type=int, default=120)
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    lane = load_lane()
    if len(lane) < 10:
        print("lane points too few: %d" % len(lane))
        return 2

    i0 = nearest_index(lane, args.ego_x, args.ego_y)
    i_ped = point_at_distance(lane, i0, args.ahead)
    px, py = lane[i_ped]
    h = heading_of(lane, i_ped)
    if i_ped <= i0:
        print("warn: 前方 %.0f m 超出车道长度，行人放在车道末端" % args.ahead)

    # 行人轨迹：cross=横向穿过车道，toward=迎着车头方向走
    tracked = []
    if args.mode == "cross":
        dirx, diry = -math.sin(h), math.cos(h)   # 车道左向
        pts = [(-args.lateral, 0.0), (0.0, 0.6), (args.lateral, 1.2)]
        for lat, t in pts:
            tracked.append({"x": round(px + dirx * lat, 3),
                            "y": round(py + diry * lat, 3),
                            "heading": round(math.atan2(diry, dirx), 6),
                            "speed": args.speed})
        start = {"x": round(px + dirx * -args.lateral, 3),
                 "y": round(py + diry * -args.lateral, 3),
                 "heading": round(math.atan2(diry, dirx), 6),
                 "speed": args.speed}
    elif args.mode == "toward":
        dirx, diry = math.cos(h + math.pi), math.sin(h + math.pi)
        for k in range(3):
            tracked.append({"x": round(px + dirx * args.speed * 2.0 * k, 3),
                            "y": round(py + diry * args.speed * 2.0 * k, 3),
                            "heading": round(math.atan2(diry, dirx), 6),
                            "speed": args.speed})
        start = {"x": round(px, 3), "y": round(py, 3),
                 "heading": round(math.atan2(diry, dirx), 6), "speed": args.speed}
    else:
        tracked = []
        start = {"x": round(px, 3), "y": round(py, 3),
                 "heading": round(h, 6), "speed": 0.0}

    # 起终点必须落在车道内部：端点处 routing 找不到 lane，会报
    # "Cannot convert the end pose to lane way point"
    i_start = min(i0 + 3, len(lane) - 30)
    i_end = len(lane) - 12
    if i_end <= i_ped:
        i_end = min(len(lane) - 1, i_ped + 5)
    start_lane = lane[i_start]
    end_lane = lane[i_end]
    scenario = {
        "name": "aeb_pedestrian_%s" % time.strftime("%Y%m%d_%H%M%S"),
        "description": "AEB 测试：%s 行人，车前 %.0f m，第 %.0f s 开始以 %.1f m/s 移动"
                       % (args.mode, args.ahead, args.delay, args.speed),
        "map_dir": "modules/map/data/st5",
        "start": {"x": round(start_lane[0], 3), "y": round(start_lane[1], 3),
                  "heading": round(heading_of(lane, i_start), 6)},
        "end": {"x": round(end_lane[0], 3), "y": round(end_lane[1], 3),
                "heading": round(heading_of(lane, i_end), 6)},
        "simulator_time": args.sim_time,
        "agent": [{
            "id": 9101,
            "description": "pedestrian_%s" % args.mode,
            "type": 2,
            "width": 0.7, "length": 0.5, "height": 1.75,
            "motiontype": 2 if args.mode != "static" else 0,
            "trigger_type": 1,
            "start_time": args.delay,
            "start_position": start,
            "tracked_point": tracked,
        }],
        "deselect_default_metric": ["Checkpoint"],
    }

    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
    with open(args.out, "w", encoding="utf-8") as f:
        json.dump(scenario, f, indent=2, ensure_ascii=False)

    print("lane idx: ego=%d ped=%d (len=%d)" % (i0, i_ped, len(lane)))
    print("pedestrian at (%.2f, %.2f) heading=%.1fdeg mode=%s v=%.1f delay=%.0fs"
          % (px, py, math.degrees(h), args.mode, args.speed, args.delay))
    print("write: %s" % args.out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
