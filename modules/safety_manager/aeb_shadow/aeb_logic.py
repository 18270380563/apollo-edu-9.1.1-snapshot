"""Pure decision logic for the AEB shadow-mode monitor."""

import math


STATUS_NO_DATA = "NO_DATA"
STATUS_INACTIVE = "INACTIVE"
STATUS_CLEAR = "CLEAR"
STATUS_WARNING = "WARNING"
STATUS_BRAKE_REQUEST = "BRAKE_REQUEST"


def transform_obstacle_to_vehicle_frame(obstacle, ego_position_x, ego_position_y, heading):
    """Transform a map-frame obstacle into the ego vehicle's forward-left frame."""
    delta_x = obstacle["position_x"] - ego_position_x
    delta_y = obstacle["position_y"] - ego_position_y
    cosine = math.cos(heading)
    sine = math.sin(heading)

    transformed = dict(obstacle)
    transformed["position_x"] = cosine * delta_x + sine * delta_y
    transformed["position_y"] = -sine * delta_x + cosine * delta_y
    transformed["velocity_x"] = (
        cosine * obstacle["velocity_x"] + sine * obstacle["velocity_y"]
    )
    return transformed


def select_nearest_forward_obstacle(obstacles, config):
    """Select the closest obstacle in the forward AEB corridor."""
    candidates = []
    for obstacle in obstacles:
        position_x = obstacle["position_x"]
        position_y = obstacle["position_y"]
        if not config["min_forward_distance_m"] < position_x <= config["max_forward_distance_m"]:
            continue

        obstacle_half_width = obstacle["width"] * 0.5
        if abs(position_y) > config["lane_half_width_m"] + obstacle_half_width:
            continue

        candidates.append(
            {
                "id": obstacle["id"],
                "distance_m": max(
                    0.0,
                    position_x - obstacle["length"] * 0.5 - config["ego_front_offset_m"],
                ),
                "lateral_offset_m": position_y,
            }
        )

    return min(candidates, key=lambda item: item["distance_m"]) if candidates else None


def classify_obstacles(obstacles, ego_speed_mps, config, track_hits):
    """Return the highest-risk forward obstacle without commanding a vehicle."""
    if ego_speed_mps < config["min_ego_speed_mps"]:
        return None

    candidates = []
    for obstacle in obstacles:
        position_x = obstacle["position_x"]
        position_y = obstacle["position_y"]
        if position_x <= config["min_forward_distance_m"]:
            continue
        if position_x > config["max_forward_distance_m"]:
            continue

        obstacle_half_width = obstacle["width"] * 0.5
        if abs(position_y) > config["lane_half_width_m"] + obstacle_half_width:
            continue

        distance_m = max(
            0.0,
            position_x - obstacle["length"] * 0.5 - config["ego_front_offset_m"],
        )
        closing_speed_mps = ego_speed_mps - obstacle["velocity_x"]
        if closing_speed_mps < config["min_closing_speed_mps"]:
            continue

        ttc_sec = distance_m / closing_speed_mps
        if not math.isfinite(ttc_sec):
            continue

        candidates.append(
            {
                "id": obstacle["id"],
                "distance_m": distance_m,
                "closing_speed_mps": closing_speed_mps,
                "ttc_sec": ttc_sec,
            }
        )

    if not candidates:
        return None

    candidate = min(candidates, key=lambda item: item["ttc_sec"])
    candidate["confirmed_frames"] = track_hits.get(candidate["id"], 0)
    return candidate


def classify_status(candidate, config):
    """Classify a confirmed candidate. This never sends a braking command."""
    if candidate is None:
        return STATUS_CLEAR

    if candidate["ttc_sec"] <= config["brake_ttc_sec"]:
        if candidate["confirmed_frames"] >= config["min_confirmed_frames"]:
            return STATUS_BRAKE_REQUEST
        return STATUS_WARNING

    if candidate["ttc_sec"] <= config["warning_ttc_sec"]:
        return STATUS_WARNING

    return STATUS_CLEAR


def recommended_deceleration_mps2(candidate, ego_speed_mps, config):
    """Calculate a bounded recommendation for diagnostics only."""
    if candidate is None or candidate["distance_m"] <= 0.0:
        return 0.0

    deceleration = ego_speed_mps * ego_speed_mps / (2.0 * candidate["distance_m"])
    return min(deceleration, config["max_recommended_deceleration_mps2"])
