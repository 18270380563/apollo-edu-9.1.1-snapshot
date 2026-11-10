#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""读车上实时定位，投影到 st5 车道上，给出编辑器要填的「起点 s」。

为什么需要它：场景编辑器按 (s, l) 沿 Lane_1 摆演员，导出注入器命令时
--ahead = 演员的 s - 自车的 s。自车实际停在车道哪个位置，只有定位知道；
猜错了导出的 --ahead 就整体偏。

在容器内以 apollo 用户执行:
    docker exec -it apollo_neo_dev_edu_9.1.1 bash
    source /opt/apollo/neo/setup.sh
    cd /apollo_workspace/virtual_scene/scene_editor
    python3 ego_pose.py [车道 JSON]

输出:
    ego s = 12.34 m  l = -0.05 m  heading 偏差 = +0.3 deg
    把「起点 s」填 12.3
"""
import io
import json
import math
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
CANDIDATES = ["map_live.json", "20261110_st5_lanes_live.json",
              "20261110_st5_lanes_corrected.json"]
DEFAULT_JSON = next((os.path.join(HERE, n) for n in CANDIDATES
                     if os.path.isfile(os.path.join(HERE, n))),
                    os.path.join(HERE, CANDIDATES[0]))

POSE_CHANNEL = "/apollo/localization/pose"


def done():
    """吐完就走。cyber 的 python 节点在退出时会 join 后台线程，
    实测会卡成 100% CPU 不返回（2026-11-10 在车上复现，跑了 34 min CPU），
    所以直接 _exit，跳过一切清理。"""
    try:
        sys.stdout.flush()
    except Exception:
        pass
    os._exit(0)


def read_pose(timeout=5.0, channel=POSE_CHANNEL):
    from cyber.python.cyber_py3 import cyber
    from modules.common_msgs.localization_msgs import localization_pb2

    cyber.init()
    node = cyber.Node("ego_pose_%d" % int(time.time() * 1000))
    box = {}

    def on_pose(msg):
        box["msg"] = msg

    node.create_reader(channel, localization_pb2.LocalizationEstimate, on_pose)
    deadline = time.time() + timeout
    while "msg" not in box and time.time() < deadline:
        time.sleep(0.02)
    if "msg" not in box:
        raise RuntimeError("在 %.1f s 内没收到 %s（模块没起？）" % (timeout, channel))

    pose = box["msg"].pose
    q = pose.orientation
    if pose.HasField("heading"):
        heading = pose.heading
    else:
        heading = math.atan2(2.0 * (q.qw * q.qz + q.qx * q.qy),
                             1.0 - 2.0 * (q.qy * q.qy + q.qz * q.qz))
    speed = math.hypot(pose.linear_velocity.x, pose.linear_velocity.y)
    return {"x": pose.position.x, "y": pose.position.y,
            "heading": heading, "speed": speed}


def project(pts, x, y):
    """返回车道上的最近点：弧长 s、带符号横向 l（左正）、车道航向 h。"""
    best = None
    run = 0.0
    for i in range(len(pts) - 1):
        ax, ay = pts[i]
        bx, by = pts[i + 1]
        ux, uy = bx - ax, by - ay
        seg = math.hypot(ux, uy)
        if seg <= 0.0:
            continue
        ux, uy = ux / seg, uy / seg
        raw = (x - ax) * ux + (y - ay) * uy
        t = max(0.0, min(seg, raw))
        # 首段前 / 末段后的点允许外推，别一律夹成 0 和 length：
        # 车停在起点前半米时 s 该是负的，夹成 0 就看不出"车在线外"
        if i == 0 and raw < 0.0:
            t = raw
        if i == len(pts) - 2 and raw > seg:
            t = raw
        px, py = ax + ux * t, ay + uy * t
        d = math.hypot(x - px, y - py)
        if best is None or d < best["d"]:
            best = {"d": d, "s": run + t, "l": ux * (y - ay) - uy * (x - ax),
                    "h": math.atan2(uy, ux), "x": px, "y": py}
        run += seg
    return best


def main():
    # docker exec 下 LANG 常是 C，stdout 编码 ascii 会直接抛 UnicodeEncodeError
    try:
        sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8",
                                      errors="replace", line_buffering=True)
    except Exception:
        pass

    bare = "--bare" in sys.argv
    argv = [a for a in sys.argv[1:] if not a.startswith("--")]
    path = argv[0] if argv else DEFAULT_JSON
    if not os.path.isfile(path):
        raise SystemExit("找不到车道 JSON: %s" % path)
    with io.open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    lane = data["lanes"][0]
    pts = [(p[0], p[1]) for p in lane["points"]]

    pose = read_pose()
    pr = project(pts, pose["x"], pose["y"])
    dh = (pose["heading"] - pr["h"] + math.pi) % (2.0 * math.pi) - math.pi

    if bare:
        print("%.1f" % pr["s"])
        done()

    print("车道   : %s  (%.2f m)" % (lane["id"], lane["length"]))
    print("定位   : x=%.3f y=%.3f heading=%.4f rad  v=%.2f m/s"
          % (pose["x"], pose["y"], pose["heading"], pose["speed"]))
    print("投影   : s=%.2f m  l=%+.2f m（左正）  距车道中心 %.2f m"
          % (pr["s"], pr["l"], abs(pr["l"])))
    print("航向差 : %+.1f deg（正=车头偏左）" % (math.degrees(dh)))
    print("")
    print("把编辑器的「起点 s」填 %.1f" % pr["s"])
    if abs(pr["l"]) > 1.5:
        print("⚠ 横向偏离中心 %.1f m，车可能不在 Lane_1 车道上" % abs(pr["l"]))
    if pr["s"] > lane["length"] - 30:
        print("⚠ 距车道终点只剩 %.1f m，可用的场景长度不多"
              % (lane["length"] - pr["s"]))
    done()


if __name__ == "__main__":
    main()
