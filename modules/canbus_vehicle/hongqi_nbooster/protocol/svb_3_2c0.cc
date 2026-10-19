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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/svb_3_2c0.h"

 #include "modules/drivers/canbus/common/byte.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

const int32_t Svb32c0::ID = 0x2C0;

// public
Svb32c0::Svb32c0() {
    Reset();
}

uint32_t Svb32c0::GetPeriod() const {
    // TODO(All) : modify every protocol's period manually
    static const uint32_t PERIOD = 20 * 1000;
    return PERIOD;
}

void Svb32c0::Parse(const std::uint8_t* bytes, int32_t length,
    Hongqi_nbooster* chassis) const {
    chassis->mutable_svb_3_2c0()->set_svb3_abs_vehiclespeed(svb3_abs_vehiclespeed(bytes, length));
    chassis->mutable_svb_3_2c0()->set_svb3_reserved1(svb3_reserved1(bytes, length));
    //chassis->mutable_svb_3_2c0()->set_svb3_reserved2(svb3_reserved2(bytes, length));
    chassis->mutable_svb_3_2c0()->set_svb3_ai(svb3_ai(bytes, length));
    chassis->mutable_svb_3_2c0()->set_svb3_livecounter(svb3_livecounter(bytes, length));
    chassis->mutable_svb_3_2c0()->set_svb3_checksum(svb3_checksum(bytes, length));
}

void Svb32c0::UpdateData_Heartbeat(uint8_t* data) {
    // TODO(All) : you should add the heartbeat manually
 }

 void Svb32c0::UpdateData(uint8_t* data) {
    set_p_svb3_abs_vehiclespeed(data, _svb3_abs_vehiclespeed);
    set_p_svb3_reserved1(data, _svb3_reserved1);
    set_p_svb3_reserved2(data, _svb3_reserved2);
    set_p_svb3_ai(data, _svb3_ai);
    // UpdateMore(data);
}

// use update_more() to add live_counter and checksum, remember to delete them in update_data()
void Svb32c0::UpdateMore(uint8_t* data) {
    _svb3_livecounter += 1;
    _svb3_livecounter %= 16;
    set_p_svb3_livecounter(data, _svb3_livecounter);
    _svb3_checksum = data[1] ^
                     data[2] ^
                     data[3] ^
                     data[4] ^
                     data[5] ^
                     data[6] ^
                     data[7];
    set_p_svb3_checksum(data, _svb3_checksum);
}

void Svb32c0::Reset() {
    _svb3_abs_vehiclespeed = 0.0;
    _svb3_reserved1 = 0;
    _svb3_reserved2 = 0;
    _svb3_ai = 0.0;
}

Svb32c0* Svb32c0::set_svb3_livecounter(int svb3_livecounter) {
    _svb3_livecounter = svb3_livecounter;
    return this;
}

// config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'SVB3_Livecounter', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
void Svb32c0::set_p_svb3_livecounter(uint8_t* data, int svb3_livecounter) {
    svb3_livecounter = ProtocolData::BoundedValue(0, 15, svb3_livecounter);
    _svb3_livecounter_x = svb3_livecounter;
    Byte to_set(data + 7);
    to_set.set_value(_svb3_livecounter_x, 4, 4);
}

Svb32c0* Svb32c0::set_svb3_abs_vehiclespeed(double svb3_abs_vehiclespeed) {
    _svb3_abs_vehiclespeed = svb3_abs_vehiclespeed;
    return this;
}

// config detail: {'description': 'Vehicle speed output from ABS(derived from raw data)', 'offset': 0.0, 'precision': 0.01, 'len': 15, 'name': 'SVB3_ABS_VehicleSpeed', 'is_signed_var': False, 'physical_range': '[0|327.67]', 'bit': 24, 'type': 'double', 'order': 'intel', 'physical_unit': 'Km/h'}
void Svb32c0::set_p_svb3_abs_vehiclespeed(uint8_t* data, double svb3_abs_vehiclespeed) {
    svb3_abs_vehiclespeed = ProtocolData::BoundedValue(0.0, 327.67, svb3_abs_vehiclespeed);
    _svb3_abs_vehiclespeed_x = svb3_abs_vehiclespeed / 0.010000;
    int x = _svb3_abs_vehiclespeed_x;
    uint8_t t = 0;
    t = x & 0xFF;
    Byte to_set0(data + 3);
    to_set0.set_value(t, 0, 8);
    x >>= 8;
    t = x & 0x7F;
    Byte to_set1(data + 4);
    to_set1.set_value(t, 0, 7);
}

