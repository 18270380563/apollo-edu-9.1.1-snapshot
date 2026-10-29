/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "modules/planning/planners/public_road/public_road_planner.h"

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "modules/edu_debug/debug_logger.h"
#include "modules/planning/planning_base/common/speed/speed_data.h"
#include "modules/planning/planning_base/gflags/planning_gflags.h"
#include "modules/planning/planning_interface_base/scenario_base/scenario.h"

namespace apollo {
namespace planning {

using apollo::common::Status;
using apollo::common::TrajectoryPoint;
using apollo::edu_debug::Entry;
using apollo::edu_debug::EduDebugLogger;

namespace {

constexpr double kSpeedStateIntervalSec = 0.2;  // 5 Hz internal speed state
constexpr double kSpeedCurveIntervalSec = 1.0;  // 1 Hz sampled curves
constexpr double kCurveMaxS = 120.0;
constexpr double kCurveStepS = 2.0;

std::string Num(const double value) {
  char buffer[32];
  std::snprintf(buffer, sizeof(buffer), "%.4f", value);
  return std::string(buffer);
}

double SpeedPlanAt(const SpeedData& speed_data, const double s) {
  common::SpeedPoint point;
  if (!speed_data.EvaluateByS(s, &point)) {
    return -1.0;
  }
  return point.v();
}

double TrajectorySpeedAt(const DiscretizedTrajectory& trajectory,
                         const double s) {
  for (const auto& point : trajectory) {
    if (point.path_point().s() >= s) {
      return point.v();
    }
  }
  return -1.0;
}

// Why the car drives at the speed it does: cruise speed, speed limit curve,
// speed optimizer output and ST boundaries for every candidate reference line.
void DumpPlanningSpeedState(Frame* frame, const std::string& scenario,
                            const std::string& stage,
                            const std::string& scenario_msg,
                            const int scenario_status) {
  auto& logger = EduDebugLogger::Instance();

  static std::string last_scenario;
  static std::string last_stage;
  if (scenario != last_scenario || stage != last_stage) {
    logger.DumpEvent("public_road_planner.Plan", "scenario_changed",
                     last_scenario + "/" + last_stage + " -> " + scenario +
                         "/" + stage);
    last_scenario = scenario;
    last_stage = stage;
  }

  if (!logger.ShouldPublish(kSpeedStateIntervalSec, "planning_speed_state")) {
    return;
  }

  std::vector<Entry> kv;
  auto add = [&kv](const std::string& key, const std::string& value) {
    kv.emplace_back(key, value);
  };
  add("frame_seq", std::to_string(frame->SequenceNum()));
  add("scenario", scenario);
  add("stage", stage);
  add("scenario_status", std::to_string(scenario_status));
  add("scenario_msg", scenario_msg.substr(0, 200));
  add("vehicle_v", Num(frame->vehicle_state().linear_velocity()));
  add("target_speed_flag", Num(FLAGS_default_cruise_speed));
  add("upper_speed_limit", Num(FLAGS_planning_upper_speed_limit));

  auto* reference_lines = frame->mutable_reference_line_info();
  add("rl_count", std::to_string(reference_lines->size()));
  int index = 0;
  for (auto& info : *reference_lines) {
    if (index >= 3) {
      break;
    }
    const std::string prefix = "rl" + std::to_string(index) + ".";
    const auto& st_graph = info.st_graph_data();
    const auto& speed_data = info.speed_data();
    const auto& trajectory = info.trajectory();
    add(prefix + "id", info.Lanes().Id());
    add(prefix + "drivable", info.IsDrivable() ? "1" : "0");
    add(prefix + "change_lane", info.IsChangeLanePath() ? "1" : "0");
    add(prefix + "traj_type",
        ADCTrajectory::TrajectoryType_Name(info.trajectory_type()));
    add(prefix + "cruise", Num(info.GetCruiseSpeed()));
    add(prefix + "base_cruise", Num(info.GetBaseCruiseSpeed()));
    add(prefix + "st_cruise", Num(st_graph.cruise_speed()));
    add(prefix + "st_init_v", Num(st_graph.init_point().v()));
    add(prefix + "path_len", Num(info.reference_line().Length()));
    add(prefix + "path_n",
        std::to_string(info.path_data().discretized_path().size()));
    add(prefix + "speed_n", std::to_string(speed_data.size()));
    add(prefix + "v_plan_s0", Num(SpeedPlanAt(speed_data, 0.0)));
    double v_plan_max = 0.0;
    for (const auto& point : speed_data) {
      v_plan_max = std::max(v_plan_max, point.v());
    }
    add(prefix + "v_plan_max", Num(v_plan_max));
    add(prefix + "v_traj0",
        trajectory.empty() ? std::string("none") : Num(trajectory.front().v()));
    add(prefix + "st_bounds", std::to_string(st_graph.st_boundaries().size()));
    add(prefix + "stops",
        std::to_string(info.GetAllStopDecisionSLPoint().size()));
    if (info.GetBlockingObstacle() != nullptr) {
      add(prefix + "blocker", info.GetBlockingObstacle()->Id());
    }
    if (st_graph.is_initialized() &&
        st_graph.speed_limit().speed_limit_points().size() >= 2) {
      const auto& limit = st_graph.speed_limit();
      for (const double s : {0.0, 10.0, 20.0, 30.0, 50.0, 100.0}) {
        add(prefix + "limit_s" + std::to_string(static_cast<int>(s)),
            Num(limit.GetSpeedLimitByS(s)));
      }
    }
    ++index;
  }
  logger.DumpState("public_road_planner.Plan", "planning_speed_state", kv);

  if (!logger.ShouldPublish(kSpeedCurveIntervalSec, "planning_speed_curve") ||
      reference_lines->empty()) {
    return;
  }
  auto& info = reference_lines->front();
  const auto& st_graph = info.st_graph_data();
  const auto& speed_data = info.speed_data();
  const auto& trajectory = info.trajectory();
  const bool has_limit =
      st_graph.is_initialized() &&
      st_graph.speed_limit().speed_limit_points().size() >= 2;
  std::ostringstream curve;
  curve << "# frame_seq=" << frame->SequenceNum() << " scenario=" << scenario
        << " stage=" << stage
        << " cruise=" << Num(info.GetCruiseSpeed())
        << " base_cruise=" << Num(info.GetBaseCruiseSpeed())
        << " default_cruise=" << Num(FLAGS_default_cruise_speed) << "\n";
  curve << "# s v_limit v_plan v_traj\n";
  for (double s = 0.0; s <= kCurveMaxS; s += kCurveStepS) {
    curve << Num(s) << " "
          << (has_limit ? Num(st_graph.speed_limit().GetSpeedLimitByS(s))
                        : std::string("na"))
          << " " << Num(SpeedPlanAt(speed_data, s)) << " "
          << Num(TrajectorySpeedAt(trajectory, s)) << "\n";
  }
  logger.DumpText("public_road_planner.Plan", "trace", "planning_speed_curve",
                  curve.str());
}

}  // namespace

Status PublicRoadPlanner::Init(
    const std::shared_ptr<DependencyInjector>& injector,
    const std::string& config_path) {
  Planner::Init(injector, config_path);
  LoadConfig<PlannerPublicRoadConfig>(config_path, &config_);
  scenario_manager_.Init(injector, config_);
  return Status::OK();
}

Status PublicRoadPlanner::Plan(const TrajectoryPoint& planning_start_point,
                               Frame* frame,
                               ADCTrajectory* ptr_computed_trajectory) {
  scenario_manager_.Update(planning_start_point, frame);
  scenario_ = scenario_manager_.mutable_scenario();
  if (!scenario_) {
    return Status(apollo::common::ErrorCode::PLANNING_ERROR,
                  "Unknown Scenario");
  }
  auto result = scenario_->Process(planning_start_point, frame);

  if (FLAGS_enable_record_debug) {
    auto scenario_debug = ptr_computed_trajectory->mutable_debug()
                              ->mutable_planning_data()
                              ->mutable_scenario();
    scenario_debug->set_scenario_plugin_type(scenario_->Name());
    scenario_debug->set_stage_plugin_type(scenario_->GetStage());
    scenario_debug->set_msg(scenario_->GetMsg());
  }

  // EDUDBG: record why the planned speed is what it is.
  DumpPlanningSpeedState(frame, scenario_->Name(), scenario_->GetStage(),
                         scenario_->GetMsg(),
                         static_cast<int>(result.GetScenarioStatus()));

  if (result.GetScenarioStatus() == ScenarioStatusType::STATUS_DONE) {
    // only updates scenario manager when previous scenario's status is
    // STATUS_DONE
    scenario_manager_.Update(planning_start_point, frame);
  } else if (result.GetScenarioStatus() == ScenarioStatusType::STATUS_UNKNOWN) {
    return Status(common::PLANNING_ERROR,
                  result.GetTaskStatus().error_message());
  }
  return Status(common::OK, result.GetTaskStatus().error_message());
}

}  // namespace planning
}  // namespace apollo
