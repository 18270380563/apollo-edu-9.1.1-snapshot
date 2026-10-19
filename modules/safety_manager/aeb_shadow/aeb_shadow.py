#!/usr/bin/env python3
"""Forward AEB shadow-mode monitor.

This process only observes Cyber RT messages and publishes diagnostic status.
It does not publish control commands or access CAN hardware.
"""

import argparse
import json
import logging
import os
import threading
import time

from google.protobuf.wrappers_pb2 import StringValue

from cyber.python.cyber_py3 import cyber
from modules.common_msgs.chassis_msgs import chassis_pb2
from modules.common_msgs.localization_msgs import localization_pb2
from modules.common_msgs.perception_msgs import perception_obstacle_pb2

from aeb_logic import (
    STATUS_INACTIVE,
    STATUS_NO_DATA,
    classify_obstacles,
    classify_status,
    recommended_deceleration_mps2,
    select_nearest_forward_obstacle,
    transform_obstacle_to_vehicle_frame,
)


LOGGER = logging.getLogger("aeb_shadow")


class AebShadowMonitor:
    def __init__(self, config):
        self._config = config
        self._lock = threading.Lock()
        self._latest_chassis = None
        self._chassis_timestamp_sec = 0.0
        self._latest_localization = None
        self._localization_timestamp_sec = 0.0
        self._track_hits = {}
        self._last_status = None
        self._last_telemetry_timestamp_sec = 0.0
        self._node = cyber.Node(config["node_name"])
        self._writer = self._node.create_writer(
            config["status_channel"], StringValue
        )
        self._node.create_reader(
            config["chassis_channel"], chassis_pb2.Chassis, self._on_chassis
        )
        self._node.create_reader(
            config["localization_channel"],
            localization_pb2.LocalizationEstimate,
            self._on_localization,
        )
        self._node.create_reader(
            config["obstacles_channel"],
            perception_obstacle_pb2.PerceptionObstacles,
            self._on_obstacles,
        )

    def _on_chassis(self, chassis):
        with self._lock:
            self._latest_chassis = chassis
            self._chassis_timestamp_sec = time.time()

    def _on_localization(self, localization):
        with self._lock:
            self._latest_localization = {
                "position_x": localization.pose.position.x,
                "position_y": localization.pose.position.y,
                "heading": localization.pose.heading,
            }
            self._localization_timestamp_sec = time.time()

    def _on_obstacles(self, obstacles):
        with self._lock:
            chassis = self._latest_chassis
            localization = self._latest_localization
            chassis_age_sec = time.time() - self._chassis_timestamp_sec
            localization_age_sec = time.time() - self._localization_timestamp_sec

            if chassis is None or chassis_age_sec > self._config["max_chassis_age_sec"]:
                self._publish_status(
                    STATUS_NO_DATA, None, 0.0, chassis_age_sec, localization_age_sec
                )
                return
            if (
                localization is None
                or localization_age_sec > self._config["max_localization_age_sec"]
            ):
                self._publish_status(
                    STATUS_NO_DATA,
                    None,
                    max(chassis.speed_mps, 0.0),
                    chassis_age_sec,
                    localization_age_sec,
                )
                return

            ego_speed_mps = max(chassis.speed_mps, 0.0)
            obstacle_views = [
                self._to_vehicle_frame_view(item, localization)
                for item in obstacles.perception_obstacle
            ]
            visible_ids = {item["id"] for item in obstacle_views}
            self._track_hits = {
                obstacle_id: count
                for obstacle_id, count in self._track_hits.items()
                if obstacle_id in visible_ids
            }
            for obstacle_id in visible_ids:
                self._track_hits[obstacle_id] = self._track_hits.get(obstacle_id, 0) + 1

            nearest_forward_obstacle = select_nearest_forward_obstacle(
                obstacle_views, self._config
            )
            candidate = classify_obstacles(
                obstacle_views, ego_speed_mps, self._config, self._track_hits
            )
            if ego_speed_mps < self._config["min_ego_speed_mps"]:
                self._publish_status(
                    STATUS_INACTIVE,
                    nearest_forward_obstacle,
                    ego_speed_mps,
                    chassis_age_sec,
                    localization_age_sec,
                )
                return

            status = classify_status(candidate, self._config)
            self._publish_status(
                status,
                candidate,
                ego_speed_mps,
                chassis_age_sec,
                localization_age_sec,
            )

    @staticmethod
    def _to_vehicle_frame_view(obstacle, localization):
        map_frame_obstacle = {
            "id": obstacle.id,
            "position_x": obstacle.position.x,
            "position_y": obstacle.position.y,
            "velocity_x": obstacle.velocity.x,
            "velocity_y": obstacle.velocity.y,
            "length": obstacle.length,
            "width": obstacle.width,
        }
        return transform_obstacle_to_vehicle_frame(
            map_frame_obstacle,
            localization["position_x"],
            localization["position_y"],
            localization["heading"],
        )

    def _publish_status(
        self, status, candidate, ego_speed_mps, chassis_age_sec, localization_age_sec
    ):
        payload = {
            "mode": "SHADOW_ONLY",
            "status": status,
            "timestamp_sec": time.time(),
            "ego_speed_mps": ego_speed_mps,
            "chassis_age_sec": chassis_age_sec,
            "localization_age_sec": localization_age_sec,
            "brake_command_sent": False,
        }
        if candidate is not None:
            payload.update(candidate)
            payload["recommended_deceleration_mps2"] = recommended_deceleration_mps2(
                candidate, ego_speed_mps, self._config
            )

        message = StringValue(value=json.dumps(payload, sort_keys=True))
        self._writer.write(message)
        now = time.time()
        if now - self._last_telemetry_timestamp_sec >= self._config["telemetry_interval_sec"]:
            LOGGER.info("AEB shadow telemetry: %s", message.value)
            self._last_telemetry_timestamp_sec = now
        if status != self._last_status:
            LOGGER.warning("AEB shadow status changed: %s", message.value)
            self._last_status = status


def load_config(path):
    with open(path, "r") as config_file:
        return json.load(config_file)


def main():
    parser = argparse.ArgumentParser(description="Apollo AEB shadow-mode monitor")
    parser.add_argument("--config", required=True, help="Path to JSON configuration")
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
    config = load_config(args.config)
    cyber.init()
    AebShadowMonitor(config)
    try:
        while not cyber.is_shutdown():
            time.sleep(0.1)
    finally:
        cyber.shutdown()


if __name__ == "__main__":
    main()
