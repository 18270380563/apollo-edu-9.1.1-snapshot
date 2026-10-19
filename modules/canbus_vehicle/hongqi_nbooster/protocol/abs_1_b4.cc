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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/abs_1_b4.h"
 
 #include "glog/logging.h"
 
 #include "modules/drivers/canbus/common/byte.h"
 #include "modules/drivers/canbus/common/canbus_consts.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

const int32_t Abs1b4::ID = 0xB4;

const double KMH2MS = 1.0 / 3.6;                    // km/h convert to m/s

Abs1b4::Abs1b4() {}

void Abs1b4::Parse(const std::uint8_t* bytes, int32_t length,
                    Hongqi_nbooster* chassis) const {
    // car_status->mutable_vehicle_spd()->set_wheel_spd_fr(wheelspeed_fr(bytes, length) * KMH2MS);
    // car_status->mutable_vehicle_spd()->set_wheel_spd_fl(wheelspeed_fl(bytes, length) * KMH2MS);
    // const int direction = abs_drivingdirection(bytes, length);
    // car_status->mutable_vehicle_spd()->set_is_vehicle_standstill(false);
    // const double speed = abs_vehiclespeed(bytes, length) * KMH2MS;
    // switch (direction) {
    //     case 0:         // forward
    //         car_status->mutable_vehicle_spd()->set_vehicle_spd(speed);
    //         car_status->mutable_safety()->set_is_driving_direction_invalid(false);
    //         break;
    //     case 1:         // backward
    //         car_status->mutable_vehicle_spd()->set_vehicle_spd(-speed);
    //         car_status->mutable_safety()->set_is_driving_direction_invalid(false);
    //         break;
    //     case 2:         // stop
    //         car_status->mutable_vehicle_spd()->set_vehicle_spd(0.0);
    //         car_status->mutable_safety()->set_is_driving_direction_invalid(false);
    //         car_status->mutable_vehicle_spd()->set_is_vehicle_standstill(true);
    //         break;
    //     case 3:
    //         car_status->mutable_safety()->set_is_driving_direction_invalid(true);
    //         break;
    //     default:        // default as forward
    //         car_status->mutable_vehicle_spd()->set_vehicle_spd(speed);
    //         LOG_ERROR << "error happened in abs_1_b4 can message: speed direction error";
    //         break;
    // }
    // car_status->mutable_vehicle_spd()->set_timestamp_sec(_timestamp);

    chassis->mutable_abs_1_b4()->set_abs_drivingdirection(abs_drivingdirection(bytes, length));
    chassis->mutable_abs_1_b4()->set_livecounter_abs_1(livecounter_abs_1(bytes, length));
    chassis->mutable_abs_1_b4()->set_wheelspeed_fl(wheelspeed_fl(bytes, length));
    chassis->mutable_abs_1_b4()->set_wheelspeed_fr(wheelspeed_fr(bytes, length));
    chassis->mutable_abs_1_b4()->set_abs_vehiclespeed(abs_vehiclespeed(bytes, length));
    chassis->mutable_abs_1_b4()->set_checksum_abs_1(checksum_abs_1(bytes, length));
    chassis->mutable_abs_1_b4()->set_reserved_abs1_1(reserved_abs1_1(bytes, length));
    chassis->mutable_abs_1_b4()->set_reserved_abs1_2(reserved_abs1_2(bytes, length));
    chassis->mutable_abs_1_b4()->set_reserved_abs1_3(reserved_abs1_3(bytes, length));
    chassis->mutable_abs_1_b4()->set_reserved_abs1_4(reserved_abs1_4(bytes, length));
}

// config detail: {'description': 'Checksum_ABS_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'checksum_abs_1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Abs1b4::checksum_abs_1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 0);
    int32_t x = t0.get_byte(0, 8);
    int ret = x;
    return  ret;
}

// config detail: {'description': 'Front right wheel speed(raw data)', 'offset': 0.0, 'precision': 0.01, 'len': 15, 'name': 'wheelspeed_fr', 'is_signed_var': False, 'physical_range': '[0|327.65]', 'bit': 24, 'type': 'double', 'order': 'intel', 'physical_unit': 'km/h'}
double Abs1b4::wheelspeed_fr(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(0, 7);
    Byte t1(bytes + 3);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;
    double ret = x * 0.010000;
    return  ret;
}

// config detail: {'description': 'Front left wheel speed(raw data)', 'offset': 0.0, 'precision': 0.01, 'len': 15, 'name': 'wheelspeed_fl', 'is_signed_var': False, 'physical_range': '[0|327.65]', 'bit': 8, 'type': 'double', 'order': 'intel', 'physical_unit': 'km/h'}
double Abs1b4::wheelspeed_fl(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(0, 7);
    Byte t1(bytes + 1);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;
    double ret = x * 0.010000;
    return  ret;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 2, 'name': 'reserved_abs1_4', 'is_signed_var': False, 'physical_range': '[0|3]', 'bit': 62, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Abs1b4::reserved_abs1_4(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(6, 2);
    int ret = x;
    return  ret;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'reserved_abs1_3', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 55, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
bool Abs1b4::reserved_abs1_3(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(7, 1);
    bool ret = x;
    return  ret;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'reserved_abs1_2', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 39, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
bool Abs1b4::reserved_abs1_2(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(7, 1);
    bool ret = x;
    return  ret;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'reserved_abs1_1', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 23, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
bool Abs1b4::reserved_abs1_1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(7, 1);
    bool ret = x;
    return  ret;
}

// config detail: {'description': 'Live counter check for message ABS_1', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'livecounter_abs_1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 56, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
int Abs1b4::livecounter_abs_1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(0, 4);
    int ret = x;
    return  ret;
}

// config detail: {'description': 'Vehicle speed output from ABS(derived from raw data)', 'offset': 0.0, 'precision': 0.01, 'len': 15, 'name': 'abs_vehiclespeed', 'is_signed_var': False, 'physical_range': '[0|327.65]', 'bit': 40, 'type': 'double', 'order': 'intel', 'physical_unit': 'km/h'}
double Abs1b4::abs_vehiclespeed(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(0, 7);
    Byte t1(bytes + 5);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;
    double ret = x * 0.010000;
    return  ret;
}

// config detail: {'description': 'Driving direction of the vehicle: forward / backward', 'enum': {0: 'ABS_DRIVINGDIRECTION_FORWARD', 1: 'ABS_DRIVINGDIRECTION_BACKWARD', 2: 'ABS_DRIVINGDIRECTION_STOP', 3: 'ABS_DRIVINGDIRECTION_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'abs_drivingdirection', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 60, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
ABS_1_b4::ABS_DrivingDirectionType Abs1b4::abs_drivingdirection(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(4, 2);
    ABS_1_b4::ABS_DrivingDirectionType ret = static_cast<ABS_1_b4::ABS_DrivingDirectionType>(x);
    return  ret;
}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
