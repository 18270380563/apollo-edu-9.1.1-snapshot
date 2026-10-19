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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/svb_4_2c2.h"

 #include "modules/drivers/canbus/common/byte.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;


const int32_t Svb42c2::ID = 0x2C2;

// public
Svb42c2::Svb42c2() {
    Reset();
}

uint32_t Svb42c2::GetPeriod() const {
    // TODO(All) : modify every protocol's period manually
    static const uint32_t PERIOD = 100 * 1000;
    return PERIOD;
  }

void Svb42c2::Parse(const std::uint8_t* bytes, int32_t length,
                        Hongqi_nbooster* chassis) const {
    chassis->mutable_svb_4_2c2()->set_svb4_obstacle_info_9(svb4_obstacle_info_9(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_obstacle_info_8(svb4_obstacle_info_8(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_obstacle_info_7(svb4_obstacle_info_7(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_obstacle_info_6(svb4_obstacle_info_6(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_obstacle_info_5(svb4_obstacle_info_5(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_obstacle_info_4(svb4_obstacle_info_4(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_obstacle_info_3(svb4_obstacle_info_3(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_obstacle_info_2_copy_1(svb4_obstacle_info_2_copy_1(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_obstacle_info_10(svb4_obstacle_info_10(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_obstacle_info_2(svb4_obstacle_info_2(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_reserved01(svb4_reserved01(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_obstacle_info_1(svb4_obstacle_info_1(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_takeoverrequestinfolamp(svb4_takeoverrequestinfolamp(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_systeminfolamp(svb4_systeminfolamp(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_livecounter(svb4_livecounter(bytes, length));
    chassis->mutable_svb_4_2c2()->set_svb4_checksum(svb4_checksum(bytes, length));
}

void Svb42c2::UpdateData_Heartbeat(uint8_t* data) {
    // TODO(All) : you should add the heartbeat manually
 }

 void Svb42c2::UpdateData(uint8_t* data) {
    set_p_svb4_obstacle_info_9(data, _svb4_obstacle_info_9);
    set_p_svb4_obstacle_info_8(data, _svb4_obstacle_info_8);
    set_p_svb4_obstacle_info_7(data, _svb4_obstacle_info_7);
    set_p_svb4_obstacle_info_6(data, _svb4_obstacle_info_6);
    set_p_svb4_obstacle_info_5(data, _svb4_obstacle_info_5);
    set_p_svb4_obstacle_info_4(data, _svb4_obstacle_info_4);
    set_p_svb4_obstacle_info_3(data, _svb4_obstacle_info_3);
    set_p_svb4_obstacle_info_2_copy_1(data, _svb4_obstacle_info_2_copy_1);
    set_p_svb4_obstacle_info_10(data, _svb4_obstacle_info_10);
    set_p_svb4_obstacle_info_2(data, _svb4_obstacle_info_2);
    set_p_svb4_reserved01(data, _svb4_reserved01);
    set_p_svb4_obstacle_info_1(data, _svb4_obstacle_info_1);
    set_p_svb4_takeoverrequestinfolamp(data, _svb4_takeoverrequestinfolamp);
    set_p_svb4_systeminfolamp(data, _svb4_systeminfolamp);
    // UpdateMore(data);
}

// use update_more() to add live_counter and checksum, remember to delete them in update_data()
void Svb42c2::UpdateMore(uint8_t* data) {
    _svb4_livecounter += 1;
    _svb4_livecounter %= 16;
    set_p_svb4_livecounter(data, _svb4_livecounter);
    _svb4_checksum = data[1] ^
                     data[2] ^
                     data[3] ^
                     data[4] ^
                     data[5] ^
                     data[6] ^
                     data[7];
    set_p_svb4_checksum(data, _svb4_checksum);
}

void Svb42c2::Reset() {
    _svb4_obstacle_info_9 = 0;
    _svb4_obstacle_info_8 = 0;
    _svb4_obstacle_info_7 = 0;
    _svb4_obstacle_info_6 = 0;
    _svb4_obstacle_info_5 = 0;
    _svb4_obstacle_info_4 = 0;
    _svb4_obstacle_info_3 = 0;
    _svb4_obstacle_info_2_copy_1 = 0;
    _svb4_obstacle_info_10 = 0;
    _svb4_obstacle_info_2 = 0;
    _svb4_reserved01 = 0;
    _svb4_obstacle_info_1 = 0;
    _svb4_takeoverrequestinfolamp = 0;
    _svb4_systeminfolamp = 0;
}

Svb42c2* Svb42c2::set_svb4_obstacle_info_9(int svb4_obstacle_info_9) {
    _svb4_obstacle_info_9 = svb4_obstacle_info_9;
    return this;
}

// config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_9_NO', 1: 'SVB4_OBSTACLE_INFO_9_YES', 2: 'SVB4_OBSTACLE_INFO_9_RESERVED', 3: 'SVB4_OBSTACLE_INFO_9_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_9', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 30, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_obstacle_info_9(uint8_t* data, int svb4_obstacle_info_9) {
    _svb4_obstacle_info_9_x = svb4_obstacle_info_9;
    Byte to_set(data + 3);
    to_set.set_value(_svb4_obstacle_info_9_x, 6, 2);
}

Svb42c2* Svb42c2::set_svb4_obstacle_info_8(int svb4_obstacle_info_8) {
    _svb4_obstacle_info_8 = svb4_obstacle_info_8;
    return this;
}

// config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_8_NO', 1: 'SVB4_OBSTACLE_INFO_8_YES', 2: 'SVB4_OBSTACLE_INFO_8_RESERVED', 3: 'SVB4_OBSTACLE_INFO_8_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_8', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 28, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_obstacle_info_8(uint8_t* data, int svb4_obstacle_info_8) {
    _svb4_obstacle_info_8_x = svb4_obstacle_info_8;
    Byte to_set(data + 3);
    to_set.set_value(_svb4_obstacle_info_8_x, 4, 2);
}

Svb42c2* Svb42c2::set_svb4_obstacle_info_7(int svb4_obstacle_info_7) {
    _svb4_obstacle_info_7 = svb4_obstacle_info_7;
    return this;
}

// config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_7_NO', 1: 'SVB4_OBSTACLE_INFO_7_YES', 2: 'SVB4_OBSTACLE_INFO_7_RESERVED', 3: 'SVB4_OBSTACLE_INFO_7_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_7', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 26, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_obstacle_info_7(uint8_t* data, int svb4_obstacle_info_7) {
    _svb4_obstacle_info_7_x = svb4_obstacle_info_7;
    Byte to_set(data + 3);
    to_set.set_value(_svb4_obstacle_info_7_x, 2, 2);
}

Svb42c2* Svb42c2::set_svb4_obstacle_info_6(int svb4_obstacle_info_6) {
    _svb4_obstacle_info_6 = svb4_obstacle_info_6;
    return this;
}

// config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_6_NO', 1: 'SVB4_OBSTACLE_INFO_6_YES', 2: 'SVB4_OBSTACLE_INFO_6_RESERVED', 3: 'SVB4_OBSTACLE_INFO_6_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_6', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_obstacle_info_6(uint8_t* data, int svb4_obstacle_info_6) {
    _svb4_obstacle_info_6_x = svb4_obstacle_info_6;
    Byte to_set(data + 3);
    to_set.set_value(_svb4_obstacle_info_6_x, 0, 2);
}

Svb42c2* Svb42c2::set_svb4_obstacle_info_5(int svb4_obstacle_info_5) {
    _svb4_obstacle_info_5 = svb4_obstacle_info_5;
    return this;
}

// config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_5_NO', 1: 'SVB4_OBSTACLE_INFO_5_YES', 2: 'SVB4_OBSTACLE_INFO_5_RESERVED', 3: 'SVB4_OBSTACLE_INFO_5_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_5', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 22, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_obstacle_info_5(uint8_t* data, int svb4_obstacle_info_5) {
    _svb4_obstacle_info_5_x = svb4_obstacle_info_5;
    Byte to_set(data + 2);
    to_set.set_value(_svb4_obstacle_info_5_x, 6, 2);
}

Svb42c2* Svb42c2::set_svb4_obstacle_info_4(int svb4_obstacle_info_4) {
    _svb4_obstacle_info_4 = svb4_obstacle_info_4;
    return this;
}

// config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_4_NO', 1: 'SVB4_OBSTACLE_INFO_4_YES', 2: 'SVB4_OBSTACLE_INFO_4_RESERVED', 3: 'SVB4_OBSTACLE_INFO_4_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_4', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_obstacle_info_4(uint8_t* data, int svb4_obstacle_info_4) {
    _svb4_obstacle_info_4_x = svb4_obstacle_info_4;
    Byte to_set(data + 2);
    to_set.set_value(_svb4_obstacle_info_4_x, 4, 2);
}

Svb42c2* Svb42c2::set_svb4_obstacle_info_3(int svb4_obstacle_info_3) {
    _svb4_obstacle_info_3 = svb4_obstacle_info_3;
    return this;
}

// config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_3_NO', 1: 'SVB4_OBSTACLE_INFO_3_YES', 2: 'SVB4_OBSTACLE_INFO_3_RESERVED', 3: 'SVB4_OBSTACLE_INFO_3_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_3', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 18, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_obstacle_info_3(uint8_t* data, int svb4_obstacle_info_3) {
    _svb4_obstacle_info_3_x = svb4_obstacle_info_3;
    Byte to_set(data + 2);
    to_set.set_value(_svb4_obstacle_info_3_x, 2, 2);
}

Svb42c2* Svb42c2::set_svb4_obstacle_info_2_copy_1(int svb4_obstacle_info_2_copy_1) {
    _svb4_obstacle_info_2_copy_1 = svb4_obstacle_info_2_copy_1;
    return this;
}

// config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_2_COPY_1_NO', 1: 'SVB4_OBSTACLE_INFO_2_COPY_1_YES', 2: 'SVB4_OBSTACLE_INFO_2_COPY_1_RESERVED', 3: 'SVB4_OBSTACLE_INFO_2_COPY_1_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_2_Copy_1', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 16, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_obstacle_info_2_copy_1(uint8_t* data, int svb4_obstacle_info_2_copy_1) {
    _svb4_obstacle_info_2_copy_1_x = svb4_obstacle_info_2_copy_1;
    Byte to_set(data + 2);
    to_set.set_value(_svb4_obstacle_info_2_copy_1_x, 0, 2);
}

Svb42c2* Svb42c2::set_svb4_obstacle_info_10(int svb4_obstacle_info_10) {
    _svb4_obstacle_info_10 = svb4_obstacle_info_10;
    return this;
}

// config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_10_NO', 1: 'SVB4_OBSTACLE_INFO_10_YES', 2: 'SVB4_OBSTACLE_INFO_10_RESERVED', 3: 'SVB4_OBSTACLE_INFO_10_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_10', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 32, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_obstacle_info_10(uint8_t* data, int svb4_obstacle_info_10) {
    _svb4_obstacle_info_10_x = svb4_obstacle_info_10;
    Byte to_set(data + 4);
    to_set.set_value(_svb4_obstacle_info_10_x, 0, 2);
}

Svb42c2* Svb42c2::set_svb4_obstacle_info_2(int svb4_obstacle_info_2) {
    _svb4_obstacle_info_2 = svb4_obstacle_info_2;
    return this;
}

// config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_2_NO', 1: 'SVB4_OBSTACLE_INFO_2_YES', 2: 'SVB4_OBSTACLE_INFO_2_RESERVED', 3: 'SVB4_OBSTACLE_INFO_2_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_2', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 14, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_obstacle_info_2(uint8_t* data, int svb4_obstacle_info_2) {
    _svb4_obstacle_info_2_x = svb4_obstacle_info_2;
    Byte to_set(data + 1);
    to_set.set_value(_svb4_obstacle_info_2_x, 6, 2);
}

Svb42c2* Svb42c2::set_svb4_livecounter(int svb4_livecounter) {
    _svb4_livecounter = svb4_livecounter;
    return this;
}

// config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'SVB4_Livecounter', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
void Svb42c2::set_p_svb4_livecounter(uint8_t* data, int svb4_livecounter) {
    svb4_livecounter = ProtocolData::BoundedValue(0, 15, svb4_livecounter);
    _svb4_livecounter_x = svb4_livecounter;
    Byte to_set(data + 7);
    to_set.set_value(_svb4_livecounter_x, 4, 4);
}

Svb42c2* Svb42c2::set_svb4_reserved01(int svb4_reserved01) {
    _svb4_reserved01 = svb4_reserved01;
    return this;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 26, 'name': 'SVB4_Reserved01', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 34, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_reserved01(uint8_t* data, int svb4_reserved01) {
    svb4_reserved01 = ProtocolData::BoundedValue(0, 0, svb4_reserved01);
    _svb4_reserved01_x = svb4_reserved01;
    int x = _svb4_reserved01_x;
    uint8_t t = 0;
    t = x & 0x3F;
    Byte to_set0(data + 4);
    to_set0.set_value(t, 2, 6);
    x >>= 6;
    t = x & 0xFF;
    Byte to_set1(data + 5);
    to_set1.set_value(t, 0, 8);
    x >>= 8;
    t = x & 0xFF;
    Byte to_set2(data + 6);
    to_set2.set_value(t, 0, 8);
    x >>= 8;
    t = x & 0xF;
    Byte to_set3(data + 7);
    to_set3.set_value(t, 0, 4);
}

Svb42c2* Svb42c2::set_svb4_obstacle_info_1(int svb4_obstacle_info_1) {
    _svb4_obstacle_info_1 = svb4_obstacle_info_1;
    return this;
}

// config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_1_NO', 1: 'SVB4_OBSTACLE_INFO_1_YES', 2: 'SVB4_OBSTACLE_INFO_1_RESERVED', 3: 'SVB4_OBSTACLE_INFO_1_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_1', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 12, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_obstacle_info_1(uint8_t* data, int svb4_obstacle_info_1) {
    _svb4_obstacle_info_1_x = svb4_obstacle_info_1;
    Byte to_set(data + 1);
    to_set.set_value(_svb4_obstacle_info_1_x, 4, 2);
}

Svb42c2* Svb42c2::set_svb4_takeoverrequestinfolamp(int svb4_takeoverrequestinfolamp) {
    _svb4_takeoverrequestinfolamp = svb4_takeoverrequestinfolamp;
    return this;
}

// config detail: {'description': 'Indicate the takeover request inforLamp off or on', 'enum': {0: 'SVB4_TAKEOVERREQUESTINFOLAMP_LAMP_OFF', 1: 'SVB4_TAKEOVERREQUESTINFOLAMP_LAMP_ON', 2: 'SVB4_TAKEOVERREQUESTINFOLAMP_RESERVED', 3: 'SVB4_TAKEOVERREQUESTINFOLAMP_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_TakeoverRequestInfoLamp', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 10, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_takeoverrequestinfolamp(uint8_t* data, int svb4_takeoverrequestinfolamp) {
    _svb4_takeoverrequestinfolamp_x = svb4_takeoverrequestinfolamp;
    Byte to_set(data + 1);
    to_set.set_value(_svb4_takeoverrequestinfolamp_x, 2, 2);
}

Svb42c2* Svb42c2::set_svb4_systeminfolamp(int svb4_systeminfolamp) {
    _svb4_systeminfolamp = svb4_systeminfolamp;
    return this;
}

// config detail: {'description': 'Indicate the system inforLamp status', 'enum': {0: 'SVB4_SYSTEMINFOLAMP_OFF', 1: 'SVB4_SYSTEMINFOLAMP_STANDBY', 2: 'SVB4_SYSTEMINFOLAMP_ACTIVE', 3: 'SVB4_SYSTEMINFOLAMP_ERROR'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_SystemInfoLamp', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_systeminfolamp(uint8_t* data, int svb4_systeminfolamp) {
    _svb4_systeminfolamp_x = svb4_systeminfolamp;
    Byte to_set(data + 1);
    to_set.set_value(_svb4_systeminfolamp_x, 0, 2);
}

Svb42c2* Svb42c2::set_svb4_checksum(int svb4_checksum) {
    _svb4_checksum = svb4_checksum;
    return this;
}

// config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'SVB4_Checksum', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svb42c2::set_p_svb4_checksum(uint8_t* data, int svb4_checksum) {
    svb4_checksum = ProtocolData::BoundedValue(0, 255, svb4_checksum);
    _svb4_checksum_x = svb4_checksum;
    Byte to_set(data + 0);
    to_set.set_value(_svb4_checksum_x, 0, 8);
}

SVB_4_2c2::SVB4_obstacle_info_9Type Svb42c2::svb4_obstacle_info_9(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(6, 2);
  
    SVB_4_2c2::SVB4_obstacle_info_9Type ret = static_cast<SVB_4_2c2::SVB4_obstacle_info_9Type>(x);
    return ret;
}

SVB_4_2c2::SVB4_obstacle_info_8Type Svb42c2::svb4_obstacle_info_8(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(4, 2);
  
    SVB_4_2c2::SVB4_obstacle_info_8Type ret = static_cast<SVB_4_2c2::SVB4_obstacle_info_8Type>(x);
    return ret;
}

SVB_4_2c2::SVB4_obstacle_info_7Type Svb42c2::svb4_obstacle_info_7(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(2, 2);
  
    SVB_4_2c2::SVB4_obstacle_info_7Type ret = static_cast<SVB_4_2c2::SVB4_obstacle_info_7Type>(x);
    return ret;
}

SVB_4_2c2::SVB4_obstacle_info_6Type Svb42c2::svb4_obstacle_info_6(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(0, 2);
  
    SVB_4_2c2::SVB4_obstacle_info_6Type ret = static_cast<SVB_4_2c2::SVB4_obstacle_info_6Type>(x);
    return ret;
}

SVB_4_2c2::SVB4_obstacle_info_5Type Svb42c2::svb4_obstacle_info_5(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(6, 2);
  
    SVB_4_2c2::SVB4_obstacle_info_5Type ret = static_cast<SVB_4_2c2::SVB4_obstacle_info_5Type>(x);
    return ret;
}

SVB_4_2c2::SVB4_obstacle_info_4Type Svb42c2::svb4_obstacle_info_4(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(4, 2);
  
    SVB_4_2c2::SVB4_obstacle_info_4Type ret = static_cast<SVB_4_2c2::SVB4_obstacle_info_4Type>(x);
    return ret;
}

SVB_4_2c2::SVB4_obstacle_info_3Type Svb42c2::svb4_obstacle_info_3(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(2, 2);
  
    SVB_4_2c2::SVB4_obstacle_info_3Type ret = static_cast<SVB_4_2c2::SVB4_obstacle_info_3Type>(x);
    return ret;
}

SVB_4_2c2::SVB4_obstacle_info_2_Copy_1Type Svb42c2::svb4_obstacle_info_2_copy_1(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(0, 2);
  
    SVB_4_2c2::SVB4_obstacle_info_2_Copy_1Type ret = static_cast<SVB_4_2c2::SVB4_obstacle_info_2_Copy_1Type>(x);
    return ret;
}

SVB_4_2c2::SVB4_obstacle_info_10Type Svb42c2::svb4_obstacle_info_10(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(0, 2);
  
    SVB_4_2c2::SVB4_obstacle_info_10Type ret = static_cast<SVB_4_2c2::SVB4_obstacle_info_10Type>(x);
    return ret;
}

SVB_4_2c2::SVB4_obstacle_info_2Type Svb42c2::svb4_obstacle_info_2(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(6, 2);
  
    SVB_4_2c2::SVB4_obstacle_info_2Type ret = static_cast<SVB_4_2c2::SVB4_obstacle_info_2Type>(x);
    return ret;
}

int Svb42c2::svb4_livecounter(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(4, 4);
  
    int ret = x;
    return ret;
}

int Svb42c2::svb4_reserved01(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x1 = t0.get_byte(2, 6);
  
    Byte t1(bytes + 5);
    int32_t x2 = t1.get_byte(0, 8);

    Byte t2(bytes + 6);
    int32_t x3 = t2.get_byte(0, 8);
  
    Byte t3(bytes + 7);
    int32_t x4 = t3.get_byte(0, 4);

    x4 <<= 8;
    x4 |= x3;
    x4 <<= 8;
    x4 |= x2;
    x4 <<= 8;
    x4 |= x1;
  
    int ret = x4;
    return ret;
}

SVB_4_2c2::SVB4_obstacle_info_1Type Svb42c2::svb4_obstacle_info_1(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(4, 2);
  
    SVB_4_2c2::SVB4_obstacle_info_1Type ret = static_cast<SVB_4_2c2::SVB4_obstacle_info_1Type>(x);
    return ret;
}

SVB_4_2c2::SVB4_TakeoverRequestInfoLampType Svb42c2::svb4_takeoverrequestinfolamp(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(2, 2);
  
    SVB_4_2c2::SVB4_TakeoverRequestInfoLampType ret = static_cast<SVB_4_2c2::SVB4_TakeoverRequestInfoLampType>(x);
    return ret;
}

SVB_4_2c2::SVB4_SystemInfoLampType Svb42c2::svb4_systeminfolamp(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(0, 2);
  
    SVB_4_2c2::SVB4_SystemInfoLampType ret = static_cast<SVB_4_2c2::SVB4_SystemInfoLampType>(x);
    return ret;
}

int Svb42c2::svb4_checksum(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 0);
    int32_t x = t0.get_byte(0, 8);
  
    int ret = x;
    return ret;
}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
