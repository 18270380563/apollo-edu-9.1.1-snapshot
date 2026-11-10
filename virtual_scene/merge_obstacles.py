#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""真实感知 + 虚拟感知 融合节点（不停真实 perception）

订阅：
  --real      /apollo/perception/obstacles     真实激光感知（照常运行，不动它）
  --virtual   /apollo/virtual/obstacles        sim_obstacle 注入的虚拟障碍
发布：
  --out       /apollo/perception/obstacles_fused   两者合并后的结果

虚拟障碍超过 --ttl 秒没更新就丢弃（sim_obstacle 退出后自动消失）。
真实障碍原样保留，只按 id 去重。
"""

import argparse
import sys
import time

from cyber.python.cyber_py3 import cyber
from modules.common_msgs.perception_msgs import perception_obstacle_pb2


class Merger:
    def __init__(self, args):
        self.args = args
        self.real = None
        self.real_ts = 0.0
        self.virt = None
        self.virt_ts = 0.0
        self.real_count = 0
        self.virt_count = 0
        self.real_fresh = False
        self.seq = 0

    def on_real(self, msg):
        self.real = msg
        self.real_ts = time.time()
        self.real_count = len(msg.perception_obstacle)

    def on_virtual(self, msg):
        self.virt = msg
        self.virt_ts = time.time()
        self.virt_count = len(msg.perception_obstacle)

    def build(self):
        out = perception_obstacle_pb2.PerceptionObstacles()
        now = time.time()
        out.header.timestamp_sec = now
        out.header.module_name = "obstacle_fusion"
        out.header.sequence_num = self.seq
        self.seq += 1

        seen = set()
        real_fresh = self.real is not None and (now - self.real_ts) <= self.args.real_ttl
        if real_fresh:
            for ob in self.real.perception_obstacle:
                out.perception_obstacle.add().CopyFrom(ob)
                seen.add(ob.id)
        self.real_fresh = real_fresh

        n_virt = 0
        if self.virt is not None and (now - self.virt_ts) <= self.args.ttl:
            for ob in self.virt.perception_obstacle:
                if ob.id in seen:
                    continue
                out.perception_obstacle.add().CopyFrom(ob)
                seen.add(ob.id)
                n_virt += 1
        return out, n_virt


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--real", default="/apollo/perception/obstacles")
    ap.add_argument("--virtual", default="/apollo/virtual/obstacles")
    ap.add_argument("--out", default="/apollo/perception/obstacles_fused")
    ap.add_argument("--rate", type=float, default=20.0)
    ap.add_argument("--ttl", type=float, default=0.6, help="虚拟障碍多久没更新就丢弃")
    ap.add_argument("--real-ttl", type=float, default=1.0,
                    help="真实感知多久没更新就不再转发（防止 perception 停了以后一直重发最后一帧）")
    ap.add_argument("--node-name", default="obstacle_fusion",
                    help="cyber 节点名，必须全局唯一；想并行跑第二个实例（比如测试通道）就换一个")
    args = ap.parse_args()

    cyber.init()
    m = Merger(args)
    node = cyber.Node(args.node_name)
    node.create_reader(args.real, perception_obstacle_pb2.PerceptionObstacles, m.on_real)
    node.create_reader(args.virtual, perception_obstacle_pb2.PerceptionObstacles, m.on_virtual)
    writer = node.create_writer(args.out, perception_obstacle_pb2.PerceptionObstacles)

    print("fusion: real=%s virtual=%s -> out=%s rate=%.0fHz ttl=%.2fs node=%s"
          % (args.real, args.virtual, args.out, args.rate, args.ttl, args.node_name))
    sys.stdout.flush()

    period = 1.0 / args.rate
    last_log = 0.0
    try:
        while True:
            time.sleep(period)
            msg, n_virt = m.build()
            writer.write(msg)
            now = time.time()
            if now - last_log >= 2.0:
                last_log = now
                print("merged: real=%d(%s) virtual=%d total=%d"
                      % (m.real_count, "fresh" if m.real_fresh else "stale",
                         n_virt, len(msg.perception_obstacle)))
                sys.stdout.flush()
    except KeyboardInterrupt:
        pass
    return 0


if __name__ == "__main__":
    sys.exit(main())
