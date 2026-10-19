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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/epb_1_104.h"

 #include "glog/logging.h"
 
 #include "modules/drivers/canbus/common/byte.h"
 #include "modules/drivers/canbus/common/canbus_consts.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

const int32_t Epb1104::ID = 0x104;

Epb1104::Epb1104() {}

void Epb1104::Parse(const std::uint8_t* bytes, int32_t length, Hongqi_nbooster* chassis) const {

    // car_status->mutable_epb()->set_is_epb_error(epb_errorstatus(bytes, length));
    // int epb = epb_status(bytes, length);
    // switch (epb) {
    //     case 0x00:
    //         car_status->mutable_epb()->set_parking_brake_status(PbEpb::PBRAKE_OFF);
    //         break;
    //     case 0x01:
    //         car_status->mutable_epb()->set_parking_brake_status(PbEpb::PBRAKE_ON);
    //         break;
    //     case 0x02:
    //         // // add releasing
    //         // car_status->mutable_epb()->set_parking_brake_status(PbEpb::PBRAKE_OFF);
    //         // break;
    //     case 0x03:
    //         car_status->mutable_epb()->set_parking_brake_status(PbEpb::PBRAKE_TRANSITION);
    //         break;
    //     default:
    //         car_status->mutable_epb()->set_parking_brake_status(PbEpb::PBRAKE_FAULT);
    //         LOG_ERROR << "error happened in epb_1_104 can message: epb status error";
    //         break;
    // }
    // car_status->mutable_safety()->set_epb_switch(epb_switch(bytes, length));

    chassis->mutable_epb_1_104()->set_reserved_epb_1_3(reserved_epb_1_3(bytes, length));
    chassis->mutable_epb_1_104()->set_epb_switch(epb_switch(bytes, length));
    chassis->mutable_epb_1_104()->set_reserved_epb_1_2(reserved_epb_1_2(bytes, length));
    chassis->mutable_epb_1_104()->set_reserved_epb_1_1(reserved_epb_1_1(bytes, length));
    chassis->mutable_epb_1_104()->set_livecounter_epb1(livecounter_epb1(bytes, length));
    chassis->mutable_epb_1_104()->set_checksum_epb_1(checksum_epb_1(bytes, length));
    chassis->mutable_epb_1_104()->set_epb_status(epb_status(bytes, length));
    chassis->mutable_epb_1_104()->set_epb_errorstatus(epb_errorstatus(bytes, length));
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 32, 'name': 'reserved_epb_1_3', 'is_signed_var': False, 'physical_range': '[0|4294967295]', 'bit': 32, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Epb1104::reserved_epb_1_3(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(0, 8);
    Byte t1(bytes + 6);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;
    Byte t2(bytes + 5);
    t = t2.get_byte(0, 8);
    x <<= 8;
    x |= t;
    Byte t3(bytes + 4);
    t = t3.get_byte(0, 8);
    x <<= 8;
    x |= t;
    int ret = x;
    return  ret;
}

// config detail: {'description': 'reserved for future extensions', 'enum': {0: 'EPB_SWITCH_NO_REQUEST', 1: 'EPB_SWITCH_LOCK', 2: 'EPB_SWITCH_RELEASE', 3: 'EPB_SWITCH_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'epb_switch', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 30, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
EPB_1_104::EPB_SwitchType Epb1104::epb_switch(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(6, 2);
    EPB_1_104::EPB_SwitchType ret = static_cast<EPB_1_104::EPB_SwitchType>(x);
    return  ret;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 9, 'name': 'reserved_epb_1_2', 'is_signed_var': False, 'physical_range': '[0|511]', 'bit': 21, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Epb1104::reserved_epb_1_2(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(0, 6);
    Byte t1(bytes + 2);
    int32_t t = t1.get_byte(5, 3);
    x <<= 3;
    x |= t;
    int ret = x;
    return  ret;
}

// config detail: {'description': 'Indicate the EGSM is error or not.', 'enum': {0: 'EPB_ERRORSTATUS_NO_ERROR', 1: 'EPB_ERRORSTATUS_ERROR'}, 'precision': 1.0, 'len': 1, 'name': 'epb_errorstatus', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
EPB_1_104::EPB_ErrorStatusType Epb1104::epb_errorstatus(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(4, 1);
    EPB_1_104::EPB_ErrorStatusType ret = static_cast<EPB_1_104::EPB_ErrorStatusType>(x);
    return  ret;
}

// config detail: {'description': 'reserved for future extensions', 'enum': {0: 'EPB_STATUS_RELEASED', 1: 'EPB_STATUS_LOCKED', 2: 'EPB_STATUS_RELEASING', 3: 'EPB_STATUS_LOCKING', 4: 'EPB_STATUS_UNKOWN'}, 'precision': 1.0, 'len': 4, 'name': 'epb_status', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|15]', 'bit': 16, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
EPB_1_104::EPB_StatusType Epb1104::epb_status(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(0, 4);
    EPB_1_104::EPB_StatusType ret = static_cast<EPB_1_104::EPB_StatusType>(x);
    return  ret;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'reserved_epb_1_1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 12, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Epb1104::reserved_epb_1_1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(4, 4);
    int ret = x;
    return  ret;
}

// config detail: {'description': 'Live counter for EPB_1', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'livecounter_epb1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 8, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
int Epb1104::livecounter_epb1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(0, 4);
    int ret = x;
    return  ret;
}

// config detail: {'description': 'Checksum of message EPB_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'checksum_epb_1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Epb1104::checksum_epb_1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 0);
    int32_t x = t0.get_byte(0, 8);
    int ret = x;
    return  ret;
}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