Svb32c0* Svb32c0::set_svb3_reserved1(int svb3_reserved1) {
    _svb3_reserved1 = svb3_reserved1;
    return this;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'SVB3_Reserved1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 20, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svb32c0::set_p_svb3_reserved1(uint8_t* data, int svb3_reserved1) {
    svb3_reserved1 = ProtocolData::BoundedValue(0, 15, svb3_reserved1);
    _svb3_reserved1_x = svb3_reserved1;
    Byte to_set(data + 2);
    to_set.set_value(_svb3_reserved1_x, 4, 4);
}

Svb32c0* Svb32c0::set_svb3_reserved2(int svb3_reserved2) {
    _svb3_reserved2 = svb3_reserved2;
    return this;
}

// config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 21, 'name': 'SVB3_Reserved2', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 39, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svb32c0::set_p_svb3_reserved2(uint8_t* data, int svb3_reserved2) {
    svb3_reserved2 = ProtocolData::BoundedValue(0, 0, svb3_reserved2);
    _svb3_reserved2_x = svb3_reserved2;
    int x = _svb3_reserved2_x;
    uint8_t t = 0;
    t = x & 0x1;
    Byte to_set0(data + 4);
    to_set0.set_value(t, 7, 1);
    x >>= 1;
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

Svb32c0* Svb32c0::set_svb3_ai(double svb3_ai) {
    _svb3_ai = svb3_ai;
    return this;
}

// config detail: {'description': 'Longitudinal acceleration(raw data;sign depends on mounting position)front of vehicle=positive,back of vehicle=negative', 'offset': -20.0, 'precision': 0.01, 'len': 12, 'name': 'SVB3_AI', 'is_signed_var': False, 'physical_range': '[-20|20]', 'bit': 8, 'type': 'double', 'order': 'intel', 'physical_unit': 'm/s2'}
void Svb32c0::set_p_svb3_ai(uint8_t* data, double svb3_ai) {
    svb3_ai = ProtocolData::BoundedValue(-20.0, 20.0, svb3_ai);
    _svb3_ai_x = (svb3_ai - -20.000000) / 0.010000;
    int x = _svb3_ai_x;
    uint8_t t = 0;
    t = x & 0xFF;
    Byte to_set0(data + 1);
    to_set0.set_value(t, 0, 8);
    x >>= 8;
    t = x & 0xF;
    Byte to_set1(data + 2);
    to_set1.set_value(t, 0, 4);
}

Svb32c0* Svb32c0::set_svb3_checksum(int svb3_checksum) {
    _svb3_checksum = svb3_checksum;
    return this;
}

// config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'SVB3_Checksum', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svb32c0::set_p_svb3_checksum(uint8_t* data, int svb3_checksum) {
    svb3_checksum = ProtocolData::BoundedValue(0, 255, svb3_checksum);
    _svb3_checksum_x = svb3_checksum;
    Byte to_set(data + 0);
    to_set.set_value(_svb3_checksum_x, 0, 8);
}

int Svb32c0::svb3_livecounter(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(4, 4);
  
    int ret = x;
    return ret;

}

double Svb32c0::svb3_abs_vehiclespeed(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(0, 8);
  
    Byte t1(bytes + 4);
    int32_t t = t1.get_byte(0, 7);
    t <<= 8;
    t |= x;
  
    double ret = t * 0.010000;
    return ret;
}

int Svb32c0::svb3_reserved1(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(4, 4);
  
    int ret = x;
    return ret;

}

int Svb32c0::svb3_reserved2(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x1 = t0.get_byte(7, 1);
  
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

double Svb32c0::svb3_ai(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(0, 8);
  
    Byte t1(bytes + 2);
    int32_t t = t1.get_byte(0, 4);
    t <<= 8;
    t |= x;
  
    double ret = t * 0.010000 + -20.000000;
    return ret;
}

int Svb32c0::svb3_checksum(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 0);
    int32_t x = t0.get_byte(0, 8);
  
    int ret = x;
    return ret;
}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
