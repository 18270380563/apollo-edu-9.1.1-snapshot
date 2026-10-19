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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/svb_nbs_req_109.h"

 #include "modules/drivers/canbus/common/byte.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

const int32_t Svbnbsreq109::ID = 0x109;

// public
Svbnbsreq109::Svbnbsreq109() { 
    Reset(); }

uint32_t Svbnbsreq109::GetPeriod() const {
    // TODO(All) : modify every protocol's period manually
    static const uint32_t PERIOD = 20 * 1000;
    return PERIOD;
}

void Svbnbsreq109::Parse(const std::uint8_t* bytes, int32_t length,
    Hongqi_nbooster* chassis) const {
    chassis->mutable_svb_nbs_req_109()->set_svb_taraccepedalposition_reqst(svb_taraccepedalposition_reqst(bytes, length));
    chassis->mutable_svb_nbs_req_109()->set_svb_taraccepedalposition_req(svb_taraccepedalposition_req(bytes, length));
    chassis->mutable_svb_nbs_req_109()->set_svb_recubraketorquefunc_switch(svb_recubraketorquefunc_switch(bytes, length));
    chassis->mutable_svb_nbs_req_109()->set_svb_brkpelpos_decrequest(svb_brkpelpos_decrequest(bytes, length));
    chassis->mutable_svb_nbs_req_109()->set_svb_decmode_request(svb_decmode_request(bytes, length));
    chassis->mutable_svb_nbs_req_109()->set_svb_taracc_decrequest(svb_taracc_decrequest(bytes, length));
    chassis->mutable_svb_nbs_req_109()->set_svb_brkreq_level(svb_brkreq_level(bytes, length));
    chassis->mutable_svb_nbs_req_109()->set_livecounter_svb_nbs(livecounter_svb_nbs(bytes, length));
    chassis->mutable_svb_nbs_req_109()->set_checksum_svb_nbs(checksum_svb_nbs(bytes, length));
}

void Svbnbsreq109::UpdateData_Heartbeat(uint8_t* data) {
    // TODO(All) : you should add the heartbeat manually
 }

void Svbnbsreq109::UpdateData(uint8_t* data) {
    set_p_svb_taraccepedalposition_reqst(data, _svb_taraccepedalposition_reqst);
    set_p_svb_taraccepedalposition_req(data, _svb_taraccepedalposition_req);
    set_p_svb_recubraketorquefunc_switch(data, _svb_recubraketorquefunc_switch); 
    set_p_svb_brkpelpos_decrequest(data, _svb_brkpelpos_decrequest);
    set_p_svb_decmode_request(data, _svb_decmode_request);
    set_p_svb_taracc_decrequest(data, _svb_taracc_decrequest);
    set_p_svb_brkreq_level(data, _svb_brkreq_level);
    // set_p_checksum_svb_nbs(data, _checksum_svb_nbs);
    // UpdateMore(data);
}

void Svbnbsreq109::UpdateMore(uint8_t* data) {
    _livecounter_svb_nbs += 1;
    _livecounter_svb_nbs %= 16; 
    set_p_livecounter_svb_nbs(data, _livecounter_svb_nbs);
    _checksum_svb_nbs = data[1] ^
                        data[2] ^
                        data[3] ^
                        data[4] ^
                        data[5] ^
                        data[6] ^
                        data[7];
    set_p_checksum_svb_nbs(data, _checksum_svb_nbs);
}

void Svbnbsreq109::Reset() {
    // TODO(All) :  you should check this manually
    _svb_brkpelpos_decrequest = 0.0;
    _livecounter_svb_nbs = 0;
    _svb_decmode_request = 0;
    _svb_taracc_decrequest = 0.0;
    _checksum_svb_nbs = 0;
    _svb_recubraketorquefunc_switch = false;
    _svb_taraccepedalposition_reqst = 0;
    _svb_taraccepedalposition_req = 0.0;
    _svb_brkreq_level = 0;
}

Svbnbsreq109* Svbnbsreq109::set_svb_brkpelpos_decrequest(
    double svb_brkpelpos_decrequest) {
    _svb_brkpelpos_decrequest = svb_brkpelpos_decrequest;
    return this;
}

// config detail: {'description': 'This signal is target brake pedal position request for wheel brake.', 'enum': {16383: 'SVB_BRKPELPOS_DECREQUEST_INVALID'}, 'precision': 0.01, 'len': 14, 'name': 'SVB_BrkPelPos_DecRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|100]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': '%'}
void Svbnbsreq109::set_p_svb_brkpelpos_decrequest(uint8_t* data,
    double svb_brkpelpos_decrequest) {
    _svb_brkpelpos_decrequest_x = svb_brkpelpos_decrequest / 0.010000;
    int x = _svb_brkpelpos_decrequest_x;
    uint8_t t = 0;
    t = x & 0xFF;
    Byte to_set0(data + 3);
    to_set0.set_value(t, 0, 8);
    x >>= 8;
    t = x & 0x3F;
    Byte to_set1(data + 4);
    to_set1.set_value(t, 0, 6);
}

