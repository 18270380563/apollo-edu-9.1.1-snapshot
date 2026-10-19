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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/hcu_4_122.h"

 #include "glog/logging.h"
 
 #include "modules/drivers/canbus/common/byte.h"
 #include "modules/drivers/canbus/common/canbus_consts.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

const int32_t Hcu4122::ID = 0x122;

Hcu4122::Hcu4122() {}

void Hcu4122::Parse(const std::uint8_t* bytes, int32_t length, Hongqi_nbooster* chassis) const {
    // if (hcu_brakeswitch(bytes, length) == 1) {
    //     car_status->mutable_brake()->set_boo_output(true);
    // } else {
    //     car_status->mutable_brake()->set_boo_output(false);
    // }
    // car_status->mutable_gas()->set_throttle_output(hcu_accelerationpedalposition(bytes, length));
    chassis->mutable_hcu_4_122()->set_hcu_brakeswitch(hcu_brakeswitch(bytes, length));
    chassis->mutable_hcu_4_122()->set_hcu_accelerationpedalposition(hcu_accelerationpedalposition(bytes, length));
    chassis->mutable_hcu_4_122()->set_hcu_reserved_6(hcu_reserved_6(bytes, length));
    chassis->mutable_hcu_4_122()->set_hcu_reserved_8(hcu_reserved_8(bytes, length));
    chassis->mutable_hcu_4_122()->set_hcu_reserved_7(hcu_reserved_7(bytes, length));
}

// config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 9, 'name': 'hcu_reserved_8', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 55, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Hcu4122::hcu_reserved_8(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(0, 8);
    Byte t1(bytes + 6);
    int32_t t = t1.get_byte(7, 1);
    x <<= 1;
    x |= t;
    int ret = x;
    return  ret;
}

// config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 21, 'name': 'hcu_reserved_7', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 32, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Hcu4122::hcu_reserved_7(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(0, 5);
    Byte t1(bytes + 5);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;
    Byte t2(bytes + 4);
    t = t2.get_byte(0, 8);
    x <<= 8;
    x |= t;
    int ret = x;
    return  ret;
}

// config detail: {'description': 'signal from brake pedal sended by HCU; harmonized brake switch status', 'enum': {0: 'HCU_BRAKESWITCH_BRAKE_NOT_ACTUATED', 1: 'HCU_BRAKESWITCH_BRAKE_ACTUATED', 2: 'HCU_BRAKESWITCH_RESERVED', 3: 'HCU_BRAKESWITCH_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'hcu_brakeswitch', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 53, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
HCU_4_122::HCU_BrakeSwitchType Hcu4122::hcu_brakeswitch(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(5, 2);
    HCU_4_122::HCU_BrakeSwitchType ret = static_cast<HCU_4_122::HCU_BrakeSwitchType>(x);
    return  ret;
}

// config detail: {'description': 'Acceleration pedal position', 'offset': 0.0, 'precision': 0.4, 'len': 8, 'name': 'hcu_accelerationpedalposition', 'is_signed_var': False, 'physical_range': '[0|100]', 'bit': 24, 'type': 'double', 'order': 'intel', 'physical_unit': '%'}
double Hcu4122::hcu_accelerationpedalposition(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(0, 8);
    double ret = x * 0.400000;
    return  ret;
}

// config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 24, 'name': 'hcu_reserved_6', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Hcu4122::hcu_reserved_6(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(0, 8);
    Byte t1(bytes + 1);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;
    Byte t2(bytes + 0);
    t = t2.get_byte(0, 8);
    x <<= 8;
    x |= t;
    int ret = x;
    return  ret;
}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
