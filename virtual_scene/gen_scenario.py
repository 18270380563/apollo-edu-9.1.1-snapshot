#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""虚实结合测试：按当前 RTK 位姿在车前方生成虚拟障碍物场景 json。

用法（在车端 docker 内执行）：
    python3 gen_scenario.py --ahead 40 --lateral 0 --out /apollo_workspace/tools/sim_scene/scene.json

会在车头正前方 --ahead 米处放一个虚拟车辆（静态），并写出 sim_obstacle 认识的场景 json。
"""

import argparse
import json
import math
import os
import sys
import time

from cyber.python.cyber_py3 import cyber
from modules.common_msgs.localization_msgs import localization_pb2


def quaternion_to_heading(qx, qy, qz, qw):
    """四元数 -> 航向角（rad），绕 Z 轴。"""
    siny = 2.0 * (qw * qz + qx * qy)
    cosy = 1.0 - 2.0 * (qy * qy + qz * qz)
    return math.atan2(siny, cosy)


def read_pose(channel="/apollo/localization/pose", timeout=5.0):
    cyber.init()
    node = cyber.Node("scenario_gen_%d" % int(time.time() * 1000))
    box = {}

    def on_pose(msg):
        box["msg"] = msg

    node.create_reader(channel, localization_pb2.LocalizationEstimate, on_pose)
    deadline = time.time() + timeout
    while "msg" not in box and time.time() < deadline:
        time.sleep(0.02)
    if "msg" not in box:
        raise RuntimeError("在 %.1fs 内没收到 %s" % (timeout, channel))
    pose = box["msg"].pose
    q = pose.orientation
    heading = pose.heading if pose.HasField("heading") else quaternion_to_heading(
        q.qx, q.qy, q.qz, q.qw
    )
    return {
        "x": pose.position.x,
        "y": pose.position.y,
        "z": pose.position.z,
        "heading": heading,
        "speed": math.hypot(pose.linear_velocity.x, pose.linear_velocity.y),
    }


def offset_pose(origin, forward_m, lateral_m=0.0):
    """沿车头方向前进 forward_m、左移 lateral_m 后的地图坐标。"""
    h = origin["heading"]
    return {
        "x": origin["x"] + forward_m * math.cos(h) - lateral_m * math.sin(h),
        "y": origin["y"] + forward_m * math.sin(h) + lateral_m * math.cos(h),
        "heading": h,
    }


TYPE_MAP = {
    "vehicle": 0,
    "bicycle": 1,
    "pedestrian": 2,
    "unknown_static": 3,
    "unknown_movable": 4,
    "big_truck": 5,
}
SIZE_MAP = {
    "vehicle": (4.8, 1.9, 1.6),
    "bicycle": (1.8, 0.6, 1.7),
    "pedestrian": (0.5, 0.7, 1.75),
    "unknown_static": (1.0, 1.0, 1.0),
    "unknown_movable": (1.0, 1.0, 1.0),
    "big_truck": (10.0, 2.5, 3.2),
}


def build_scenario(ego, args):
    start = {"x": round(ego["x"], 3), "y": round(ego["y"], 3),
             "heading": round(ego["heading"], 6)}
    end_pose = offset_pose(ego, args.route_length)
    end = {"x": round(end_pose["x"], 3), "y": round(end_pose["y"], 3),
           "heading": round(end_pose["heading"], 6)}

    agents = []
    for i in range(args.count):
        d = args.ahead + i * args.spacing
        p = offset_pose(ego, d, args.lateral)
        length, width, height = SIZE_MAP[args.kind]
        agent = {
            "id": args.id_base + i,
            "description": "virtual_%s_%.0fm" % (args.kind, d),
            "type": TYPE_MAP[args.kind],
            "motiontype": 0 if args.speed <= 0 else 2,
            "width": width,
            "length": length,
            "height": height,
            "start_position": {
                "x": round(p["x"], 3),
                "y": round(p["y"], 3),
                "heading": round(p["heading"], 6),
                "speed": args.speed,
            },
        }
        if args.speed > 0:
            # TRACKED：给一条沿车头方向的直线轨迹，让虚拟车匀速开走
            tp = []
            for k in range(6):
                q = offset_pose(ego, d + args.speed * k * 2.0, args.lateral)
                tp.append({"x": round(q["x"], 3), "y": round(q["y"], 3),
                           "heading": round(q["heading"], 6),
                           "speed": args.speed})
            agent["tracked_point"] = tp
        agents.append(agent)

    scenario = {
        "name": "realcar_virtual_%s_%s" % (args.kind, time.strftime("%Y%m%d_%H%M%S")),
        "description": "真车自动驾驶 + 虚拟障碍物（%s，前方 %s m）" % (args.kind, args.ahead),
        "map_dir": args.map_dir,
        "start": start,
        "end": end,
        "simulator_time": args.simulator_time,
        "start_velocity": round(ego["speed"], 3),
        "agent": agents,
        "deselect_default_metric": ["Checkpoint"],
    }
    # sim_obstacle 的场景文件在 studio 里是包了一层的外壳，这里同时给出两种形态
    wrapper = {
        "_id": "local_%d" % int(time.time()),
        "category": "private",
        "description": scenario["description"],
        "map_id": "",
        "number": 1,
        "scenario": scenario,
    }
    return scenario, wrapper


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--map-dir", default="modules/map/data/st5")
    ap.add_argument("--ahead", type=float, default=40.0, help="障碍物在车头前方多少米")
    ap.add_argument("--lateral", type=float, default=0.0, help="横向偏移，正=左")
    ap.add_argument("--route-length", type=float, default=120.0)
    ap.add_argument("--count", type=int, default=1)
    ap.add_argument("--spacing", type=float, default=30.0)
    ap.add_argument("--id-base", type=int, default=9001)
    ap.add_argument("--kind", default="vehicle", choices=list(TYPE_MAP))
    ap.add_argument("--speed", type=float, default=0.0, help=">0 时生成 TRACKED 运动的障碍物")
    ap.add_argument("--simulator-time", type=int, default=120)
    ap.add_argument("--wrapper", action="store_true", help="输出 studio 风格的外壳 json")
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    ego = read_pose()
    scenario, wrapper = build_scenario(ego, args)
    data = wrapper if args.wrapper else scenario

    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
    with open(args.out, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)

    print("ego  : x=%.2f y=%.2f heading=%.2f° v=%.2f m/s"
          % (ego["x"], ego["y"], math.degrees(ego["heading"]), ego["speed"]))
    print("obst : %d 个 %s, 前方 %.1f m ~ %.1f m"
          % (args.count, args.kind, args.ahead, args.ahead + (args.count - 1) * args.spacing))
    print("write: %s" % args.out)


if __name__ == "__main__":
    main()
