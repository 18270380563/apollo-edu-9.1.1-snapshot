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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/svb_1_108.h"

 #include "modules/drivers/canbus/common/byte.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

const int32_t Svb1108::ID = 0x108;

// public
Svb1108::Svb1108() {
    Reset();
}

uint32_t Svb1108::GetPeriod() const {
    // TODO(All) : modify every protocol's period manually
    static const uint32_t PERIOD = 10 * 1000;
    return PERIOD;
  }

void Svb1108::Parse(const std::uint8_t* bytes, int32_t length,
    Hongqi_nbooster* chassis) const {
    chassis->mutable_svb_1_108()->set_svb_steeringspeedrequest(svb_steeringspeedrequest(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_gearrequest(svb_gearrequest(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_accelerationrequestst(svb_accelerationrequestst(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_takeoverrequest(svb_takeoverrequest(bytes, length));
    chassis->mutable_svb_1_108()->set_reserved_svb1(reserved_svb1(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_accelerationrequest(svb_accelerationrequest(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_standstillrequest(svb_standstillrequest(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_steeringspeedsignrequest(svb_steeringspeedsignrequest(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_steeringanglesignrequest(svb_steeringanglesignrequest(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_steeringanglerequest(svb_steeringanglerequest(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_steeringrequestst(svb_steeringrequestst(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_failurestatus(svb_failurestatus(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_decelerationrequestst(svb_decelerationrequestst(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_decelerationrequest(svb_decelerationrequest(bytes, length));
    chassis->mutable_svb_1_108()->set_svb_maxspeedlimit(svb_maxspeedlimit(bytes, length));
    chassis->mutable_svb_1_108()->set_livecounter_svb_1(livecounter_svb_1(bytes, length));
    chassis->mutable_svb_1_108()->set_checksum_svb_1(checksum_svb_1(bytes, length));
}

void Svb1108::UpdateData_Heartbeat(uint8_t* data) {
    // TODO(All) : you should add the heartbeat manually
 }

 void Svb1108::UpdateData(uint8_t* data) {
    set_p_svb_steeringspeedrequest(data, _svb_steeringspeedrequest);
    set_p_svb_gearrequest(data, _svb_gearrequest);
    set_p_svb_accelerationrequestst(data, _svb_accelerationrequestst);
    set_p_svb_takeoverrequest(data, _svb_takeoverrequest);
    set_p_reserved_svb1(data, _reserved_svb1);
    set_p_svb_accelerationrequest(data, _svb_accelerationrequest);
    set_p_svb_standstillrequest(data, _svb_standstillrequest);
    set_p_svb_steeringspeedsignrequest(data, _svb_steeringspeedsignrequest);
    set_p_svb_steeringanglesignrequest(data, _svb_steeringanglesignrequest);
    set_p_svb_steeringanglerequest(data, _svb_steeringanglerequest);
    set_p_svb_steeringrequestst(data, _svb_steeringrequestst);
    set_p_svb_failurestatus(data, _svb_failurestatus);
    set_p_svb_decelerationrequestst(data, _svb_decelerationrequestst);
    set_p_svb_decelerationrequest(data, _svb_decelerationrequest);
    set_p_svb_maxspeedlimit(data, _svb_maxspeedlimit);
    // UpdateMore(data);
  }

void Svb1108::UpdateMore(uint8_t* data) {
    _livecounter_svb_1 += 1;
    _livecounter_svb_1 %= 16;
    set_p_livecounter_svb_1(data, _livecounter_svb_1);
    _checksum_svb_1 = data[1] ^
                      data[2] ^
                      data[3] ^
                      data[4] ^
                      data[5] ^
                      data[6] ^
                      data[7];
    set_p_checksum_svb_1(data, _checksum_svb_1);
}

void Svb1108::Reset() {
      // TODO(All) :  you should check this manually
    _svb_steeringspeedrequest = 0;
    _svb_gearrequest = 0;
    _svb_accelerationrequestst = false;
    _svb_takeoverrequest = false;
    _reserved_svb1 = false;
    _svb_accelerationrequest = 0.0;
    _svb_standstillrequest = false;
    _svb_steeringspeedsignrequest = 0;
    _svb_steeringanglesignrequest = 0;
    _svb_steeringanglerequest = 0.0;
    _svb_steeringrequestst = 0;
    //_svb_failurestatus = 0;
    _svb_decelerationrequestst = 0;
    _svb_decelerationrequest = 0.0;
    _svb_maxspeedlimit = 0;
    // _checksum_svb_1 = 0;
}

Svb1108* Svb1108::set_svb_maxspeedlimit(int svb_maxspeedlimit) {
    _svb_maxspeedlimit = svb_maxspeedlimit;
    return this;
}

// config detail: {'description': 'ndicate the Max speed limit', 'enum': {0: 'SVB_MAXSPEEDLIMIT_LIMIT', 1: 'SVB_MAXSPEEDLIMIT_NO_LIMIT'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_MaxSpeedLimit', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 54, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_svb_maxspeedlimit(uint8_t* data, int svb_maxspeedlimit) {
    _svb_maxspeedlimit_x = svb_maxspeedlimit;
    Byte to_set(data + 6);
    to_set.set_value(_svb_maxspeedlimit_x, 6, 1);
}

Svb1108* Svb1108::set_svb_steeringspeedrequest(double svb_steeringspeedrequest) {
    _svb_steeringspeedrequest = svb_steeringspeedrequest;
    return this;
}

// config detail: {'description': 'Steering wheel speed request from SVB.The maximum speed shall bedetermined in calibrating phase.', 'enum': {511: 'SVB_STEERINGSPEEDREQUEST_INVALID'}, 'precision': 1.0, 'len': 9, 'name': 'SVB_SteeringSpeedRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|500]', 'bit': 39, 'type': 'enum', 'order': 'intel', 'physical_unit': 'deg/s'}
void Svb1108::set_p_svb_steeringspeedrequest(uint8_t* data, double svb_steeringspeedrequest) {
    _svb_steeringspeedrequest_x = svb_steeringspeedrequest;
    int x = _svb_steeringspeedrequest_x;
    uint8_t t = 0;
    t = x & 0x1;
    Byte to_set0(data + 4);
    to_set0.set_value(t, 7, 1);
    x >>= 1;
    t = x & 0xFF;
    Byte to_set1(data + 5);
    to_set1.set_value(t, 0, 8);
}

Svb1108* Svb1108::set_livecounter_svb_1(int livecounter_svb_1) {
    _livecounter_svb_1 = livecounter_svb_1;
    return this;
}

// config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Livecounter_SVB_1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
void Svb1108::set_p_livecounter_svb_1(uint8_t* data, int livecounter_svb_1) {
    livecounter_svb_1 = ProtocolData::BoundedValue(0, 15, livecounter_svb_1);
    _livecounter_svb_1_x = livecounter_svb_1;
    Byte to_set(data + 7);
    to_set.set_value(_livecounter_svb_1_x, 4, 4);
}

Svb1108* Svb1108::set_svb_gearrequest(int svb_gearrequest) {
    _svb_gearrequest = svb_gearrequest;
    return this;
}

// config detail: {'description': 'Gear request from SVB. SVB shall keep sending requested gear signalvalue in automatically mode.', 'enum': {0: 'SVB_GEARREQUEST_NO_REQUEST', 1: 'SVB_GEARREQUEST_P', 2: 'SVB_GEARREQUEST_R', 3: 'SVB_GEARREQUEST_N', 4: 'SVB_GEARREQUEST_D', 5: 'SVB_GEARREQUEST_RESERVED', 6: 'SVB_GEARREQUEST_RESERVED', 7: 'SVB_GEARREQUEST_INVALID_VALUE'}, 'precision': 1.0, 'len': 3, 'name': 'SVB_GearRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 57, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_svb_gearrequest(uint8_t* data, int svb_gearrequest) {
    _svb_gearrequest_x = svb_gearrequest;
    Byte to_set(data + 7);
    to_set.set_value(_svb_gearrequest_x, 1, 3);
}

Svb1108* Svb1108::set_svb_accelerationrequestst(bool svb_accelerationrequestst) {
    _svb_accelerationrequestst = svb_accelerationrequestst;
    return this;
}

// config detail: {'description': 'This signal is the status of acceleration request for driving.', 'enum': {0: 'SVB_ACCELERATIONREQUESTST_NO_REQUEST', 1: 'SVB_ACCELERATIONREQUESTST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_AccelerationRequestSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 56, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_svb_accelerationrequestst(uint8_t* data, bool svb_accelerationrequestst) {
    _svb_accelerationrequestst_x = svb_accelerationrequestst;
    Byte to_set(data + 7);
    to_set.set_value(_svb_accelerationrequestst_x, 0, 1);
}

Svb1108* Svb1108::set_svb_takeoverrequest(bool svb_takeoverrequest) {
    _svb_takeoverrequest = svb_takeoverrequest;
    return this;
}

// config detail: {'description': 'This signal indicates the taking over request from SVB.When the requestfrom SVB could not be actuated correctly(including message lost, receivedsignal value unexpected) , this signal shall be set to ask driver taking over.', 'enum': {0: 'SVB_TAKEOVERREQUEST_NO_REQUEST', 1: 'SVB_TAKEOVERREQUEST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_TakeOverRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 55, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_svb_takeoverrequest(uint8_t* data, bool svb_takeoverrequest) {
    _svb_takeoverrequest_x = svb_takeoverrequest;
    Byte to_set(data + 6);
    to_set.set_value(_svb_takeoverrequest_x, 7, 1);
}

Svb1108* Svb1108::set_reserved_svb1(bool reserved_svb1) {
    _reserved_svb1 = reserved_svb1;
    return this;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'Reserved_SVB1', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 54, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_reserved_svb1(uint8_t* data, bool reserved_svb1) {
    _reserved_svb1_x = reserved_svb1;
    Byte to_set(data + 6);
    to_set.set_value(_reserved_svb1_x, 6, 1);
}

Svb1108* Svb1108::set_svb_accelerationrequest(double svb_accelerationrequest) {
    _svb_accelerationrequest = svb_accelerationrequest;
    return this;
}

// config detail: {'description': 'Acceleration request from SVB.', 'enum': {63: 'SVB_ACCELERATIONREQUEST_INVALID'}, 'precision': 0.1, 'len': 6, 'name': 'SVB_AccelerationRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|6.3]', 'bit': 48, 'type': 'enum', 'order': 'intel', 'physical_unit': 'm/s2'}
void Svb1108::set_p_svb_accelerationrequest(uint8_t* data, double svb_accelerationrequest) {
    _svb_accelerationrequest_x = svb_accelerationrequest / 0.100000;
    Byte to_set(data + 6);
    to_set.set_value(_svb_accelerationrequest_x, 0, 6);
}

Svb1108* Svb1108::set_svb_standstillrequest(bool svb_standstillrequest) {
    _svb_standstillrequest = svb_standstillrequest;
    return this;
}

// config detail: {'description': 'This signal indicates the stand-still request. This signal is used for comfortbraking when vehicle speed is lower than 2km/h(calibrated value).', 'enum': {0: 'SVB_STANDSTILLREQUEST_NO_REQUEST', 1: 'SVB_STANDSTILLREQUEST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_StandStillRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 38, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_svb_standstillrequest(uint8_t* data, bool svb_standstillrequest) {
    _svb_standstillrequest_x = svb_standstillrequest;
    Byte to_set(data + 4);
    to_set.set_value(_svb_standstillrequest_x, 6, 1);
}

Svb1108* Svb1108::set_svb_steeringspeedsignrequest(int svb_steeringspeedsignrequest) {
    _svb_steeringspeedsignrequest = svb_steeringspeedsignrequest;
    return this;
}

// config detail: {'description': 'Steering wheel speed sign request from SVB.', 'enum': {0: 'SVB_STEERINGSPEEDSIGNREQUEST_NO_REQUEST', 1: 'SVB_STEERINGSPEEDSIGNREQUEST_REQUEST', 2: 'SVB_STEERINGSPEEDSIGNREQUEST_RESERVED', 3: 'SVB_STEERINGSPEEDSIGNREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_SteeringSpeedSignRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 36, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_svb_steeringspeedsignrequest(uint8_t* data, int svb_steeringspeedsignrequest) {
    _svb_steeringspeedsignrequest_x = svb_steeringspeedsignrequest;
    Byte to_set(data + 4);
    to_set.set_value(_svb_steeringspeedsignrequest_x, 4, 2);
}

Svb1108* Svb1108::set_svb_steeringanglesignrequest(int svb_steeringanglesignrequest) {
    _svb_steeringanglesignrequest = svb_steeringanglesignrequest;
    return this;
}

// config detail: {'description': 'Steering wheel angle sign request from SVB.', 'enum': {0: 'SVB_STEERINGANGLESIGNREQUEST_NO_REQUEST', 1: 'SVB_STEERINGANGLESIGNREQUEST_LEFT_POSITIVE', 2: 'SVB_STEERINGANGLESIGNREQUEST_RIGHT_NEGATIVE', 3: 'SVB_STEERINGANGLESIGNREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_SteeringAngleSignRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 34, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_svb_steeringanglesignrequest(uint8_t* data, int svb_steeringanglesignrequest) {
    _svb_steeringanglesignrequest_x = svb_steeringanglesignrequest;
    Byte to_set(data + 4);
    to_set.set_value(_svb_steeringanglesignrequest_x, 2, 2);
}

Svb1108* Svb1108::set_svb_steeringanglerequest(double svb_steeringanglerequest) {
    _svb_steeringanglerequest = svb_steeringanglerequest;
    return this;
}

// config detail: {'description': 'Steering wheel angle request from SVB. The maximum angle shall bedetermined in calibrating phase.', 'enum': {1023: 'SVB_STEERINGANGLEREQUEST_INVALID'}, 'precision': 1.0, 'len': 10, 'name': 'SVB_SteeringAngleRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1022]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': 'deg'}
void Svb1108::set_p_svb_steeringanglerequest(uint8_t* data, double svb_steeringanglerequest) {
    _svb_steeringanglerequest_x = svb_steeringanglerequest;
    int x = _svb_steeringanglerequest_x;
    uint8_t t = 0;
    t = x & 0xFF;
    Byte to_set0(data + 3);
    to_set0.set_value(t, 0, 8);
    x >>= 8;
    t = x & 0x3;
    Byte to_set1(data + 4);
    to_set1.set_value(t, 0, 2);
}

Svb1108* Svb1108::set_svb_steeringrequestst(int svb_steeringrequestst) {
    _svb_steeringrequestst = svb_steeringrequestst;
    return this;
}

// config detail: {'description': 'This signal is the status of steering wheel request for driving.', 'enum': {0: 'SVB_STEERINGREQUESTST_NO_REQUEST', 1: 'SVB_STEERINGREQUESTST_REQUEST', 2: 'SVB_STEERINGREQUESTST_RESERVED', 3: 'SVB_STEERINGREQUESTST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_SteeringRequestSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 22, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_svb_steeringrequestst(uint8_t* data, int svb_steeringrequestst) {
    _svb_steeringrequestst_x = svb_steeringrequestst;
    Byte to_set(data + 2);
    to_set.set_value(_svb_steeringrequestst_x, 6, 2);
}

Svb1108* Svb1108::set_svb_failurestatus(int svb_failurestatus) {
    _svb_failurestatus = svb_failurestatus;
    return this;
}

// config detail: {'description': 'This signal indicates the status of SVB system. When SVB detected its ownfault, this signal shall be set.', 'enum': {0: 'SVB_FAILURESTATUS_NO_ERROR', 1: 'SVB_FAILURESTATUS_ERROR', 2: 'SVB_FAILURESTATUS_RESERVED', 3: 'SVB_FAILURESTATUS_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_FailureStatus', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_svb_failurestatus(uint8_t* data, int svb_failurestatus) {
    _svb_failurestatus_x = svb_failurestatus;
    Byte to_set(data + 2);
    to_set.set_value(_svb_failurestatus_x, 4, 2);
}

Svb1108* Svb1108::set_svb_decelerationrequestst(int svb_decelerationrequestst) {
    _svb_decelerationrequestst = svb_decelerationrequestst;
    return this;
}

// config detail: {'description': 'This signal indicates the status of deceleration request status of SVB.', 'enum': {0: 'SVB_DECELERATIONREQUESTST_NO_REQUEST', 1: 'SVB_DECELERATIONREQUESTST_ACC_REQUEST', 2: 'SVB_DECELERATIONREQUESTST_AEB_REQUEST', 3: 'SVB_DECELERATIONREQUESTST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_DecelerationRequestSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 18, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_svb_decelerationrequestst(uint8_t* data, int svb_decelerationrequestst) {
    _svb_decelerationrequestst_x = svb_decelerationrequestst;
    Byte to_set(data + 2);
    to_set.set_value(_svb_decelerationrequestst_x, 2, 2);
}

Svb1108* Svb1108::set_svb_decelerationrequest(double svb_decelerationrequest) {
    _svb_decelerationrequest = svb_decelerationrequest;
    return this;
}

// config detail: {'description': 'This signal is deceleration request for wheel brake.', 'enum': {1023: 'SVB_DECELERATIONREQUEST_INVALID'}, 'precision': 0.01, 'len': 10, 'name': 'SVB_DecelerationRequest', 'is_signed_var': False, 'offset': -10.0, 'physical_range': '[-10|0]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': 'm/s2'}
void Svb1108::set_p_svb_decelerationrequest(uint8_t* data, double svb_decelerationrequest) {
    _svb_decelerationrequest_x = (svb_decelerationrequest - -10.000000) / 0.010000;
    int x = _svb_decelerationrequest_x;
    uint8_t t = 0;
    t = x & 0xFF;
    Byte to_set0(data + 1);
    to_set0.set_value(t, 0, 8);
    x >>= 8;
    t = x & 0x3;
    Byte to_set1(data + 2);
    to_set1.set_value(t, 0, 2);
}

Svb1108* Svb1108::set_checksum_svb_1(int checksum_svb_1) {
    _checksum_svb_1 = checksum_svb_1;
    return this;
}

// config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_SVB_1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svb1108::set_p_checksum_svb_1(uint8_t* data, int checksum_svb_1) {
    checksum_svb_1 = ProtocolData::BoundedValue(0, 255, checksum_svb_1);
    _checksum_svb_1_x = checksum_svb_1;
    Byte to_set(data + 0);
    to_set.set_value(_checksum_svb_1_x, 0, 8);
}

SVB_1_108::SVB_MaxSpeedLimitType Svb1108::svb_maxspeedlimit(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(6, 1);

    SVB_1_108::SVB_MaxSpeedLimitType ret = static_cast<SVB_1_108::SVB_MaxSpeedLimitType>(x);
    return ret;
}

int Svb1108::svb_steeringspeedrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(7, 1);

    Byte t1(bytes + 5);
    int32_t t = t1.get_byte(0, 8);
    t <<= 8;
    t |= x;

    int ret = t;
    return ret;
}

int Svb1108::livecounter_svb_1(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(4, 4);

    int ret = x;
    return ret;
}

SVB_1_108::SVB_GearRequestType Svb1108::svb_gearrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(1, 3);

    SVB_1_108::SVB_GearRequestType ret = static_cast<SVB_1_108::SVB_GearRequestType>(x);
    return ret;
}

SVB_1_108::SVB_AccelerationRequestStType Svb1108::svb_accelerationrequestst(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(0, 1);

    SVB_1_108::SVB_AccelerationRequestStType ret = static_cast<SVB_1_108::SVB_AccelerationRequestStType>(x);
    return ret;
}

SVB_1_108::SVB_TakeOverRequestType Svb1108::svb_takeoverrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(7, 1);

    SVB_1_108::SVB_TakeOverRequestType ret = static_cast<SVB_1_108::SVB_TakeOverRequestType>(x);
    return ret;
}

int Svb1108::reserved_svb1(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(6, 1);

    int ret = x;
    return ret;
}

double Svb1108::svb_accelerationrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(0, 6);

    double ret = x * 0.100000;
    return ret;
}

SVB_1_108::SVB_StandStillRequestType Svb1108::svb_standstillrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(6, 1);

    SVB_1_108::SVB_StandStillRequestType ret = static_cast<SVB_1_108::SVB_StandStillRequestType>(x);
    return ret;
}

SVB_1_108::SVB_SteeringSpeedSignRequestType Svb1108::svb_steeringspeedsignrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(4, 2);

    SVB_1_108::SVB_SteeringSpeedSignRequestType ret = static_cast<SVB_1_108::SVB_SteeringSpeedSignRequestType>(x);
    return ret;
}

SVB_1_108::SVB_SteeringAngleSignRequestType Svb1108::svb_steeringanglesignrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(2, 2);

    SVB_1_108::SVB_SteeringAngleSignRequestType ret = static_cast<SVB_1_108::SVB_SteeringAngleSignRequestType>(x);
    return ret;
}

int Svb1108::svb_steeringanglerequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(0, 8);

    Byte t1(bytes + 4);
    int32_t t = t1.get_byte(0, 2);
    t <<= 8;
    t |= x;

    int ret = t;
    return ret;
}

SVB_1_108::SVB_SteeringRequestStType Svb1108::svb_steeringrequestst(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(6, 2);

    SVB_1_108::SVB_SteeringRequestStType ret = static_cast<SVB_1_108::SVB_SteeringRequestStType>(x);
    return ret;
}

SVB_1_108::SVB_FailureStatusType Svb1108::svb_failurestatus(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(4, 2);

    SVB_1_108::SVB_FailureStatusType ret = static_cast<SVB_1_108::SVB_FailureStatusType>(x);
    return ret;
}

SVB_1_108::SVB_DecelerationRequestStType Svb1108::svb_decelerationrequestst(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(2, 2);

    SVB_1_108::SVB_DecelerationRequestStType ret = static_cast<SVB_1_108::SVB_DecelerationRequestStType>(x);
    return ret;
}

double Svb1108::svb_decelerationrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(0, 8);

    Byte t1(bytes + 2);
    int32_t t = t1.get_byte(0, 2);
    t <<= 8;
    t |= x;

    double ret = t * 0.010000 + -10.000000;
    return ret;
}

int Svb1108::checksum_svb_1(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 0);
    int32_t x = t0.get_byte(0, 8);
  
    int ret = x;
    return ret;
}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo 
