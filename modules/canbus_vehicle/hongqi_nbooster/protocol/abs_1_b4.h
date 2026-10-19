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

class Abs1b4 : public ::apollo::drivers::canbus::ProtocolData<
            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;
    Abs1b4();
    void Parse(const std::uint8_t* bytes, int32_t length,
        Hongqi_nbooster* chassis) const override;
private:
    // config detail: {'description': 'Checksum_ABS_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_ABS_1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int checksum_abs_1(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Front right wheel speed(raw data)', 'offset': 0.0, 'precision': 0.01, 'len': 15, 'name': 'WheelSpeed_FR', 'is_signed_var': False, 'physical_range': '[0|327.65]', 'bit': 24, 'type': 'double', 'order': 'intel', 'physical_unit': 'km/h'}
    double wheelspeed_fr(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Front left wheel speed(raw data)', 'offset': 0.0, 'precision': 0.01, 'len': 15, 'name': 'WheelSpeed_FL', 'is_signed_var': False, 'physical_range': '[0|327.65]', 'bit': 8, 'type': 'double', 'order': 'intel', 'physical_unit': 'km/h'}
    double wheelspeed_fl(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 2, 'name': 'Reserved_ABS1_4', 'is_signed_var': False, 'physical_range': '[0|3]', 'bit': 62, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int reserved_abs1_4(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'Reserved_ABS1_3', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 55, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
    bool reserved_abs1_3(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'Reserved_ABS1_2', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 39, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
    bool reserved_abs1_2(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'Reserved_ABS1_1', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 23, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
    bool reserved_abs1_1(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Live counter check for message ABS_1', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'LiveCounter_ABS_1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 56, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    int livecounter_abs_1(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Vehicle speed output from ABS(derived from raw data)', 'offset': 0.0, 'precision': 0.01, 'len': 15, 'name': 'ABS_VehicleSpeed', 'is_signed_var': False, 'physical_range': '[0|327.65]', 'bit': 40, 'type': 'double', 'order': 'intel', 'physical_unit': 'km/h'}
    double abs_vehiclespeed(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Driving direction of the vehicle: forward / backward', 'enum': {0: 'ABS_DRIVINGDIRECTION_FORWARD', 1: 'ABS_DRIVINGDIRECTION_BACKWARD', 2: 'ABS_DRIVINGDIRECTION_STOP', 3: 'ABS_DRIVINGDIRECTION_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'ABS_DrivingDirection', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 60, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    ABS_1_b4::ABS_DrivingDirectionType abs_drivingdirection(const std::uint8_t* bytes, int length) const;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
