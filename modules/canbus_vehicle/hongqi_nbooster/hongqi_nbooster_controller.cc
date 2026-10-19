/******************************************************************************
 * Copyright 2023 The Apollo Authors. All Rights Reserved.
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

#include "modules/canbus_vehicle/hongqi_nbooster/hongqi_nbooster_controller.h"

#include <string>

#include "modules/common_msgs/basic_msgs/vehicle_signal.pb.h"

#include "cyber/common/log.h"
#include "cyber/time/time.h"
#include "modules/canbus/common/canbus_gflags.h"
#include "modules/canbus/vehicle/vehicle_controller.h"
#include "modules/canbus_vehicle/hongqi_nbooster/hongqi_nbooster_message_manager.h"
#include "modules/drivers/canbus/can_comm/can_sender.h"
#include "modules/drivers/canbus/can_comm/protocol_data.h"

namespace apollo {
namespace canbus {
namespace hongqi_nbooster {

using ::apollo::common::ErrorCode;
using ::apollo::common::VehicleSignal;
using ::apollo::control::ControlCommand;
using ::apollo::drivers::canbus::ProtocolData;

namespace {
const int32_t kMaxFailAttempt = 10;
const int32_t CHECK_RESPONSE_STEER_UNIT_FLAG = 1;
const int32_t CHECK_RESPONSE_SPEED_UNIT_FLAG = 2;
bool emergency_brake = false;

}  // namespace

void Hongqi_nboosterController::AddSendMessage() {
  can_sender_->AddMessage(Svbnbsreq109::ID, svb_nbs_req_109_, false);
  can_sender_->AddMessage(Svb42c2::ID, svb_4_2c2_, false);
  can_sender_->AddMessage(Svb32c0::ID, svb_3_2c0_, false);
  can_sender_->AddMessage(Svb21a0::ID, svb_2_1a0_, false);
  can_sender_->AddMessage(Svb1108::ID, svb_1_108_, false);
}

ErrorCode Hongqi_nboosterController::Init(
	const VehicleParameter& params,
	CanSender<::apollo::canbus::Hongqi_nbooster> *const can_sender,
  MessageManager<::apollo::canbus::Hongqi_nbooster> *const message_manager) {
  if (is_initialized_) {
    AINFO << "Hongqi_nboosterController has already been initiated.";
    return ErrorCode::CANBUS_ERROR;
  }

  vehicle_params_.CopyFrom(
      common::VehicleConfigHelper::Instance()->GetConfig().vehicle_param());
  params_.CopyFrom(params);
  if (!params_.has_driving_mode()) {
    AERROR << "Vehicle conf pb not set driving_mode.";
    return ErrorCode::CANBUS_ERROR;
  }

  if (can_sender == nullptr) {
    AERROR << "Canbus sender is null.";
    return ErrorCode::CANBUS_ERROR;
  }
  can_sender_ = can_sender;

  if (message_manager == nullptr) {
    AERROR << "protocol manager is null.";
    return ErrorCode::CANBUS_ERROR;
  }
  message_manager_ = message_manager;

  // sender part
  svb_nbs_req_109_ = dynamic_cast<Svbnbsreq109*>
          (message_manager_->GetMutableProtocolDataById(Svbnbsreq109::ID));
  if (svb_nbs_req_109_ == nullptr) {
     AERROR << "Svbnbsreq109 does not exist in the Hongqi_nboosterMessageManager!";
     return ErrorCode::CANBUS_ERROR;
  }

  svb_4_2c2_ = dynamic_cast<Svb42c2*>
          (message_manager_->GetMutableProtocolDataById(Svb42c2::ID));
  if (svb_4_2c2_ == nullptr) {
     AERROR << "Svb42c2 does not exist in the Hongqi_nboosterMessageManager!";
     return ErrorCode::CANBUS_ERROR;
  }

  svb_3_2c0_ = dynamic_cast<Svb32c0*>
          (message_manager_->GetMutableProtocolDataById(Svb32c0::ID));
  if (svb_3_2c0_ == nullptr) {
     AERROR << "Svb32c0 does not exist in the Hongqi_nboosterMessageManager!";
     return ErrorCode::CANBUS_ERROR;
  }

  svb_2_1a0_ = dynamic_cast<Svb21a0*>
          (message_manager_->GetMutableProtocolDataById(Svb21a0::ID));
  if (svb_2_1a0_ == nullptr) {
     AERROR << "Svb21a0 does not exist in the Hongqi_nboosterMessageManager!";
     return ErrorCode::CANBUS_ERROR;
  }

  svb_1_108_ = dynamic_cast<Svb1108*>
          (message_manager_->GetMutableProtocolDataById(Svb1108::ID));
  if (svb_1_108_ == nullptr) {
     AERROR << "Svb1108 does not exist in the Hongqi_nboosterMessageManager!";
     return ErrorCode::CANBUS_ERROR;
  }

  AddSendMessage();

  AINFO << "Hongqi_nboosterController is initialized.";

  is_initialized_ = true;
  return ErrorCode::OK;
}

Hongqi_nboosterController::~Hongqi_nboosterController() {}

bool Hongqi_nboosterController::Start() {
  if (!is_initialized_) {
    AERROR << "Hongqi_nboosterController has NOT been initiated.";
    return false;
  }
  const auto& update_func = [this] { SecurityDogThreadFunc(); };
  thread_.reset(new std::thread(update_func));

  return true;
}

void Hongqi_nboosterController::Stop() {
  if (!is_initialized_) {
    AERROR << "Hongqi_nboosterController stops or starts improperly!";
    return;
  }

  if (thread_ != nullptr && thread_->joinable()) {
    thread_->join();
    thread_.reset();
    AINFO << "Hongqi_nboosterController stopped.";
  }
}

Chassis Hongqi_nboosterController::chassis() {
  chassis_.Clear();
  Hongqi_nbooster chassis_detail = GetNewRecvChassisDetail();;

  // 1, 2
  // if (driving_mode() == Chassis::EMERGENCY_MODE) {
  //   set_chassis_error_code(Chassis::NO_ERROR);
  // }

  chassis_.set_driving_mode(driving_mode());
  chassis_.set_error_code(chassis_error_code());

  // 3
  chassis_.set_engine_started(true);
  // 4 engine rpm ch has no engine rpm
  // chassis_.set_engine_rpm(0);
  // 5 wheel spd
  // 6 speed_mps
  if (chassis_detail.has_abs_1_b4() &&
      chassis_detail.abs_1_b4().has_abs_vehiclespeed()) {
        const ABS_1_b4::ABS_DrivingDirectionType direction = chassis_detail.abs_1_b4().abs_drivingdirection();
        const double speed = chassis_detail.abs_1_b4().abs_vehiclespeed() * 1.0 / 3.6;
        switch (direction) {
              case ABS_1_b4::ABS_DRIVING_DIRECTION_FORWARD:         // forward
                  chassis_.set_speed_mps(std::abs(static_cast<float>(speed)));
                  break;
              case ABS_1_b4::ABS_DRIVING_DIRECTION_BACKWARD:         // backward
                  chassis_.set_speed_mps(std::abs(static_cast<float>(-speed)));
                  break;
              case ABS_1_b4::ABS_DRIVING_DIRECTION_STOP:         // stop
                  chassis_.set_speed_mps(std::abs(static_cast<float>(0)));
                  break;
              default:        // default as forward
                  chassis_.set_speed_mps(std::abs(static_cast<float>(speed)));
                  AERROR << "error happened in abs_1_b4 can message: speed direction error";
                  break;
        }
  } else {
    chassis_.set_speed_mps(0);
  }
  // 7 no odometer
  // chassis_.set_odometer_m(0);
  // 8 no fuel. do not set;
  // chassis_.set_fuel_range_m(0);
  // 9 throttle
  if(chassis_detail.has_hcu_4_122() && 
      chassis_detail.hcu_4_122().has_hcu_accelerationpedalposition()) {
        chassis_.set_throttle_percentage(static_cast<float>(
          chassis_detail.hcu_4_122().hcu_accelerationpedalposition()));
  } else {
    chassis_.set_throttle_percentage(0);
  }
  // chassis_.set_throttle_percentage(static_cast<float>(_throttle_percentage));

  // throttle sender cmd
  if(chassis_detail.has_svb_nbs_req_109() && 
      chassis_detail.svb_nbs_req_109().has_svb_taraccepedalposition_req()) {
        chassis_.set_throttle_percentage_cmd(static_cast<float>(
          chassis_detail.svb_nbs_req_109().svb_taraccepedalposition_req()));
  } else {
    chassis_.set_throttle_percentage_cmd(0);
  }
  // 10 brake
  if(chassis_detail.has_nbs_status_1_10d() && 
      chassis_detail.nbs_status_1_10d().has_nbs_brktorqact_percent()) {
        chassis_.set_brake_percentage(static_cast<float>(
          chassis_detail.nbs_status_1_10d().nbs_brktorqact_percent()));
  } else {
    chassis_.set_brake_percentage(0);
  }
  // chassis_.set_brake_percentage(static_cast<float>(_brake_percentage));

  // brake sender cmd
  if(chassis_detail.has_svb_nbs_req_109() && 
      chassis_detail.svb_nbs_req_109().has_svb_brkpelpos_decrequest()) {
        chassis_.set_brake_percentage_cmd(static_cast<float>(
          chassis_detail.svb_nbs_req_109().svb_brkpelpos_decrequest()));
  } else {
    chassis_.set_brake_percentage_cmd(0);
  }  
  // 23, previously 11 gear
  if (chassis_detail.has_hcu_14_248() &&
    chassis_detail.hcu_14_248().has_hcu_leverinfo()) {
  Chassis::GearPosition gear_pos = Chassis::GEAR_INVALID;
    if (chassis_detail.hcu_14_248().hcu_leverinfo() ==
      HCU_14_248::HCU_LEVERINFO_INVALID_VALUE) {
      gear_pos = Chassis::GEAR_INVALID;
      }
      if (chassis_detail.hcu_14_248().hcu_leverinfo() ==
        HCU_14_248::HCU_LEVERINFO_N) {
        gear_pos = Chassis::GEAR_NEUTRAL;
      }
      if (chassis_detail.hcu_14_248().hcu_leverinfo() ==
        HCU_14_248::HCU_LEVERINFO_R) {
        gear_pos = Chassis::GEAR_REVERSE;
      }
      if (chassis_detail.hcu_14_248().hcu_leverinfo() ==
        HCU_14_248::HCU_LEVERINFO_D) {
        gear_pos = Chassis::GEAR_DRIVE;
      }
      if (chassis_detail.hcu_14_248().hcu_leverinfo() ==
        HCU_14_248::HCU_LEVERINFO_P) {
        gear_pos = Chassis::GEAR_PARKING;
      }
      chassis_.set_gear_location(gear_pos);
  } else {
    chassis_.set_gear_location(Chassis::GEAR_NONE);
  }
    // 12 steering
    if (chassis_detail.has_sas_1_96() &&
      chassis_detail.sas_1_96().has_sas_steeringwheelangle()) {
        SAS_1_96::SAS_SteeringWheelAngleSignType sign = 
            chassis_detail.sas_1_96().sas_steeringwheelanglesign();
      if (SAS_1_96::SAS_STEERING_WHEEL_ANGLE_SIGN_LEFT == sign) {
        chassis_.set_steering_percentage(static_cast<float>(
          chassis_detail.sas_1_96().sas_steeringwheelangle() * 100.0 /
          vehicle_params_.max_steer_angle() * M_PI / 180));
      } else {
        chassis_.set_steering_percentage(static_cast<float>(
          chassis_detail.sas_1_96().sas_steeringwheelangle() * -100.0 /
          vehicle_params_.max_steer_angle() * M_PI / 180));
      } 
  } else {
    chassis_.set_steering_percentage(0);
  }
  // steering sender cmd
  if (chassis_detail.has_svb_1_108() &&
      chassis_detail.svb_1_108().has_svb_steeringanglerequest()) {
        if(SVB_1_108::SVB_STEERING_ANGLE_SIGN_REQUEST_LEFT == 
          chassis_detail.svb_1_108().svb_steeringanglesignrequest()){
            chassis_.set_steering_percentage_cmd(static_cast<float>(
              chassis_detail.svb_1_108().svb_steeringanglerequest()));
          } else {
            chassis_.set_steering_percentage_cmd(static_cast<float>(
              -chassis_detail.svb_1_108().svb_steeringanglerequest()));
          }
  } else {
    chassis_.set_steering_percentage_cmd(0);
  }
  // 13 parking brake
  if (chassis_detail.has_epb_1_104() &&
      chassis_detail.epb_1_104().has_epb_status()) {
        EPB_1_104::EPB_StatusType epb = chassis_detail.epb_1_104().epb_status();
        switch (epb)
        {
          case EPB_1_104::EPB_STATUS_RELEASED:
            chassis_.set_parking_brake(false);
            break;
          case EPB_1_104::EPB_STATUS_LOCKED:
            chassis_.set_parking_brake(true);
            break;
          case EPB_1_104::EPB_STATUS_RELEASING:
            chassis_.set_parking_brake(false);
            break;
          case EPB_1_104::EPB_STATUS_LOCKING:
            chassis_.set_parking_brake(true);
            break;
          default:
            break;
        }
  } else {
    chassis_.set_parking_brake(false);
  }
  // 14 battery soc
  if (chassis_detail.has_cds_2_10b() &&
      chassis_detail.cds_2_10b().has_hcu_batterypacksoc()) {
    chassis_.set_battery_soc_percentage(
      chassis_detail.cds_2_10b().hcu_batterypacksoc());
  } else {
    chassis_.set_battery_soc_percentage(0);
  }

  // 20 add checkresponse signal
  if (chassis_detail.has_cds_1_10a() &&
      chassis_detail.cds_1_10a().has_cds_steeringautocontrolst()) {
        if ((chassis_detail.cds_1_10a().cds_steeringautocontrolst() == 
            CDS_1_10a::CDS_STEERING_AUTOCONTROL_ST_ACTIVE) && 
            (chassis_detail.cds_1_10a().cds_steeringtakeoverst() != 
            CDS_1_10a::CDS_STEERING_TAKEOVER_ST_ACTIVE)){
              chassis_.mutable_check_response()->set_is_eps_online(
                chassis_detail.cds_1_10a().cds_steeringautocontrolst() == 
            CDS_1_10a::CDS_STEERING_AUTOCONTROL_ST_ACTIVE);
        }
  }
  if (chassis_detail.has_nbs_status_1_10d() &&
      chassis_detail.nbs_status_1_10d().has_nbs_brkautocontrol_st()) {
    chassis_.mutable_check_response()->set_is_esp_online(
      chassis_detail.nbs_status_1_10d().nbs_brkautocontrol_st() == 
      NBS_STATUS_1_10d::NBS_BRK_AUTO_CONTROL_ST_BRKPELPOS_CONTROL_ACTIVE);
  }
  if (chassis_detail.has_hcu_torque_status_tonbs_191() &&
      chassis_detail.hcu_torque_status_tonbs_191().has_hcu_accecontrol_status()) {
    chassis_.mutable_check_response()->set_is_vcu_online(
      chassis_detail.hcu_torque_status_tonbs_191().hcu_accecontrol_status() == 
      HCU_Torque_Status_ToNBS_191::HCU_ACCECONTROL_STATUS_VIRTUAL_PEDAL_MODE);
  }
  // check chassis error
  if (CheckChassisError()) {
    chassis_.mutable_engage_advice()->set_advice(
        apollo::common::EngageAdvice::DISALLOW_ENGAGE);
    chassis_.mutable_engage_advice()->set_reason(
        "Chassis has some fault, please check the chassis_detail.");
  }

  // check the chassis detail lost
  if (is_chassis_communication_error_) {
    chassis_.mutable_engage_advice()->set_advice(
        apollo::common::EngageAdvice::DISALLOW_ENGAGE);
    chassis_.mutable_engage_advice()->set_reason(
        "hongqi_nbooster chassis detail is lost! Please check the communication error.");
    set_chassis_error_code(Chassis::CHASSIS_CAN_LOST);
    set_driving_mode(Chassis::EMERGENCY_MODE);
  }

  /* ADD YOUR OWN CAR CHASSIS OPERATION
  // 14 battery soc
  // 16 sonor list
  // 17 set vin
  // 18,19 bumper event
  // 20 add checkresponse signal
  */

  return chassis_;
}

