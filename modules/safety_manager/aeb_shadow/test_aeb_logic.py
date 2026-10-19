#!/usr/bin/env python3
import math
import unittest

from aeb_logic import (
    STATUS_BRAKE_REQUEST,
    STATUS_CLEAR,
    STATUS_WARNING,
    classify_obstacles,
    classify_status,
    recommended_deceleration_mps2,
    select_nearest_forward_obstacle,
    transform_obstacle_to_vehicle_frame,
)


CONFIG = {
    "min_ego_speed_mps": 1.0,
    "min_forward_distance_m": 0.2,
    "max_forward_distance_m": 80.0,
    "lane_half_width_m": 1.75,
    "ego_front_offset_m": 1.2,
    "min_closing_speed_mps": 0.5,
    "warning_ttc_sec": 5.0,
    "brake_ttc_sec": 3.0,
    "min_confirmed_frames": 3,
    "max_recommended_deceleration_mps2": 5.0,
}


class AebLogicTest(unittest.TestCase):
    def setUp(self):
        self.obstacle = {
            "id": 11,
            "position_x": 16.0,
            "position_y": 0.2,
            "velocity_x": 0.0,
            "velocity_y": 0.0,
            "length": 4.0,
            "width": 1.8,
        }

    def test_requests_brake_after_confirmation(self):
        candidate = classify_obstacles([self.obstacle], 8.0, CONFIG, {11: 3})
        self.assertEqual(STATUS_BRAKE_REQUEST, classify_status(candidate, CONFIG))
        self.assertGreater(recommended_deceleration_mps2(candidate, 8.0, CONFIG), 0.0)

    def test_warns_before_confirmation(self):
        candidate = classify_obstacles([self.obstacle], 8.0, CONFIG, {11: 1})
        self.assertEqual(STATUS_WARNING, classify_status(candidate, CONFIG))

    def test_ignores_lateral_obstacle(self):
        self.obstacle["position_y"] = 4.0
        candidate = classify_obstacles([self.obstacle], 8.0, CONFIG, {11: 4})
        self.assertEqual(STATUS_CLEAR, classify_status(candidate, CONFIG))

    def test_selects_nearest_forward_obstacle_at_low_speed(self):
        farther_obstacle = dict(self.obstacle, id=12, position_x=30.0)
        nearest = select_nearest_forward_obstacle(
            [farther_obstacle, self.obstacle], CONFIG
        )
        self.assertEqual(11, nearest["id"])
        self.assertAlmostEqual(12.8, nearest["distance_m"])

    def test_transforms_map_position_and_velocity_to_vehicle_frame(self):
        map_obstacle = {
            "id": 12,
            "position_x": 100.0,
            "position_y": 210.0,
            "velocity_x": 1.0,
            "velocity_y": 0.0,
            "length": 1.0,
            "width": 1.0,
        }
        transformed = transform_obstacle_to_vehicle_frame(
            map_obstacle, 100.0, 200.0, math.pi / 2.0
        )
        self.assertAlmostEqual(10.0, transformed["position_x"])
        self.assertAlmostEqual(0.0, transformed["position_y"])
        self.assertAlmostEqual(0.0, transformed["velocity_x"])


if __name__ == "__main__":
    unittest.main()