Svbnbsreq109* Svbnbsreq109::set_svb_recubraketorquefunc_switch(
    bool svb_recubraketorquefunc_switch) {
    _svb_recubraketorquefunc_switch = svb_recubraketorquefunc_switch;
    return this;
}
                                                                                                                                            
// config detail: {'name': 'SVB_RecuBrakeTorqueFunc_Switch', 'enum': {0: 'SVB_RECUBRAKETORQUEFUNC_SWITCH_ON', 1: 'SVB_RECUBRAKETORQUEFUNC_SWITCH_OFF'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 38, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svbnbsreq109::set_p_svb_recubraketorquefunc_switch(uint8_t* data,
    bool svb_recubraketorquefunc_switch) {
    _svb_recubraketorquefunc_switch_x = svb_recubraketorquefunc_switch;
    Byte to_set(data + 4); 
    to_set.set_value(_svb_recubraketorquefunc_switch_x, 6, 1); 
}

Svbnbsreq109* Svbnbsreq109::set_svb_taraccepedalposition_reqst(
    bool svb_taraccepedalposition_reqst) {
    _svb_taraccepedalposition_reqst = svb_taraccepedalposition_reqst;
    return this;
}

// config detail: {'description': 'Requested recuperative brake torque', 'enum': {0: 'SVB_TARACCEPEDALPOSITION_REQST_NO_REQUEST', 1: 'SVB_TARACCEPEDALPOSITION_REQST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_TarAccePedalPosition_ReqSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 39, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svbnbsreq109::set_p_svb_taraccepedalposition_reqst(uint8_t* data,
    bool svb_taraccepedalposition_reqst) {
    _svb_taraccepedalposition_reqst_x = svb_taraccepedalposition_reqst;
    Byte to_set(data + 4); 
    to_set.set_value(_svb_taraccepedalposition_reqst_x, 7, 1); 
}

Svbnbsreq109* Svbnbsreq109::set_svb_taraccepedalposition_req(
    double svb_taraccepedalposition_req) {
    _svb_taraccepedalposition_req = svb_taraccepedalposition_req;
    return this;
}

// config detail: {'description': 'Acceleration pedal position', 'offset': 0.0, 'precision': 0.4, 'len': 8, 'name': 'SVB_TarAccePedalPosition_Req', 'is_signed_var': False, 'physical_range': '[0|100]', 'bit': 40, 'type': 'double', 'order': 'intel', 'physical_unit': '%'}
void Svbnbsreq109::set_p_svb_taraccepedalposition_req(uint8_t* data,
    double svb_taraccepedalposition_req) {
    svb_taraccepedalposition_req = ProtocolData::BoundedValue(0.0, 100.0, svb_taraccepedalposition_req);
    _svb_taraccepedalposition_req_x = svb_taraccepedalposition_req / 0.400000;
    Byte to_set(data + 5); 
    to_set.set_value(_svb_taraccepedalposition_req_x, 0, 8); 
}

Svbnbsreq109* Svbnbsreq109::set_livecounter_svb_nbs(
    int livecounter_svb_nbs) {
    _livecounter_svb_nbs = livecounter_svb_nbs;
    return this;
}

// config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2,\xad15, 0, 1 \xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Livecounter_SVB_NBS', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
void Svbnbsreq109::set_p_livecounter_svb_nbs(uint8_t* data,
    int livecounter_svb_nbs) {
    livecounter_svb_nbs = ProtocolData::BoundedValue(0, 15, livecounter_svb_nbs);
    _livecounter_svb_nbs_x = livecounter_svb_nbs;
    Byte to_set(data + 7);
    to_set.set_value(_livecounter_svb_nbs_x, 4, 4);
}

Svbnbsreq109* Svbnbsreq109::set_svb_decmode_request(
    int svb_decmode_request) {
    _svb_decmode_request = svb_decmode_request;
    return this;
}

// config detail: {'description': 'This signal indicates the status of deceleration request status of SVB.', 'enum': {0: 'SVB_DECMODE_REQUEST_NO_REQUEST', 1: 'SVB_DECMODE_REQUEST_BRKPELPOS_REQUEST', 2: 'SVB_DECMODE_REQUEST_TARDEC_REQUEST', 3: 'SVB_DECMODE_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_DecMode_Request', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 18, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svbnbsreq109::set_p_svb_decmode_request(uint8_t* data,
    int svb_decmode_request) {
    _svb_decmode_request_x = svb_decmode_request;
    Byte to_set(data + 2);
    to_set.set_value(_svb_decmode_request_x, 2, 2);
}

Svbnbsreq109* Svbnbsreq109::set_svb_taracc_decrequest(
    double svb_taracc_decrequest) {
    _svb_taracc_decrequest = svb_taracc_decrequest;
    return this;
}