void Hongqi_nboosterController::Emergency() {
  set_driving_mode(Chassis::EMERGENCY_MODE);
  ResetProtocol();
}

ErrorCode Hongqi_nboosterController::EnableAutoMode() {
  if (driving_mode() == Chassis::COMPLETE_AUTO_DRIVE) {
    AINFO << "already in COMPLETE_AUTO_DRIVE mode";
    return ErrorCode::OK;
  }
  // set enable
  svb_nbs_req_109_->set_svb_taraccepedalposition_reqst(1);
  svb_1_108_->set_svb_steeringrequestst(true);
  svb_nbs_req_109_->set_svb_decmode_request(1);

  svb_1_108_->set_svb_steeringanglerequest(0.0);
  svb_1_108_->set_svb_steeringanglesignrequest(2);
  svb_1_108_->set_svb_steeringspeedsignrequest(2);
  // if (real_angle > 0) { // left
  //     svb_1_108->set_svb_steeringanglesignrequest(1);
  //     svb_1_108->set_svb_steeringspeedsignrequest(1);

  // } else { // right
  //     svb_1_108->set_svb_steeringanglesignrequest(2);
  //     svb_1_108->set_svb_steeringspeedsignrequest(2);
  //     svb_1_108->set_svb_steeringanglerequest(fabs(real_angle));
  // }
  
  can_sender_->Update();
  const int32_t flag =
      CHECK_RESPONSE_STEER_UNIT_FLAG | CHECK_RESPONSE_SPEED_UNIT_FLAG;
  if (!CheckResponse(flag, true)) {
    AERROR << "Failed to switch to COMPLETE_AUTO_DRIVE mode. Please check the "
              "emergency button or chassis.";
    Emergency();
    set_chassis_error_code(Chassis::CHASSIS_ERROR);
    return ErrorCode::CANBUS_ERROR;
  }
  set_driving_mode(Chassis::COMPLETE_AUTO_DRIVE);
  AINFO << "Switch to COMPLETE_AUTO_DRIVE mode ok.";
  return ErrorCode::OK;
}

