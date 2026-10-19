#include "modules/planning/scenarios/acc_lane_follow/acc_lane_follow_scenario.h"

#include "cyber/common/log.h"
#include "modules/planning/planning_base/gflags/planning_gflags.h"

namespace apollo {
namespace planning {

namespace {

struct FrontDynamicObstacle {
  const Obstacle* obstacle = nullptr;
  double longitudinal_distance = 0.0;
};

FrontDynamicObstacle FindFrontDynamicObstacle(const Frame& frame) {
  for (const auto& reference_line_info : frame.reference_line_info()) {
    const auto& adc_sl_boundary = reference_line_info.AdcSlBoundary();
    for (const auto* obstacle :
         reference_line_info.path_decision().obstacles().Items()) {
      if (obstacle->IsVirtual() || obstacle->IsStatic()) {
        continue;
      }

      const auto& obstacle_sl_boundary = obstacle->PerceptionSLBoundary();
      const double longitudinal_distance =
          obstacle_sl_boundary.start_s() - adc_sl_boundary.end_s();
      const bool overlaps_adc_lane =
          obstacle_sl_boundary.start_l() <= adc_sl_boundary.end_l() + 1.0 &&
          obstacle_sl_boundary.end_l() >= adc_sl_boundary.start_l() - 1.0;
      if (longitudinal_distance >= 0.0 &&
          longitudinal_distance <= FLAGS_acc_lane_follow_trigger_distance &&
          overlaps_adc_lane) {
        return {obstacle, longitudinal_distance};
      }
    }
  }
  return {};
}

}  // namespace

bool AccLaneFollowScenario::IsTransferable(const Scenario* other_scenario,
                                           const Frame& frame) {
  if (!FLAGS_enable_acc_lane_follow_scenario) {
    return false;
  }
  if (!frame.local_view().planning_command->has_lane_follow_command()) {
    return false;
  }
  if (frame.reference_line_info().empty()) {
    return false;
  }
  const auto front_dynamic_obstacle = FindFrontDynamicObstacle(frame);
  if (front_dynamic_obstacle.obstacle == nullptr) {
    if (other_scenario != nullptr && other_scenario->Name() == Name()) {
      AINFO << "ACC_SCENARIO exited: no front dynamic obstacle";
    }
    return false;
  }
  if (other_scenario != nullptr && other_scenario->Name() == Name()) {
    return true;
  }

  const auto& obstacle_sl_boundary =
      front_dynamic_obstacle.obstacle->PerceptionSLBoundary();
  AINFO << "ACC_SCENARIO enter: obstacle_id="
        << front_dynamic_obstacle.obstacle->Id()
        << ", distance=" << front_dynamic_obstacle.longitudinal_distance
        << "m, speed=" << front_dynamic_obstacle.obstacle->speed()
        << "mps, lateral_range=[" << obstacle_sl_boundary.start_l() << ", "
        << obstacle_sl_boundary.end_l() << "]";
  return true;
}

}  // namespace planning
}  // namespace apollo
