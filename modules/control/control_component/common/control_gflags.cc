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

#include "modules/control/control_component/common/control_gflags.h"

DEFINE_string(pipeline_file,
              "/apollo/modules/control/control_component/conf/pipeline.pb.txt",
              "default control conf data file");

DEFINE_string(control_conf_file,
              "/apollo/modules/control/conf/control_conf.pb.txt",
              "default control conf data file");

DEFINE_string(mpc_controller_conf_file,
              "/apollo/modules/control/conf/mpc_controller_conf.pb.txt",
              "mpc controller conf data file");

DEFINE_string(lateral_controller_conf_file,
              "/apollo/modules/control/conf/lateral_controller_conf.pb.txt",
              "lateral controller conf data file");

DEFINE_string(
    longitudinal_controller_conf_file,
    "/apollo/modules/control/conf/longitudinal_controller_conf.pb.txt",
    "longitudinal controller conf data file");

DEFINE_string(
    calibration_table_file,
    "/apollo/modules/control/control_component/conf/calibration_table.pb.txt",
    "calibration table file");

DEFINE_double(control_test_duration, -1.0, "control test duration");

DEFINE_bool(enable_csv_debug, false, "True to write out csv debug file.");

DEFINE_string(preprocessor_submodule_name, "PreprocessorSubmodule",
              "preprocessor submodule name in proto");

DEFINE_string(mpc_controller_submodule_name, "MPCControllerSubmodule",
              "MPC controller node name in proto");

DEFINE_string(lat_lon_controller_submodule_name, "LatLonControllerSubmodule",
              "lateral+longitudinal controller node name in proto");

DEFINE_string(postprocessor_submodule_name, "PostprocessorSubmodule",
              "postprocessor submodule name in proto");

DEFINE_bool(is_control_test_mode, false, "True to run control in test mode");
DEFINE_bool(use_preview_speed_for_table, false,
            "True to use preview speed for table lookup");

DEFINE_double(steer_angle_rate, 100.0,
              "Steer angle change rate in percentage.");
DEFINE_bool(enable_gain_scheduler, false,
            "Enable gain scheduler for higher vehicle speed");
DEFINE_bool(set_steer_limit, false, "Set steer limit");

DEFINE_int32(chassis_pending_queue_size, 10, "Max chassis pending queue size");
DEFINE_int32(planning_pending_queue_size, 10,
             "Max planning pending queue size");
DEFINE_int32(planning_status_msg_pending_queue_size, 10,
             "Max planning status message pending queue size");
DEFINE_int32(localization_pending_queue_size, 10,
             "Max localization pending queue size");
DEFINE_int32(pad_msg_pending_queue_size, 10,
             "Max pad message pending queue size");

DEFINE_bool(reverse_heading_control, false, "test vehicle reverse control");

DEFINE_bool(query_forward_time_point_only, false,
            "only use the trajectory point in future");

DEFINE_bool(
    enable_gear_drive_negative_speed_protection, false,
    "Enable estop to prevent following negative speed during gear drive");

DEFINE_bool(use_control_submodules, false,
            "use control submodules instead of controller agent");

DEFINE_bool(enable_input_timestamp_check, false,
            "enable input timestamp check");

DEFINE_int32(max_localization_miss_num, 20, "max localization miss num");

DEFINE_int32(max_chassis_miss_num, 20, "max chassis miss num");

DEFINE_int32(max_planning_miss_num, 20, "max planning miss num");

DEFINE_double(max_acceleration_when_stopped, 0.01,
              "max acceleration when stopped.");

DEFINE_double(max_path_remain_when_stopped, 0.3,
              "max path remain when stopped.");

DEFINE_bool(enable_persistent_estop, true, "enable persistent estop");

DEFINE_double(control_period, 0.01, "control period");

DEFINE_double(soft_estop_brake, 50.0, "soft estop brake.");

DEFINE_double(trajectory_period, 0.1, "trajectory period.");

DEFINE_double(chassis_period, 0.01, "chassis period.");

DEFINE_double(localization_period, 0.01, "localization period.");

DEFINE_double(minimum_speed_resolution, 0.2, "minimum speed resolution.");

DEFINE_double(minimum_speed_protection, 0.1, "minimum speed protection.");

DEFINE_int32(action, 1, "START = 1; RESET = 2; VIN_REQ = 3");

DEFINE_bool(use_vehicle_epb, false, "enable epb brake for vehicle.");

DEFINE_double(pitch_offset_deg, 0.0, "vehicle pitch offset when in horizon.");

DEFINE_bool(is_control_ut_test_mode, false,
            "True to run control in ut test mode");

DEFINE_bool(publish_control_debug_info, false,
            "True to run control in ut test mode");

DEFINE_bool(query_forward_station_point_only, false,
            "only use the trajectory point in future");

DEFINE_bool(use_speed_filter, false, "use speed smooth filter");

DEFINE_bool(use_throttle_filter, false, "use throttle smooth filter");

DEFINE_double(speed_smoothing_factor, 0.05, "speed smooth factor");

DEFINE_double(throttle_smoothing_factor, 0.05, "speed smooth factor");

DEFINE_bool(use_calibration_dimension_equal_check, false,
            "use calibration dimension equal check");

DEFINE_bool(enable_aeb_estop, false,
            "Enable obstacle-based AEB estop in the control component.");
DEFINE_string(aeb_obstacles_topic, "/apollo/perception/obstacles",
              "Perception obstacle topic used by AEB.");
DEFINE_double(aeb_max_obstacle_age_sec, 0.2,
              "Maximum accepted age of an AEB obstacle frame in seconds.");
DEFINE_double(aeb_min_ego_speed_mps, 1.0,
              "Minimum ego speed required for AEB estop.");
DEFINE_double(aeb_front_offset_m, 3.62,
              "Distance from localization reference to the front bumper.");
DEFINE_double(aeb_lane_half_width_m, 1.75,
              "Half-width of the forward AEB corridor in meters.");
DEFINE_double(aeb_min_closing_speed_mps, 0.5,
              "Minimum closing speed required for AEB estop.");
DEFINE_double(aeb_brake_ttc_sec, 3.0,
              "TTC threshold for AEB estop.");
DEFINE_int32(aeb_min_confirmed_frames, 3,
             "Consecutive perception frames required before AEB estop.");
