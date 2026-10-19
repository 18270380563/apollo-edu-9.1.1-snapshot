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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/hcu_14_248.h"
 
 #include "glog/logging.h"
 
 #include "modules/drivers/canbus/common/byte.h"
 #include "modules/drivers/canbus/common/canbus_consts.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

const int32_t Hcu14248::ID = 0x248;

Hcu14248::Hcu14248() {}

void Hcu14248::Parse(const std::uint8_t* bytes, int32_t length, Hongqi_nbooster* chassis) const {
//     int gear = hcu_leverinfo(bytes, length);
//     switch (gear) {
//         case 0x00:
//             car_status->mutable_gear()->set_gear_state(::adu::common::status::GEAR_PARKING);
//             break;
//         case 0x01:
//             car_status->mutable_gear()->set_gear_state(::adu::common::status::GEAR_REVERSE);
//             break;
//         case 0x02:
//             car_status->mutable_gear()->set_gear_state(::adu::common::status::GEAR_NEUTRAL);
//             break;
//         case 0x03:
//             car_status->mutable_gear()->set_gear_state(::adu::common::status::GEAR_DRIVE);
//             break;
//         case 0xF:
//             car_status->mutable_gear()->set_gear_state(::adu::common::status::GEAR_INVALID);
//             break;
//         default:
//             break;
//     }
    chassis->mutable_hcu_14_248()->set_hcu_leverinfo(hcu_leverinfo(bytes, length));
    chassis->mutable_hcu_14_248()->set_hcu_reserved_2(hcu_reserved_2(bytes, length));
}

// config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 20, 'name': 'hcu_reserved_2', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 44, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Hcu14248::hcu_reserved_2(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(0, 8);
    Byte t1(bytes + 6);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;
    Byte t2(bytes + 5);
    t = t2.get_byte(4, 4);
    x <<= 4;
    x |= t;
    int ret = x;
    return  ret;
}

// config detail: {'description': 'Lever info for display', 'enum': {0: 'HCU_LEVERINFO_P', 1: 'HCU_LEVERINFO_R', 2: 'HCU_LEVERINFO_N', 3: 'HCU_LEVERINFO_D', 4: 'HCU_LEVERINFO_RESERVED', 5: 'HCU_LEVERINFO_RESERVED', 6: 'HCU_LEVERINFO_RESERVED', 7: 'HCU_LEVERINFO_NO_CONNECTION', 8: 'HCU_LEVERINFO_RESERVED', 9: 'HCU_LEVERINFO_RESERVED', 10: 'HCU_LEVERINFO_RESERVED', 11: 'HCU_LEVERINFO_RESERVED', 12: 'HCU_LEVERINFO_RESERVED', 13: 'HCU_LEVERINFO_RESERVED', 14: 'HCU_LEVERINFO_RESERVED', 15: 'HCU_LEVERINFO_INVALID_VALUE'}, 'precision': 1.0, 'len': 4, 'name': 'hcu_leverinfo', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|15]', 'bit': 40, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
HCU_14_248::HCU_LeverInfoType Hcu14248::hcu_leverinfo(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 5);
    int32_t x = t0.get_byte(0, 4);

    HCU_14_248::HCU_LeverInfoType ret = static_cast<HCU_14_248::HCU_LeverInfoType>(x);
    return  ret;
}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