ErrorCode Hongqi_nboosterController::DisableAutoMode() {
  ResetProtocol();
  can_sender_->Update();
  set_driving_mode(Chassis::COMPLETE_MANUAL);
  set_chassis_error_code(Chassis::NO_ERROR);
  AINFO << "Switch to COMPLETE_MANUAL ok.";
  return ErrorCode::OK;
}

ErrorCode Hongqi_nboosterController::EnableSteeringOnlyMode() {
  if (driving_mode() == Chassis::COMPLETE_AUTO_DRIVE ||
      driving_mode() == Chassis::AUTO_STEER_ONLY) {
    set_driving_mode(Chassis::AUTO_STEER_ONLY);
    AINFO << "Already in AUTO_STEER_ONLY mode.";
    return ErrorCode::OK;
  }
  AFATAL << "SteeringOnlyMode is not supported in Hongqi_nbooster!";
  return ErrorCode::CANBUS_ERROR;
}

ErrorCode Hongqi_nboosterController::EnableSpeedOnlyMode() {
  if (driving_mode() == Chassis::COMPLETE_AUTO_DRIVE ||
      driving_mode() == Chassis::AUTO_SPEED_ONLY) {
    set_driving_mode(Chassis::AUTO_SPEED_ONLY);
    AINFO << "Already in AUTO_SPEED_ONLY mode";
    return ErrorCode::OK;
  }
  AFATAL << "SpeedOnlyMode is not supported in Hongqi_nbooster!";
  return ErrorCode::CANBUS_ERROR;
}

// NEUTRAL, REVERSE, DRIVE
void Hongqi_nboosterController::Gear(Chassis::GearPosition gear_position) {
  if (driving_mode() != Chassis::COMPLETE_AUTO_DRIVE &&
      driving_mode() != Chassis::AUTO_SPEED_ONLY) {
    AINFO << "This drive mode no need to set gear.";
    return;
  }
  switch (gear_position) {
    case Chassis::GEAR_NEUTRAL: {
      svb_1_108_->set_svb_gearrequest(3);
      // gear_command_103_->set_gear_target(Gear_command_103::GEAR_TARGET_NEUTRAL);
      break;
    }
    case Chassis::GEAR_REVERSE: {
      svb_1_108_->set_svb_gearrequest(2);
      // gear_command_103_->set_gear_target(Gear_command_103::GEAR_TARGET_REVERSE);
      break;
    }
    case Chassis::GEAR_DRIVE: {
      svb_1_108_->set_svb_gearrequest(4);
      // gear_command_103_->set_gear_target(Gear_command_103::GEAR_TARGET_DRIVE);
      break;
    }
    case Chassis::GEAR_PARKING: {
      svb_1_108_->set_svb_gearrequest(1);
      // if (car_status->epb().parking_brake_status() == PbEpb::PBRAKE_ON || 
      //     car_status->epb().parking_brake_status() == PbEpb::PBRAKE_TRANSITION) {
      //     _svb_2_1a0->set_svb_epbrequest(0);
      //     _svb_1_108->set_svb_gearrequest(1);
      // } else {
      //     LOG_INFO << "Before switch to gear parking, pull up epb";
      //     _svb_2_1a0->set_svb_epbrequest(2);
      // }
      // gear_command_103_->set_gear_target(Gear_command_103::GEAR_TARGET_PARK);
      break;
    }
    case Chassis::GEAR_INVALID: {
      svb_1_108_->set_svb_gearrequest(3);
      // gear_command_103_->set_gear_target(Gear_command_103::GEAR_TARGET_NEUTRAL);
      break;
    }
    default: {
      svb_1_108_->set_svb_gearrequest(3);
      // gear_command_103_->set_gear_target(Gear_command_103::GEAR_TARGET_NEUTRAL);
      break;
    }
  }
  const double shift_brake = 20;
  svb_nbs_req_109_->set_svb_taraccepedalposition_req(0.0);
  svb_nbs_req_109_->set_svb_brkpelpos_decrequest(shift_brake);
}

