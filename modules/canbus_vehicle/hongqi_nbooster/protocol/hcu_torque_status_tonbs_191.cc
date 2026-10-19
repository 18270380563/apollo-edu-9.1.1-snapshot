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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/hcu_torque_status_tonbs_191.h"
 
 #include "glog/logging.h"
 
 #include "modules/drivers/canbus/common/byte.h"
 #include "modules/drivers/canbus/common/canbus_consts.h"
  
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

Hcutorquestatustonbs191::Hcutorquestatustonbs191() {}

const int32_t Hcutorquestatustonbs191::ID = 0x191;

void Hcutorquestatustonbs191::Parse(const std::uint8_t* bytes, int32_t length,
    Hongqi_nbooster* chassis) const {
    // if (hcu_accecontrol_status(bytes, length) == 2) {
    //     chassis->mutable_handshake()->set_is_vcu_online(true);
    // } else {
    //     chassis->mutable_handshake()->set_is_vcu_online(false);
    // }
    // chassis->mutable_gas()->set_driver_override(hcu_acceoverride_status(bytes, length));
    // chassis->mutable_safety()->set_pedal_control_available(hcu_pedalctl_available(bytes, length));
    chassis->mutable_hcu_torque_status_tonbs_191()->set_hcu_acttorque_ev(hcu_acttorque_ev(bytes, length));
    chassis->mutable_hcu_torque_status_tonbs_191()->set_hcu_acceoverride_status(hcu_acceoverride_status(bytes, length));
    chassis->mutable_hcu_torque_status_tonbs_191()->set_hcu_accecontrol_status(hcu_accecontrol_status(bytes, length));
    chassis->mutable_hcu_torque_status_tonbs_191()->set_hcu_pedalctl_available(hcu_pedalctl_available(bytes, length));
    chassis->mutable_hcu_torque_status_tonbs_191()->set_hcu_mcu1tmmagtorque_tonbs_front(hcu_mcu1tmmagtorque_tonbs_front(bytes, length));
    chassis->mutable_hcu_torque_status_tonbs_191()->set_hcu_mcuacttorque_tonbs_front(hcu_mcuacttorque_tonbs_front(bytes, length));
}

// config detail: {'description': 'Indicates the actual torque', 'offset': -800.0, 'precision': 0.1, 'len': 14, 'name': 'hcu_acttorque_ev', 'is_signed_var': False, 'physical_range': '[-800|800]', 'bit': 8, 'type': 'double', 'order': 'intel', 'physical_unit': 'Nm'}
double Hcutorquestatustonbs191::hcu_acttorque_ev(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(0, 6);
    Byte t1(bytes + 1);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;
    
    double ret = x * 0.100000 + -800.000000;
    return ret;
}

// config detail: {'name': 'hcu_acceoverride_status', 'enum': {0: 'HCU_ACCEOVERRIDE_STATUS_NOT_OVERRIDE', 1: 'HCU_ACCEOVERRIDE_STATUS_OVERRIDE'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 27, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
HCU_Torque_Status_ToNBS_191::HCU_AcceOverride_StatusType Hcutorquestatustonbs191::hcu_acceoverride_status(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(3, 1);

    HCU_Torque_Status_ToNBS_191::HCU_AcceOverride_StatusType ret = static_cast<HCU_Torque_Status_ToNBS_191::HCU_AcceOverride_StatusType>(x);
    return ret;
}

// config detail: {'name': 'hcu_accecontrol_status', 'enum': {0: 'HCU_ACCECONTROL_STATUS_MANUEL_MODE', 1: 'HCU_ACCECONTROL_STATUS_TARGET_TORQUE_MODE', 2: 'HCU_ACCECONTROL_STATUS_VIRTUAL_PEDAL_MODE'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 25, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
HCU_Torque_Status_ToNBS_191::HCU_AcceControl_StatusType Hcutorquestatustonbs191::hcu_accecontrol_status(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(1, 2);

    HCU_Torque_Status_ToNBS_191::HCU_AcceControl_StatusType ret = static_cast<HCU_Torque_Status_ToNBS_191::HCU_AcceControl_StatusType>(x);
    return ret;
}

// config detail: {'name': 'hcu_pedalctl_available', 'enum': {0: 'HCU_PEDALCTL_AVAILABLE_AVAILABLE', 1: 'HCU_PEDALCTL_AVAILABLE_NOT_AVAILABLE'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
HCU_Torque_Status_ToNBS_191::HCU_PedalCTL_AvailableType Hcutorquestatustonbs191::hcu_pedalctl_available(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(0, 1);

    HCU_Torque_Status_ToNBS_191::HCU_PedalCTL_AvailableType ret = static_cast<HCU_Torque_Status_ToNBS_191::HCU_PedalCTL_AvailableType>(x);
    return ret;
}

// config detail: {'name': 'hcu_mcu1tmmagtorque_tonbs_front', 'offset': -400.0, 'precision': 0.1, 'len': 13, 'is_signed_var': False, 'physical_range': '[-400|0]', 'bit': 48, 'type': 'double', 'order': 'intel', 'physical_unit': 'Nm'}
double Hcutorquestatustonbs191::hcu_mcu1tmmagtorque_tonbs_front(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(0, 5);
    Byte t1(bytes + 6);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;

    double ret = x * 0.100000 + -400.000000;
    return ret;
}

// config detail: {'name': 'hcu_mcuacttorque_tonbs_front', 'offset': -400.0, 'precision': 0.1, 'len': 13, 'is_signed_var': False, 'physical_range': '[-400|0]', 'bit': 32, 'type': 'double', 'order': 'intel', 'physical_unit': 'Nm'}
double Hcutorquestatustonbs191::hcu_mcuacttorque_tonbs_front(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 5);
    int32_t x = t0.get_byte(0, 5);
    Byte t1(bytes + 4);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;

    double ret = x * 0.100000 + -400.000000;
    return ret;
}
}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
