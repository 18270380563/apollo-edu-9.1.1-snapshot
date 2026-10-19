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

class Sas196 : public ::apollo::drivers::canbus::ProtocolData<
            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;
    Sas196();
    void Parse(const std::uint8_t* bytes, int32_t length,
                Hongqi_nbooster* chassis) const override;

    // config detail: {'description': 'Sign of steering wheel angle (+/-)definition according to DIN 70000 / ISO 8855wheel position to left = positivewheel position to right = negativewheel position straightforward = 0 (positive)', 'enum': {0: 'SAS_STEERINGWHEELSPEEDSIGN_LEFT_POSITIVE', 1: 'SAS_STEERINGWHEELSPEEDSIGN_RIGHT_NEGATIVE'}, 'precision': 1.0, 'len': 1, 'name': 'SAS_SteeringWheelSpeedSign', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 39, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    SAS_1_96::SAS_SteeringWheelSpeedSignType sas_steeringwheelspeedsign(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Sign of steering wheel speed (+/-)', 'offset': 0.0, 'precision': 0.04375, 'len': 15, 'name': 'SAS_SteeringWheelSpeed', 'is_signed_var': False, 'physical_range': '[0|1433.5125]', 'bit': 24, 'type': 'double', 'order': 'intel', 'physical_unit': '\xa1\xe3/s'}
    double sas_steeringwheelspeed(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'ID of steering angle sensor (used in combination with class 2 sensor)', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'SAS_ID', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 40, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int sas_id(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 6, 'name': 'SAS_Reserved_2', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 58, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int sas_reserved_2(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Status of steering angle sensor (used in combination with class 3 sensor)', 'enum': {0: 'SAS_STATUS_SAS_ANGLE_AND_SPEED_CORRECT', 1: 'SAS_STATUS_SAS_NOT_CALIBRATED', 2: 'SAS_STATUS_INTERMITTENT_ERROR_DETECTED', 3: 'SAS_STATUS_PERMANENT_ERROR_DETECTED'}, 'precision': 1.0, 'len': 2, 'name': 'SAS_Status', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 56, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    SAS_1_96::SAS_StatusType sas_status(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Live counter for SAS_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'SAS_LiveCounter_SAS_1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 48, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    int sas_livecounter_sas_1(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Sign of steering wheel angle (+/-)definition according to DIN 70000 / ISO 8855wheel position to left = positivewheel position to right = negativewheel position straightforward = 0 (positive)', 'enum': {0: 'SAS_STEERINGWHEELANGLESIGN_LEFT_POSITIVE', 1: 'SAS_STEERINGWHEELANGLESIGN_RIGHT_NEGATIVE'}, 'precision': 1.0, 'len': 1, 'name': 'SAS_SteeringWheelAngleSign', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 23, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    SAS_1_96::SAS_SteeringWheelAngleSignType sas_steeringwheelanglesign(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Angle of steering wheel (raw data)', 'offset': 0.0, 'precision': 0.04375, 'len': 15, 'name': 'SAS_SteeringWheelAngle', 'is_signed_var': False, 'physical_range': '[0|1433.5125]', 'bit': 8, 'type': 'double', 'order': 'intel', 'physical_unit': '\xa1\xe3'}
    double sas_steeringwheelangle(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Checksum of message SAS_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'SAS_Checksum_SAS_1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int sas_checksum_sas_1(const std::uint8_t* bytes, int length) const;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