// brake with pedal
// pedal:0.00~99.99, unit:percentage
void Hongqi_nboosterController::Brake(double pedal) {
  // double real_value = vehicle_params_.max_acceleration() * acceleration / 100;
  // TODO(All) :  Update brake value based on mode
  if (driving_mode() != Chassis::COMPLETE_AUTO_DRIVE &&
      driving_mode() != Chassis::AUTO_SPEED_ONLY) {
    AINFO << "The current drive mode does not need to set brake pedal.";
    return;
  }
  if (!emergency_brake) {
    _brake_percentage = pedal;
    svb_nbs_req_109_->set_svb_brkpelpos_decrequest(pedal);
    // normaly will not work, mock data for regulator
    // _svb_1_108->set_svb_decelerationrequest(-pedal * 0.1 + 10.28);
  } else {
    _brake_percentage = 0.0;
  }
}

// drive with pedal
// pedal:0.0~99.9 unit:percentage
void Hongqi_nboosterController::Throttle(double pedal) {
  if (driving_mode() != Chassis::COMPLETE_AUTO_DRIVE &&
      driving_mode() != Chassis::AUTO_SPEED_ONLY) {
    AINFO << "The current drive mode does not need to set throttle pedal.";
    return;
  }
  if (!emergency_brake) {
    _throttle_percentage = pedal;
    svb_nbs_req_109_->set_svb_taraccepedalposition_req(pedal);
    // normaly will not work, mock data for regulator
    // _svb_1_108->set_svb_accelerationrequest(pedal * 0.05);  }
  } else {
    _throttle_percentage = 0.0;
  }
}
// confirm the car is driven by acceleration command instead of
// throttle/brake pedal drive with acceleration/deceleration acc:-7.0 ~ 5.0,
// unit:m/s^2
void Hongqi_nboosterController::Acceleration(double acc) {
  if (driving_mode() != Chassis::COMPLETE_AUTO_DRIVE &&
      driving_mode() != Chassis::AUTO_SPEED_ONLY) {
    AINFO << "The current drive mode does not need to set acceleration.";
    return;
  }
  /* ADD YOUR OWN CAR CHASSIS OPERATION
  // TODO(ALL): CHECK YOUR VEHICLE WHETHER SUPPORT THIS DRIVE MODE
  */
}

// confirm the car is driven by speed command
// speed:-xx.0~xx.0, unit:m/s
void Hongqi_nboosterController::Speed(double speed) {
  if (driving_mode() != Chassis::COMPLETE_AUTO_DRIVE &&
      driving_mode() != Chassis::AUTO_SPEED_ONLY) {
    AINFO << "The current drive mode does not need to set speed.";
    return;
  }
  /* ADD YOUR OWN CAR CHASSIS OPERATION
  // TODO(ALL): CHECK YOUR VEHICLE WHETHER SUPPORT THIS DRIVE MODE
  */
}

// hongqi_nbooster default, +470 ~ -470 or other, left:+, right:-
// need to be compatible with control module, so reverse
// steering with steering angle
// angle:99.99~0.00~-99.99, unit:deg, left:+, right:-
void Hongqi_nboosterController::Steer(double angle) {
  if (driving_mode() != Chassis::COMPLETE_AUTO_DRIVE &&
      driving_mode() != Chassis::AUTO_STEER_ONLY) {
    AINFO << "The current driving mode does not need to set steer.";
    return;
  }
  const double real_angle =
      vehicle_params_.max_steer_angle() / M_PI * 180 * angle / 100.0;

  if (!emergency_brake) {
    svb_1_108_->set_svb_steeringanglerequest(fabs(real_angle));
    svb_1_108_->set_svb_steeringspeedrequest(fabs(vehicle_params_.max_steer_angle_rate()));
    if (real_angle > 0) { // left
      svb_1_108_->set_svb_steeringanglesignrequest(1);
      svb_1_108_->set_svb_steeringspeedsignrequest(1);
  
    } else { // right
      svb_1_108_->set_svb_steeringanglesignrequest(2);
      svb_1_108_->set_svb_steeringspeedsignrequest(2);
      svb_1_108_->set_svb_steeringanglerequest(fabs(real_angle));
    }
  }
  // double real_angle = static_cast<double>(_vehicle_conf.max_steer_angle() * angle / 100);
  // _svb_1_108->set_svb_steeringanglerequest(fabs(real_angle));

  // if (real_angle > 0) { // left
  //     _svb_1_108->set_svb_steeringanglesignrequest(1);
  //     _svb_1_108->set_svb_steeringspeedsignrequest(1);

  // } else { // right
  //     _svb_1_108->set_svb_steeringanglesignrequest(2);
  //     _svb_1_108->set_svb_steeringspeedsignrequest(2);
  //     _svb_1_108->set_svb_steeringanglerequest(fabs(real_angle));
  // }

}