// config detail: {'description': 'This signal is deceleration request for wheel brake.', 'enum': {1023: 'SVB_TARACC_DECREQUEST_INVALID'}, 'precision': 0.01, 'len': 10, 'name': 'SVB_TarACC_DecRequest', 'is_signed_var': False, 'offset': -10.0, 'physical_range': '[-10|0]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': 'm/s2'}
void Svbnbsreq109::set_p_svb_taracc_decrequest(uint8_t* data,
    double svb_taracc_decrequest) {
    _svb_taracc_decrequest_x = (svb_taracc_decrequest - -10.000000) / 0.010000;
    int x = _svb_taracc_decrequest_x;
    uint8_t t = 0;
    t = x & 0xFF;
    Byte to_set0(data + 1);
    to_set0.set_value(t, 0, 8);
    x >>= 8;
    t = x & 0x3;
    Byte to_set1(data + 2);
    to_set1.set_value(t, 0, 2);
}

Svbnbsreq109* Svbnbsreq109::set_checksum_svb_nbs(
    int checksum_svb_nbs) {
    _checksum_svb_nbs = checksum_svb_nbs;
    return this;
}

// config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_SVB_NBS', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svbnbsreq109::set_p_checksum_svb_nbs(uint8_t* data,
    int checksum_svb_nbs) {
    checksum_svb_nbs = ProtocolData::BoundedValue(0, 255, checksum_svb_nbs);
    _checksum_svb_nbs_x = checksum_svb_nbs;
    Byte to_set(data + 0);
    to_set.set_value(_checksum_svb_nbs_x, 0, 8);
}

// config detail: {'name': 'SVB_BrkReq_Level', 'enum': {0: 'SVB_BRKREQ_LEVEL_NORMAL_BRAKE', 1: 'SVB_BRKREQ_LEVEL_EMERGENCY_BRAKE', 2: 'SVB_BRKREQ_LEVEL_RESERVED', 3: 'SVB_BRKREQ_LEVEL_RESERVED'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svbnbsreq109::set_p_svb_brkreq_level(uint8_t* data, int svb_brkreq_level) {
    _svb_brkreq_level_x = svb_brkreq_level;
    Byte to_set(data + 2);
    to_set.set_value(_svb_brkreq_level_x, 4, 2);
}

Svbnbsreq109* Svbnbsreq109::set_svb_brkreq_level(int svb_brkreq_level) {
    _svb_brkreq_level = svb_brkreq_level;
    return this;
}

SVB_NBS_REQ_109::SVB_TarAccePedalPosition_ReqStType Svbnbsreq109::svb_taraccepedalposition_reqst(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(7, 1);
  
    SVB_NBS_REQ_109::SVB_TarAccePedalPosition_ReqStType ret = static_cast<SVB_NBS_REQ_109::SVB_TarAccePedalPosition_ReqStType>(x);
    return ret;
}

double Svbnbsreq109::svb_taraccepedalposition_req(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 5);
    int32_t x = t0.get_byte(0, 8);
  
    double ret = x * 0.400000;
    return ret;
}

SVB_NBS_REQ_109::SVB_RecuBrakeTorqueFunc_SwitchType Svbnbsreq109::svb_recubraketorquefunc_switch(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(6, 1);
  
    SVB_NBS_REQ_109::SVB_RecuBrakeTorqueFunc_SwitchType ret = static_cast<SVB_NBS_REQ_109::SVB_RecuBrakeTorqueFunc_SwitchType>(x);
    return ret;
}

double Svbnbsreq109::svb_brkpelpos_decrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(0, 8);
  
    Byte t1(bytes + 4);
    int32_t t = t1.get_byte(0, 6);
    t <<= 8;
    t |= x;
  
    double ret = t * 0.010000;
    return ret;
}

int Svbnbsreq109::livecounter_svb_nbs(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(4, 4);
  
    int ret = x;
    return ret;
}

SVB_NBS_REQ_109::SVB_DecMode_RequestType Svbnbsreq109::svb_decmode_request(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(2, 2);
  
    SVB_NBS_REQ_109::SVB_DecMode_RequestType ret = static_cast<SVB_NBS_REQ_109::SVB_DecMode_RequestType>(x);
    return ret;
}

double Svbnbsreq109::svb_taracc_decrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(0, 8);
  
    Byte t1(bytes + 2);
    int32_t t = t1.get_byte(0, 2);
    t <<= 8;
    t |= x;
  
    double ret = t * 0.010000 + -10.000000;
    return ret;
}

int Svbnbsreq109::checksum_svb_nbs(const std::uint8_t* bytes, const int32_t length) const {
    // checksum_svb_nbs = ProtocolData::BoundedValue(0, 255, checksum_svb_nbs);
    Byte t0(bytes + 0);
    int32_t x = t0.get_byte(0, 8);
  
    int ret = x;
    return ret;
}

SVB_NBS_REQ_109::SVB_BrkReq_LevelType Svbnbsreq109::svb_brkreq_level(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(4, 2);
  
    SVB_NBS_REQ_109::SVB_BrkReq_LevelType ret = static_cast<SVB_NBS_REQ_109::SVB_BrkReq_LevelType>(x);
    return ret;
}
}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo

