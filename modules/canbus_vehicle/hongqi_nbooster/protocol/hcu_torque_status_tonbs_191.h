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

 #pragma once

 #include "modules/canbus_vehicle/hongqi_nbooster/proto/hongqi_nbooster.pb.h"
 
 #include "modules/drivers/canbus/can_comm/protocol_data.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

class Hcutorquestatustonbs191 : public ::apollo::drivers::canbus::ProtocolData<
                            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;
    Hcutorquestatustonbs191();
    void Parse(const std::uint8_t* bytes, int32_t length,
                Hongqi_nbooster* chassis) const override;

    // config detail: {'description': 'Indicates the actual torque', 'offset': -800.0, 'precision': 0.1, 'len': 14, 'name': 'HCU_ActTorque_EV', 'is_signed_var': False, 'physical_range': '[-800|800]', 'bit': 8, 'type': 'double', 'order': 'intel', 'physical_unit': 'Nm'}
    double hcu_acttorque_ev(const std::uint8_t* bytes, int length) const;

    // config detail: {'name': 'HCU_AcceOverride_Status', 'enum': {0: 'HCU_ACCEOVERRIDE_STATUS_NOT_OVERRIDE', 1: 'HCU_ACCEOVERRIDE_STATUS_OVERRIDE'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 27, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    HCU_Torque_Status_ToNBS_191::HCU_AcceOverride_StatusType hcu_acceoverride_status(const std::uint8_t* bytes, int length) const;

    // config detail: {'name': 'HCU_AcceControl_Status', 'enum': {0: 'HCU_ACCECONTROL_STATUS_MANUEL_MODE', 1: 'HCU_ACCECONTROL_STATUS_TARGET_TORQUE_MODE', 2: 'HCU_ACCECONTROL_STATUS_VIRTUAL_PEDAL_MODE'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 25, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    HCU_Torque_Status_ToNBS_191::HCU_AcceControl_StatusType hcu_accecontrol_status(const std::uint8_t* bytes, int length) const;

    // config detail: {'name': 'HCU_PedalCTL_Available', 'enum': {0: 'HCU_PEDALCTL_AVAILABLE_AVAILABLE', 1: 'HCU_PEDALCTL_AVAILABLE_NOT_AVAILABLE'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    HCU_Torque_Status_ToNBS_191::HCU_PedalCTL_AvailableType hcu_pedalctl_available(const std::uint8_t* bytes, int length) const;

    // config detail: {'name': 'HCU_MCU1TMMAGTorque_ToNBS_Front', 'offset': -400.0, 'precision': 0.1, 'len': 13, 'is_signed_var': False, 'physical_range': '[-400|0]', 'bit': 48, 'type': 'double', 'order': 'intel', 'physical_unit': 'Nm'}
    double hcu_mcu1tmmagtorque_tonbs_front(const std::uint8_t* bytes, int length) const;

    // config detail: {'name': 'HCU_MCUACTTorque_ToNBS_Front', 'offset': -400.0, 'precision': 0.1, 'len': 13, 'is_signed_var': False, 'physical_range': '[-400|0]', 'bit': 32, 'type': 'double', 'order': 'intel', 'physical_unit': 'Nm'}
    double hcu_mcuacttorque_tonbs_front(const std::uint8_t* bytes, int length) const;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