// hongqi_nbooster default, steering with new angle and angle speed
// angle:99.99~0.00~-99.99, unit:deg, left:+, right:-
// angle_spd:0.00~99.99, unit:deg/s
void Hongqi_nboosterController::Steer(double angle, double angle_spd) {
  if (driving_mode() != Chassis::COMPLETE_AUTO_DRIVE &&
      driving_mode() != Chassis::AUTO_STEER_ONLY) {
    AINFO << "The current driving mode does not need to set steer.";
    return;
  }
  const double real_angle = vehicle_params_.max_steer_angle() / M_PI * 180 * angle / 100.0;
  double real_angle_spd = vehicle_params_.max_steer_angle_rate() * angle_spd / 100;
  real_angle_spd = static_cast<double>(real_angle_spd > vehicle_params_.max_steer_angle_rate() ? 
                  vehicle_params_.max_steer_angle_rate() : (real_angle_spd < 0.0 ? 0.0 : real_angle_spd));
  // double real_angle_spd = static_cast<double>(ProtocolData::BoundedValue(0.0,
  //                         vehicle_params_.max_steer_angle_rate(),
  //                         vehicle_params_.max_steer_angle_rate() * angle_spd / 100));

  if (!emergency_brake) {
    svb_1_108_->set_svb_steeringanglerequest(fabs(real_angle));
    svb_1_108_->set_svb_steeringspeedrequest(fabs(real_angle_spd));

  if (real_angle > 0) { // left
    svb_1_108_->set_svb_steeringanglesignrequest(1);
    svb_1_108_->set_svb_steeringspeedsignrequest(1);

  } else { // right
    svb_1_108_->set_svb_steeringanglesignrequest(2);
    svb_1_108_->set_svb_steeringspeedsignrequest(2);
    svb_1_108_->set_svb_steeringanglerequest(fabs(real_angle));
  }
}
// double real_angle = static_cast<double>(_vehicle_conf.max_steer_angle() * angle / 100);
// _svb_1_108->set_svb_steeringanglerequest(fabs(real_angle));

// if (real_angle > 0) { // left
//     _svb_1_108->set_svb_steeringanglesignrequest(1);
//     _svb_1_108->set_svb_steeringspeedsignrequest(1);

// } else { // right
//     _svb_1_108->set_svb_steeringanglesignrequest(2);
//     _svb_1_108->set_svb_steeringspeedsignrequest(2);
//     _svb_1_108->set_svb_steeringanglerequest(fabs(real_angle));
// }
}

void Hongqi_nboosterController::SetEpbBreak(const ControlCommand& command) {
  if (command.parking_brake()) {
    svb_2_1a0_->set_svb_epbrequest(2);  // request epb
    // _back_svb_2_1a0->set_svb_epbrequest(2);
  } else {
    svb_2_1a0_->set_svb_epbrequest(1);
    // _back_svb_2_1a0->set_svb_epbrequest(1);  // cancel epb
  }
}

void Hongqi_nboosterController::SetBeam(const VehicleSignal& vehicle_signal) {
  if (vehicle_signal.high_beam()) {
    // None
  } else if (vehicle_signal.low_beam()) {
    // None
  } else {
    // None
  }
}

void Hongqi_nboosterController::SetHorn(const VehicleSignal& vehicle_signal) {
  if (vehicle_signal.horn()) {
    // None
  } else {
    // None
  }
}

void Hongqi_nboosterController::SetTurningSignal(const VehicleSignal& vehicle_signal) {
  // Set Turn Signal
  /* ADD YOUR OWN CAR CHASSIS OPERATION
  auto signal = vehicle_signal.turn_signal();
  if (signal == common::VehicleSignal::TURN_LEFT) {

  } else if (signal == common::VehicleSignal::TURN_RIGHT) {

  } else {

  }
  */
}

ErrorCode Hongqi_nboosterController::HandleCustomOperation(
    const external_command::ChassisCommand& command) {
  return ErrorCode::OK;
}

bool Hongqi_nboosterController::VerifyID() {
  if (!CheckVin()) {
    AERROR << "Failed to get the vin. Get vin again.";
    GetVin();
    return false;
  } else {
    ResetVin();
    return true;
  }
}

bool Hongqi_nboosterController::CheckVin() {
  /* ADD YOUR OWN CAR CHASSIS OPERATION
  if (chassis_.vehicle_id().vin().size() >= 7) {
    AINFO << "Vin check success! Vehicel vin is "
          << chassis_.vehicle_id().vin();
    return true;
  } else {
    AINFO << "Vin check failed! Current vin size is "
          << chassis_.vehicle_id().vin().size();
    return false;
  }
  */
  return false;
}

void Hongqi_nboosterController::GetVin() {
  // Get vin from vehicle if exist
  /* ADD YOUR OWN CAR CHASSIS OPERATION
  vehicle_mode_command_116_->set_vin_req_cmd(
      Vehicle_mode_command_116::VIN_REQ_CMD_VIN_REQ_ENABLE);
  AINFO << "Get vin";
  can_sender_->Update();
  */
}

void Hongqi_nboosterController::ResetVin() {
  // Reset vin from vehicle if exist
  /* ADD YOUR OWN CAR CHASSIS OPERATION
  vehicle_mode_command_116_->set_vin_req_cmd(
      Vehicle_mode_command_116::VIN_REQ_CMD_VIN_REQ_DISABLE);
  AINFO << "Reset vin";
  can_sender_->Update();
  */
}

void Hongqi_nboosterController::ResetProtocol() {
  message_manager_->ResetSendMessages();
}

