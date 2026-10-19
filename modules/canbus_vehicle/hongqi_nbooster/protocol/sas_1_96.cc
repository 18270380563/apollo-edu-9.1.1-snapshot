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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/sas_1_96.h"

 #include "glog/logging.h"
 
 #include "modules/drivers/canbus/common/byte.h"
 #include "modules/drivers/canbus/common/canbus_consts.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

const int32_t Sas196::ID = 0x96;

Sas196::Sas196() {}

void Sas196::Parse(const std::uint8_t* bytes, int32_t length, Hongqi_nbooster* chassis) const {

    // if (sas_steeringwheelanglesign(bytes, length)) {
    //     car_status->mutable_eps()->set_steering_angle(-sas_steeringwheelangle(bytes, length));
    // } else {
    //     car_status->mutable_eps()->set_steering_angle(sas_steeringwheelangle(bytes, length));
    // }
    // car_status->mutable_safety()->set_sas_system_fault(sas_status(bytes, length));

    chassis->mutable_sas_1_96()->set_sas_steeringwheelspeedsign(sas_steeringwheelspeedsign(bytes, length));
    chassis->mutable_sas_1_96()->set_sas_steeringwheelspeed(sas_steeringwheelspeed(bytes, length));
    chassis->mutable_sas_1_96()->set_sas_id(sas_id(bytes, length));
    chassis->mutable_sas_1_96()->set_sas_reserved_2(sas_reserved_2(bytes, length));
    chassis->mutable_sas_1_96()->set_sas_status(sas_status(bytes, length));
    chassis->mutable_sas_1_96()->set_sas_livecounter_sas_1(sas_livecounter_sas_1(bytes, length));
    chassis->mutable_sas_1_96()->set_sas_checksum_sas_1(sas_checksum_sas_1(bytes, length));
    chassis->mutable_sas_1_96()->set_sas_steeringwheelanglesign(sas_steeringwheelanglesign(bytes, length));
    chassis->mutable_sas_1_96()->set_sas_steeringwheelangle(sas_steeringwheelangle(bytes, length));
}

// config detail: {'description': 'Sign of steering wheel angle (+/-)definition according to DIN 70000 / ISO 8855wheel position to left = positivewheel position to right = negativewheel position straightforward = 0 (positive)', 'enum': {0: 'SAS_STEERINGWHEELSPEEDSIGN_LEFT_POSITIVE', 1: 'SAS_STEERINGWHEELSPEEDSIGN_RIGHT_NEGATIVE'}, 'precision': 1.0, 'len': 1, 'name': 'sas_steeringwheelspeedsign', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 39, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
SAS_1_96::SAS_SteeringWheelSpeedSignType Sas196::sas_steeringwheelspeedsign(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(7, 1);
    SAS_1_96::SAS_SteeringWheelSpeedSignType ret = static_cast<SAS_1_96::SAS_SteeringWheelSpeedSignType>(x);
    return  ret;
}

// config detail: {'description': 'Sign of steering wheel speed (+/-)', 'offset': 0.0, 'precision': 0.04375, 'len': 15, 'name': 'sas_steeringwheelspeed', 'is_signed_var': False, 'physical_range': '[0|1433.5125]', 'bit': 24, 'type': 'double', 'order': 'intel', 'physical_unit': '\xa1\xe3/s'}
double Sas196::sas_steeringwheelspeed(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(0, 7);
    Byte t1(bytes + 3);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;
    double ret = x * 0.043750;
    return  ret;
}

// config detail: {'description': 'ID of steering angle sensor (used in combination with class 2 sensor)', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'sas_id', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 40, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Sas196::sas_id(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 5);
    int32_t x = t0.get_byte(0, 8);
    int ret = x;
    return  ret;
}

// config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 6, 'name': 'sas_reserved_2', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 58, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Sas196::sas_reserved_2(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(2, 6);
    int ret = x;
    return  ret;
}

// config detail: {'description': 'Status of steering angle sensor (used in combination with class 3 sensor)', 'enum': {0: 'SAS_STATUS_SAS_ANGLE_AND_SPEED_CORRECT', 1: 'SAS_STATUS_SAS_NOT_CALIBRATED', 2: 'SAS_STATUS_INTERMITTENT_ERROR_DETECTED', 3: 'SAS_STATUS_PERMANENT_ERROR_DETECTED'}, 'precision': 1.0, 'len': 2, 'name': 'sas_status', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 56, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
SAS_1_96::SAS_StatusType Sas196::sas_status(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(0, 2);
    SAS_1_96::SAS_StatusType ret = static_cast<SAS_1_96::SAS_StatusType>(x);
    return  ret;
}

// config detail: {'description': 'Live counter for SAS_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'sas_livecounter_sas_1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 48, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
int Sas196::sas_livecounter_sas_1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(0, 8);
    int ret = x;
    return  ret;
}

// config detail: {'description': 'Sign of steering wheel angle (+/-)definition according to DIN 70000 / ISO 8855wheel position to left = positivewheel position to right = negativewheel position straightforward = 0 (positive)', 'enum': {0: 'SAS_STEERINGWHEELANGLESIGN_LEFT_POSITIVE', 1: 'SAS_STEERINGWHEELANGLESIGN_RIGHT_NEGATIVE'}, 'precision': 1.0, 'len': 1, 'name': 'sas_steeringwheelanglesign', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 23, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
SAS_1_96::SAS_SteeringWheelAngleSignType Sas196::sas_steeringwheelanglesign(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(7, 1);
    SAS_1_96::SAS_SteeringWheelAngleSignType ret = static_cast<SAS_1_96::SAS_SteeringWheelAngleSignType>(x);
    return  ret;
}

// config detail: {'description': 'Angle of steering wheel (raw data)', 'offset': 0.0, 'precision': 0.04375, 'len': 15, 'name': 'sas_steeringwheelangle', 'is_signed_var': False, 'physical_range': '[0|1433.5125]', 'bit': 8, 'type': 'double', 'order': 'intel', 'physical_unit': '\xa1\xe3'}
double Sas196::sas_steeringwheelangle(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(0, 7);
    Byte t1(bytes + 1);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;
    double ret = x * 0.043750;
    return  ret;
}

// config detail: {'description': 'Checksum of message SAS_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'sas_checksum_sas_1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Sas196::sas_checksum_sas_1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 0);
    int32_t x = t0.get_byte(0, 8);
    int ret = x;
    return  ret;
}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
