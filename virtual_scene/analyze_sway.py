#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""从 cyber record 里提取航向/横摆角速度，量化方向盘左右摆。

用法: python3 analyze_sway.py <record_file> [channel ...]
只读 localization / imu，其它消息直接跳过。
"""

import math
import sys
import time

from cyber.python.cyber_py3 import record
from modules.common_msgs.localization_msgs import localization_pb2
from modules.common_msgs.chassis_msgs import chassis_pb2


def load(path, want):
    """返回 {channel: [(t, msg), ...]}"""
    out = {c: [] for c in want}
    reader = record.RecordReader(path)
    n = 0
    for item in reader.read_messages():
        n += 1
        channel = item[0]
        msg = item[1]
        if channel in out:
            out[channel].append((item[-1], msg))
    return out, n


def wrap(a):
    while a > math.pi:
        a -= 2 * math.pi
    while a < -math.pi:
        a += 2 * math.pi
    return a


def main():
    path = sys.argv[1]
    t0 = time.time()
    data, n = load(path, {"/apollo/localization/pose"})
    print("total messages: %d, took %.1fs" % (n, time.time() - t0))

    poses = data["/apollo/localization/pose"]
    if not poses:
        print("no localization pose")
        return 1

    rows = []
    for t, raw in poses:
        m = localization_pb2.LocalizationEstimate()
        m.ParseFromString(raw)
        rows.append({
            "t": m.header.timestamp_sec,
            "x": m.pose.position.x,
            "y": m.pose.position.y,
            "h": m.pose.heading,
            "v": math.hypot(m.pose.linear_velocity.x, m.pose.linear_velocity.y),
            "wz": m.pose.angular_velocity.z,
        })

    rows.sort(key=lambda r: r["t"])
    ts = [r["t"] for r in rows]
    dur = ts[-1] - ts[0]
    print("duration %.1fs, %d poses, %.1f Hz" % (dur, len(rows), len(rows) / max(dur, 1e-6)))

    # 1) 航向振荡：去趋势（减去起止连线）后看峰峰值
    h = [r["h"] for r in rows]
    h0, h1 = h[0], h[-1]
    dev = [wrap(hi - (h0 + (h1 - h0) * (ts[i] - ts[0]) / max(ts[-1] - ts[0], 1e-6)))
           for i, hi in enumerate(h)]
    print("heading deviation: p2p=%.2f deg, std=%.2f deg"
          % (math.degrees(max(dev) - min(dev)),
             math.degrees(math.sqrt(sum(d * d for d in dev) / len(dev)))))

    # 2) 横摆角速度零穿越 → 摆动次数与周期
    wz = [r["wz"] for r in rows]
    idx = [i for i in range(1, len(wz)) if wz[i - 1] * wz[i] < 0 and abs(wz[i]) > 0.02]
    print("yaw-rate sig changes: %d over %.1fs" % (len(idx), dur))
    if len(idx) >= 4:
        gaps = [(ts[idx[k + 1]] - ts[idx[k]]) for k in range(len(idx) - 1)]
        gaps.sort()
        med = gaps[len(gaps) // 2]
        print("half-period median: %.2fs -> ~%.2f Hz" % (med, 0.5 / med))
    amp = sorted(abs(w) for w in wz)[int(len(wz) * 0.95)]
    print("yaw-rate p95 |wz| = %.2f deg/s, max = %.2f deg/s"
          % (math.degrees(amp), math.degrees(max(abs(w) for w in wz))))

    # 3) 速度区间
    vs = [r["v"] for r in rows]
    print("speed: min=%.2f max=%.2f mean=%.2f m/s" % (min(vs), max(vs), sum(vs) / len(vs)))

    # 4) 每 0.5 s 打一行，方便肉眼判断摆动
    print("\n   t(s)   v(m/s)  heading(deg)  wz(deg/s)")
    last = -1e9
    for r in rows:
        if r["t"] - last >= 0.5:
            last = r["t"]
            print("%7.2f  %6.2f  %11.2f  %9.2f"
                  % (r["t"] - ts[0], r["v"], math.degrees(r["h"]), math.degrees(r["wz"])))
    return 0


if __name__ == "__main__":
    sys.exit(main())