bool Hongqi_nboosterController::CheckChassisError() {
  if (is_chassis_communication_error_) {
    AERROR_EVERY(100) << "ChassisDetail has no hongqi_nbooster vehicle info.";
    return false;
  }

  Hongqi_nbooster chassis_detail = GetNewRecvChassisDetail();

  // steer fault
  if (chassis_detail.has_cds_1_10a()) {
    if ((CDS_1_10a::CDS_STEERING_FAILURE_ST_EPS_FAIL ==
        chassis_detail.cds_1_10a().cds_steeringfailurest()) || 
        (CDS_1_10a::CDS_STEERING_FAILURE_ST_SAS_FAIL ==
        chassis_detail.cds_1_10a().cds_steeringfailurest()) || 
        (CDS_1_10a::CDS_STEERING_FAILURE_ST_STEER_TEMP_FAIL ==
        chassis_detail.cds_1_10a().cds_steeringfailurest())) {
          AERROR_EVERY(100) << "Chassis has steer system fault.";
          return true;
    }
  }
  // drive fault
  if (chassis_detail.has_cds_1_10a()) {
    if ((CDS_1_10a::CDS_DRIVING_FAILURE_ST_HCU_FAIL ==
        chassis_detail.cds_1_10a().cds_drivingfailurest()) || 
        (CDS_1_10a::CDS_DRIVING_FAILURE_ST_SPD_SENSOR_FAIL ==
        chassis_detail.cds_1_10a().cds_drivingfailurest()) || 
        (CDS_1_10a::CDS_DRIVING_FAILURE_ST_YRS_FAIL ==
        chassis_detail.cds_1_10a().cds_drivingfailurest()) || 
        (CDS_1_10a::CDS_DRIVING_FAILURE_ST_SPD_SENSOR_DEGRADE ==
        chassis_detail.cds_1_10a().cds_drivingfailurest())) {
          AERROR_EVERY(100) << "Chassis has drive system fault.";
          return true;
    }
  }
  // brake fault
  if (chassis_detail.has_cds_1_10a()) {
    if ((CDS_1_10a::CDS_BRAKING_FAILURE_ST_ESP_FAIL ==
        chassis_detail.cds_1_10a().cds_brakingfailurest()) || 
        (CDS_1_10a::CDS_BRAKING_FAILURE_ST_SPD_SENSOR_FAIL ==
        chassis_detail.cds_1_10a().cds_brakingfailurest()) || 
        (CDS_1_10a::CDS_BRAKING_FAILURE_ST_YRS_FAIL ==
        chassis_detail.cds_1_10a().cds_brakingfailurest()) || 
        (CDS_1_10a::CDS_BRAKING_FAILURE_ST_HCU_FAIL ==
        chassis_detail.cds_1_10a().cds_brakingfailurest()) || 
        (CDS_1_10a::CDS_BRAKING_FAILURE_ST_ESP_AND_HCU_FAIL ==
        chassis_detail.cds_1_10a().cds_brakingfailurest()) || 
        (CDS_1_10a::CDS_BRAKING_FAILURE_ST_SPD_SENSOR_DEGRADE ==
        chassis_detail.cds_1_10a().cds_brakingfailurest())) {
          AERROR_EVERY(100) << "Chassis has brake system fault.";
          return true;
    }
  }
  // system fault
  // if (chassis_detail.has_cds_1_10a()) {
  //   if ((CDS_1_10a::CDS_SYSTEM_FAILURE_ST_LOSE_SVB1 ==
  //       chassis_detail.cds_1_10a().cds_systemfailurest()) || 
  //       (CDS_1_10a::CDS_SYSTEM_FAILURE_ST_LOSE_SVB2 ==
  //       chassis_detail.cds_1_10a().cds_systemfailurest()) || 
  //       (CDS_1_10a::CDS_SYSTEM_FAILURE_ST_LOSE_SVB1_AND_LOSE_SVB2 ==
  //       chassis_detail.cds_1_10a().cds_systemfailurest())) {
  //         AERROR_EVERY(100) << "Chassis has system fault.";
  //         return true;
  //   }
  // }
  // NBS fault
  if (chassis_detail.has_nbs_status_1_10d()) {
    if ((NBS_STATUS_1_10d::NBS_FAULTST_FAULT_LEVEL_1 ==
        chassis_detail.nbs_status_1_10d().nbs_faultst()) || 
        (NBS_STATUS_1_10d::NBS_FAULTST_FAULT_LEVEL_2 ==
        chassis_detail.nbs_status_1_10d().nbs_faultst()) || 
        (NBS_STATUS_1_10d::NBS_FAULTST_FAULT_LEVEL_3 ==
        chassis_detail.nbs_status_1_10d().nbs_faultst()) || 
        (NBS_STATUS_1_10d::NBS_FAULTST_FAULT_LEVEL_4 ==
        chassis_detail.nbs_status_1_10d().nbs_faultst()) || 
        (NBS_STATUS_1_10d::NBS_FAULTST_FAULT_LEVEL_5 ==
        chassis_detail.nbs_status_1_10d().nbs_faultst()) || 
        (NBS_STATUS_1_10d::NBS_FAULTST_FAULT_LEVEL_6 ==
        chassis_detail.nbs_status_1_10d().nbs_faultst())) {
          AERROR_EVERY(100) << "Chassis has NBS system fault.";
          return true;
    }
  }
// // steer fault
// bool steer_fault = car_status->safety().steering_system_fault();
// // brake fault
// bool brake_fault = car_status->safety().braking_system_fault();
// // throttle fault
// bool throttle_fault = car_status->safety().driving_system_fault();

  return false;
}

void Hongqi_nboosterController::SecurityDogThreadFunc() {
  int32_t vertical_ctrl_fail = 0;
  int32_t horizontal_ctrl_fail = 0;

  if (can_sender_ == nullptr) {
    AERROR << "Failed to run SecurityDogThreadFunc() because can_sender_ is "
              "nullptr.";
    return;
  }
  while (!can_sender_->IsRunning()) {
    std::this_thread::yield();
  }

  std::chrono::duration<double, std::micro> default_period{50000};
  int64_t start = 0;
  int64_t end = 0;
  while (can_sender_->IsRunning()) {
    start = ::apollo::cyber::Time::Now().ToMicrosecond();
    const Chassis::DrivingMode mode = driving_mode();
    bool emergency_mode = false;
    emergency_brake = false;

    // 1. horizontal control check
    if ((mode == Chassis::COMPLETE_AUTO_DRIVE ||
         mode == Chassis::AUTO_STEER_ONLY) &&
        !CheckResponse(CHECK_RESPONSE_STEER_UNIT_FLAG, false)) {
      ++horizontal_ctrl_fail;
      if (horizontal_ctrl_fail >= kMaxFailAttempt) {
        emergency_mode = true;
        AERROR << "Driving_mode is into emergency by steer manual intervention";
        set_chassis_error_code(Chassis::MANUAL_INTERVENTION);
      }
    } else {
      horizontal_ctrl_fail = 0;
    }

    // 2. vertical control check
    if ((mode == Chassis::COMPLETE_AUTO_DRIVE ||
         mode == Chassis::AUTO_SPEED_ONLY) &&
        !CheckResponse(CHECK_RESPONSE_SPEED_UNIT_FLAG, false)) {
      ++vertical_ctrl_fail;
      if (vertical_ctrl_fail >= kMaxFailAttempt) {
        emergency_mode = true;
        AERROR << "Driving_mode is into emergency by speed manual intervention";
        set_chassis_error_code(Chassis::MANUAL_INTERVENTION);
      }
    } else {
      vertical_ctrl_fail = 0;
    }

    // 3. chassis fault check
    if (CheckChassisError()) {
      set_chassis_error_code(Chassis::CHASSIS_ERROR);
      emergency_mode = true;
      if (chassis_.speed_mps() > 0.3) {
        emergency_brake = true;
      }
    }

    // process emergency_mode
    if (emergency_mode && mode != Chassis::EMERGENCY_MODE) {
      set_driving_mode(Chassis::EMERGENCY_MODE);
      if (emergency_brake) {
        svb_1_108_->set_svb_steeringanglerequest(0);
        svb_nbs_req_109_->set_svb_brkpelpos_decrequest(40);
        svb_nbs_req_109_->set_svb_taraccepedalposition_req(0);
        std::this_thread::sleep_for(
            std::chrono::duration<double, std::milli>(3000));
      }
      message_manager_->ResetSendMessages();
      can_sender_->Update();
      emergency_brake = false;
    }

    // recove error code
    if (!emergency_mode && !is_chassis_communication_error_ &&
        mode == Chassis::EMERGENCY_MODE) {
      set_chassis_error_code(Chassis::NO_ERROR);
    }

    end = ::apollo::cyber::Time::Now().ToMicrosecond();
    std::chrono::duration<double, std::micro> elapsed{end - start};
    if (elapsed < default_period) {
      std::this_thread::sleep_for(default_period - elapsed);
    } else {
      AERROR << "Too much time consumption in Hongqi_nboosterController looping process:"
             << elapsed.count();
    }
  }
}

bool Hongqi_nboosterController::CheckResponse(const int32_t flags, bool need_wait) {
  // for hongqi, the handshake is commonly 2000ms
  // try 4s to enter auto.
  int32_t retry_num = 200;
  bool is_eps_online = false;
  bool is_vcu_online = false;
  bool is_esp_online = false;

  do {
    bool check_ok = true;
    if (flags & CHECK_RESPONSE_STEER_UNIT_FLAG) {
      is_eps_online = chassis_.has_check_response() &&
                      chassis_.check_response().has_is_eps_online() &&
                      chassis_.check_response().is_eps_online();
      check_ok = check_ok && is_eps_online;
    }

    if (flags & CHECK_RESPONSE_SPEED_UNIT_FLAG) {
      is_vcu_online = chassis_.has_check_response() &&
                      chassis_.check_response().has_is_vcu_online() &&
                      chassis_.check_response().is_vcu_online();
      is_esp_online = chassis_.has_check_response() &&
                      chassis_.check_response().has_is_esp_online() &&
                      chassis_.check_response().is_esp_online();
      check_ok = check_ok && is_vcu_online && is_esp_online;
    }
    if (check_ok) {
      return true;
    } else {
      AINFO << "Need to check response again.";
    }
    if (need_wait) {
      --retry_num;
      std::this_thread::sleep_for(
          std::chrono::duration<double, std::milli>(20));
    }
  } while (need_wait && retry_num);

  if (flags & CHECK_RESPONSE_STEER_UNIT_FLAG) {
    AERROR << "steer check_response fail: is_eps_online:" << is_eps_online;
  }

  if (flags & CHECK_RESPONSE_SPEED_UNIT_FLAG) {
    AERROR << "speed check_response fail: " << "is_vcu_online:" << is_vcu_online
           << ", is_esp_online:" << is_esp_online;
  }

  return false;
}

void Hongqi_nboosterController::set_chassis_error_mask(const int32_t mask) {
  std::lock_guard<std::mutex> lock(chassis_mask_mutex_);
  chassis_error_mask_ = mask;
}

int32_t Hongqi_nboosterController::chassis_error_mask() {
  std::lock_guard<std::mutex> lock(chassis_mask_mutex_);
  return chassis_error_mask_;
}

Chassis::ErrorCode Hongqi_nboosterController::chassis_error_code() {
  std::lock_guard<std::mutex> lock(chassis_error_code_mutex_);
  return chassis_error_code_;
}

void Hongqi_nboosterController::set_chassis_error_code(
    const Chassis::ErrorCode& error_code) {
  std::lock_guard<std::mutex> lock(chassis_error_code_mutex_);
  chassis_error_code_ = error_code;
}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
