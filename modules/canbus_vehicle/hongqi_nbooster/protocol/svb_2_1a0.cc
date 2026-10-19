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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/svb_2_1a0.h"

 #include "modules/drivers/canbus/common/byte.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

const int32_t Svb21a0::ID = 0x1a0;

// public
Svb21a0::Svb21a0() {
    Reset();
}

uint32_t Svb21a0::GetPeriod() const {
    // TODO(All) : modify every protocol's period manually
    static const uint32_t PERIOD = 100 * 1000;
    return PERIOD;
  }

void Svb21a0::Parse(const std::uint8_t* bytes, int32_t length,
            Hongqi_nbooster* chassis) const {
    chassis->mutable_svb_2_1a0()->set_reserved_svb2_03(reserved_svb2_03(bytes, length));
    chassis->mutable_svb_2_1a0()->set_reserved_svb2_02(reserved_svb2_02(bytes, length));
    chassis->mutable_svb_2_1a0()->set_reserved_svb2(reserved_svb2(bytes, length));
    chassis->mutable_svb_2_1a0()->set_request_turnoff_lowvoltage(request_turnoff_lowvoltage(bytes, length));
    chassis->mutable_svb_2_1a0()->set_svb_epbrequest(svb_epbrequest(bytes, length));
    chassis->mutable_svb_2_1a0()->set_position_light_request(position_light_request(bytes, length));
    chassis->mutable_svb_2_1a0()->set_hu_speechrequest_setlefttemp(hu_speechrequest_setlefttemp(bytes, length));
    chassis->mutable_svb_2_1a0()->set_hu_speechrequest_setleftblw(hu_speechrequest_setleftblw(bytes, length));
    chassis->mutable_svb_2_1a0()->set_hu_speechrequest_lefttempadjsw(hu_speechrequest_lefttempadjsw(bytes, length));
    chassis->mutable_svb_2_1a0()->set_hu_speechrequest_frsrec(hu_speechrequest_frsrec(bytes, length));
    chassis->mutable_svb_2_1a0()->set_hu_speechrequest_frseatheatlevel(hu_speechrequest_frseatheatlevel(bytes, length));
    chassis->mutable_svb_2_1a0()->set_hu_speechrequest_flseatheatlevel(hu_speechrequest_flseatheatlevel(bytes, length));
    chassis->mutable_svb_2_1a0()->set_hu_speechrequest_blwleveladjsw(hu_speechrequest_blwleveladjsw(bytes, length));
    chassis->mutable_svb_2_1a0()->set_hu_speechrequest_ac(hu_speechrequest_ac(bytes, length));
    chassis->mutable_svb_2_1a0()->set_lowbeam_request(lowbeam_request(bytes, length));
    chassis->mutable_svb_2_1a0()->set_horn_request(horn_request(bytes, length));
    chassis->mutable_svb_2_1a0()->set_highbeam_request(highbeam_request(bytes, length));
    chassis->mutable_svb_2_1a0()->set_fog_request(fog_request(bytes, length));
    chassis->mutable_svb_2_1a0()->set_svb_doorrequest(svb_doorrequest(bytes, length));
    chassis->mutable_svb_2_1a0()->set_svb_hazardlightrequest(svb_hazardlightrequest(bytes, length));
    chassis->mutable_svb_2_1a0()->set_svb_rightturninglightrequest(svb_rightturninglightrequest(bytes, length));
    chassis->mutable_svb_2_1a0()->set_svb_leftturninglightrequest(svb_leftturninglightrequest(bytes, length));
    chassis->mutable_svb_2_1a0()->set_livecounter_svb_2(livecounter_svb_2(bytes, length));
    chassis->mutable_svb_2_1a0()->set_checksum_svb_2(checksum_svb_2(bytes, length));
}

void Svb21a0::UpdateData_Heartbeat(uint8_t* data) {
    // TODO(All) : you should add the heartbeat manually
 }

 void Svb21a0::UpdateData(uint8_t* data) {
    set_p_reserved_svb2_03(data, _reserved_svb2_03);
    set_p_reserved_svb2_02(data, _reserved_svb2_02);
    set_p_reserved_svb2(data, _reserved_svb2);
    set_p_request_turnoff_lowvoltage(data, _request_turnoff_lowvoltage);
    set_p_svb_epbrequest(data, _svb_epbrequest);
    set_p_position_light_request(data, _position_light_request);
    set_p_hu_speechrequest_setlefttemp(data, _hu_speechrequest_setlefttemp);
    set_p_hu_speechrequest_setleftblw(data, _hu_speechrequest_setleftblw);
    set_p_hu_speechrequest_lefttempadjsw(data, _hu_speechrequest_lefttempadjsw);
    set_p_hu_speechrequest_frsrec(data, _hu_speechrequest_frsrec);
    set_p_hu_speechrequest_frseatheatlevel(data, _hu_speechrequest_frseatheatlevel);
    set_p_hu_speechrequest_flseatheatlevel(data, _hu_speechrequest_flseatheatlevel);
    set_p_hu_speechrequest_blwleveladjsw(data, _hu_speechrequest_blwleveladjsw);
    set_p_hu_speechrequest_ac(data, _hu_speechrequest_ac);
    set_p_lowbeam_request(data, _lowbeam_request);
    set_p_horn_request(data, _horn_request);
    set_p_highbeam_request(data, _highbeam_request);
    set_p_fog_request(data, _fog_request);
    set_p_svb_doorrequest(data, _svb_doorrequest);
    set_p_svb_hazardlightrequest(data, _svb_hazardlightrequest);
    set_p_svb_rightturninglightrequest(data, _svb_rightturninglightrequest);
    set_p_svb_leftturninglightrequest(data, _svb_leftturninglightrequest);
    // set_p_checksum_svb_2(data, _checksum_svb_2);
    // UpdateMore(data);
  }

// use update_more() to add live_counter and checksum, remember to delete them in update_data()
void Svb21a0::UpdateMore(uint8_t* data) {
    _livecounter_svb_2 += 1;
    _livecounter_svb_2 %= 16;
    set_p_livecounter_svb_2(data, _livecounter_svb_2);
    _checksum_svb_2 = data[1] ^
                      data[2] ^
                      data[3] ^
                      data[4] ^
                      data[5] ^
                      data[6] ^
                      data[7];
    set_p_checksum_svb_2(data, _checksum_svb_2);
}

void Svb21a0::Reset() {
    _reserved_svb2_03 = 0;
    _reserved_svb2_02 = false;
    _reserved_svb2 = 0;
    _request_turnoff_lowvoltage = 0;
    _svb_epbrequest = 0;
    _position_light_request = 0;
    // _hu_speechrequest_setlefttemp = 0.0;
    // _hu_speechrequest_setleftblw = 0;
    // _hu_speechrequest_lefttempadjsw = 0;
    _hu_speechrequest_frsrec = 0;
    _hu_speechrequest_frseatheatlevel = 0;
    _hu_speechrequest_flseatheatlevel = 0;
    _hu_speechrequest_blwleveladjsw = 0;
    _hu_speechrequest_ac = 0;
    _lowbeam_request = 0;
    _horn_request = false;
    _highbeam_request = 0;
    _fog_request = 0;
    _svb_doorrequest = 0;
    // _svb_hazardlightrequest = 0;
    _svb_rightturninglightrequest = 0;
    _svb_leftturninglightrequest = 0;
    // _checksum_svb_2 = 0;
}

Svb21a0* Svb21a0::set_reserved_svb2_03(int reserved_svb2_03) {
    _reserved_svb2_03 = reserved_svb2_03;
    return this;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 2, 'name': 'Reserved_SVB2_03', 'is_signed_var': False, 'physical_range': '[0|3]', 'bit': 58, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_reserved_svb2_03(uint8_t* data, int reserved_svb2_03) {
    reserved_svb2_03 = ProtocolData::BoundedValue(0, 3, reserved_svb2_03);
    _reserved_svb2_03_x = reserved_svb2_03;
    Byte to_set(data + 7);
    to_set.set_value(_reserved_svb2_03_x, 2, 2);
}

Svb21a0* Svb21a0::set_reserved_svb2_02(bool reserved_svb2_02) {
    _reserved_svb2_02 = reserved_svb2_02;
    return this;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'Reserved_SVB2_02', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 39, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_reserved_svb2_02(uint8_t* data, bool reserved_svb2_02) {
    _reserved_svb2_02_x = reserved_svb2_02;
    Byte to_set(data + 4);
    to_set.set_value(_reserved_svb2_02_x, 7, 1);
}

Svb21a0* Svb21a0::set_reserved_svb2(int reserved_svb2) {
    _reserved_svb2 = reserved_svb2;
    return this;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 2, 'name': 'Reserved_SVB2', 'is_signed_var': False, 'physical_range': '[0|3]', 'bit': 8, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_reserved_svb2(uint8_t* data, int reserved_svb2) {
    reserved_svb2 = ProtocolData::BoundedValue(0, 3, reserved_svb2);
    _reserved_svb2_x = reserved_svb2;
    Byte to_set(data + 1);
    to_set.set_value(_reserved_svb2_x, 0, 2);
}

Svb21a0* Svb21a0::set_request_turnoff_lowvoltage(int request_turnoff_lowvoltage) {
    _request_turnoff_lowvoltage = request_turnoff_lowvoltage;
    return this;
}

// config detail: {'name': 'Request_turnoff_lowvoltage', 'offset': 0.0, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 56, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_request_turnoff_lowvoltage(uint8_t* data, int request_turnoff_lowvoltage) {
    request_turnoff_lowvoltage = ProtocolData::BoundedValue(0, 0, request_turnoff_lowvoltage);
    _request_turnoff_lowvoltage_x = request_turnoff_lowvoltage;
    Byte to_set(data + 7);
    to_set.set_value(_request_turnoff_lowvoltage_x, 0, 2);
}

Svb21a0* Svb21a0::set_svb_epbrequest(int svb_epbrequest) {
    _svb_epbrequest = svb_epbrequest;
    return this;
}

// config detail: {'description': 'SVB request to EPB.', 'enum': {0: 'SVB_EPBREQUEST_NO_REQUEST', 1: 'SVB_EPBREQUEST_RELEASING', 2: 'SVB_EPBREQUEST_LOCKING', 3: 'SVB_EPBREQUEST_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_EPBRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 54, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_svb_epbrequest(uint8_t* data, int svb_epbrequest) {
    _svb_epbrequest_x = svb_epbrequest;
    Byte to_set(data + 6);
    to_set.set_value(_svb_epbrequest_x, 6, 2);
}

Svb21a0* Svb21a0::set_position_light_request(int position_light_request) {
    _position_light_request = position_light_request;
    return this;
}

// config detail: {'name': 'Position_light_request', 'enum': {0: 'POSITION_LIGHT_REQUEST_NO_REQUEST', 1: 'POSITION_LIGHT_REQUEST_REQUEST_FRONT_POSITION_LIGHT', 2: 'POSITION_LIGHT_REQUEST_REQUEST_REAR_POSITION_LIGHT', 3: 'POSITION_LIGHT_REQUEST_REQUEST_BOTH_POSITION_LIGHT', 4: 'POSITION_LIGHT_REQUEST_RESERVED', 5: 'POSITION_LIGHT_REQUEST_RESERVED', 6: 'POSITION_LIGHT_REQUEST_RESERVED', 7: 'POSITION_LIGHT_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 3, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 32, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_position_light_request(uint8_t* data, int position_light_request) {
    _position_light_request_x = position_light_request;
    Byte to_set(data + 4);
    to_set.set_value(_position_light_request_x, 0, 3);
}

Svb21a0* Svb21a0::set_hu_speechrequest_setlefttemp(double hu_speechrequest_setlefttemp) {
    _hu_speechrequest_setlefttemp = hu_speechrequest_setlefttemp;
    return this;
}

// config detail: {'description': 'Speech request of setting left temperature.', 'offset': 15.0, 'precision': 0.5, 'len': 6, 'name': 'HU_SpeechRequest_SetLeftTemp', 'is_signed_var': False, 'physical_range': '[15|40]', 'bit': 48, 'type': 'double', 'order': 'intel', 'physical_unit': '\xa1\xe6'}
void Svb21a0::set_p_hu_speechrequest_setlefttemp(uint8_t* data, double hu_speechrequest_setlefttemp) {
    hu_speechrequest_setlefttemp = ProtocolData::BoundedValue(15.0, 40.0, hu_speechrequest_setlefttemp);
    _hu_speechrequest_setlefttemp_x = (hu_speechrequest_setlefttemp - 15.000000) / 0.500000;
    Byte to_set(data + 6);
    to_set.set_value(_hu_speechrequest_setlefttemp_x, 0, 6);
}

Svb21a0* Svb21a0::set_hu_speechrequest_setleftblw(int hu_speechrequest_setleftblw) {
    _hu_speechrequest_setleftblw = hu_speechrequest_setleftblw;
    return this;
}

// config detail: {'description': 'Speech request of setting left blower.', 'offset': 0.0, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_SetLeftBLW', 'is_signed_var': False, 'physical_range': '[0|7]', 'bit': 29, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_hu_speechrequest_setleftblw(uint8_t* data, int hu_speechrequest_setleftblw) {
    hu_speechrequest_setleftblw = ProtocolData::BoundedValue(0, 7, hu_speechrequest_setleftblw);
    _hu_speechrequest_setleftblw_x = hu_speechrequest_setleftblw;
    Byte to_set(data + 3);
    to_set.set_value(_hu_speechrequest_setleftblw_x, 5, 3);
}

Svb21a0* Svb21a0::set_hu_speechrequest_lefttempadjsw(int hu_speechrequest_lefttempadjsw) {
    _hu_speechrequest_lefttempadjsw = hu_speechrequest_lefttempadjsw;
    return this;
}

// config detail: {'description': 'Speech request of LeftTempAdjSwitch.', 'enum': {0: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_NO_REQUEST', 1: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_ADD_ONE_STEP', 2: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_ADD_TWO_STEPS', 3: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_RESERVED', 4: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_MINUS_ONE_STEP', 5: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_MINUS_TWO_STEPS', 6: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_RESERVED', 7: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_INVALID_VALUE'}, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_LeftTempAdjSw', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 26, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_hu_speechrequest_lefttempadjsw(uint8_t* data, int hu_speechrequest_lefttempadjsw) {
    _hu_speechrequest_lefttempadjsw_x = hu_speechrequest_lefttempadjsw;
    Byte to_set(data + 3);
    to_set.set_value(_hu_speechrequest_lefttempadjsw_x, 2, 3);
}

Svb21a0* Svb21a0::set_hu_speechrequest_frsrec(int hu_speechrequest_frsrec) {
    _hu_speechrequest_frsrec = hu_speechrequest_frsrec;
    return this;
}

// config detail: {'description': 'Speech request of FRSREC', 'enum': {0: 'HU_SPEECHREQUEST_FRSREC_NO_REQUEST', 1: 'HU_SPEECHREQUEST_FRSREC_FRS', 2: 'HU_SPEECHREQUEST_FRSREC_REC', 3: 'HU_SPEECHREQUEST_FRSREC_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'HU_SpeechRequest_FRSREC', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 37, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_hu_speechrequest_frsrec(uint8_t* data, int hu_speechrequest_frsrec) {
    _hu_speechrequest_frsrec_x = hu_speechrequest_frsrec;
    Byte to_set(data + 4);
    to_set.set_value(_hu_speechrequest_frsrec_x, 5, 2);
}

Svb21a0* Svb21a0::set_hu_speechrequest_frseatheatlevel(int hu_speechrequest_frseatheatlevel) {
    _hu_speechrequest_frseatheatlevel = hu_speechrequest_frseatheatlevel;
    return this;
}

// config detail: {'description': 'Speech request of FRseatHeatLevel', 'enum': {0: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_NO_REQUEST', 1: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_OFF', 2: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_1', 3: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_2', 4: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_3', 5: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_4', 6: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_5', 7: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_FRseatHeatLevel', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 45, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_hu_speechrequest_frseatheatlevel(uint8_t* data, int hu_speechrequest_frseatheatlevel) {
    _hu_speechrequest_frseatheatlevel_x = hu_speechrequest_frseatheatlevel;
    Byte to_set(data + 5);
    to_set.set_value(_hu_speechrequest_frseatheatlevel_x, 5, 3);
}

Svb21a0* Svb21a0::set_hu_speechrequest_flseatheatlevel(int hu_speechrequest_flseatheatlevel) {
    _hu_speechrequest_flseatheatlevel = hu_speechrequest_flseatheatlevel;
    return this;
}

// config detail: {'description': 'Speech request of FlseatHeatLevel', 'enum': {0: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_NO_REQUEST', 1: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_OFF', 2: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_1', 3: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_2', 4: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_3', 5: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_4', 6: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_5', 7: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_FLseatHeatLevel', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 42, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_hu_speechrequest_flseatheatlevel(uint8_t* data, int hu_speechrequest_flseatheatlevel) {
    _hu_speechrequest_flseatheatlevel_x = hu_speechrequest_flseatheatlevel;
    Byte to_set(data + 5);
    to_set.set_value(_hu_speechrequest_flseatheatlevel_x, 2, 3);
}

Svb21a0* Svb21a0::set_hu_speechrequest_blwleveladjsw(int hu_speechrequest_blwleveladjsw) {
    _hu_speechrequest_blwleveladjsw = hu_speechrequest_blwleveladjsw;
    return this;
}

// config detail: {'description': 'Speech request of BlwlevelAdjSwitch', 'enum': {0: 'HU_SPEECHREQUEST_BLWLEVELADJSW_NO_REQUEST', 1: 'HU_SPEECHREQUEST_BLWLEVELADJSW_UP', 2: 'HU_SPEECHREQUEST_BLWLEVELADJSW_DOWN', 3: 'HU_SPEECHREQUEST_BLWLEVELADJSW_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'HU_SpeechRequest_BlwlevelAdjSw', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 40, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_hu_speechrequest_blwleveladjsw(uint8_t* data, int hu_speechrequest_blwleveladjsw) {
    _hu_speechrequest_blwleveladjsw_x = hu_speechrequest_blwleveladjsw;
    Byte to_set(data + 5);
    to_set.set_value(_hu_speechrequest_blwleveladjsw_x, 0, 2);
}

Svb21a0* Svb21a0::set_hu_speechrequest_ac(int hu_speechrequest_ac) {
    _hu_speechrequest_ac = hu_speechrequest_ac;
    return this;
}

// config detail: {'description': 'Speech request of AC', 'enum': {0: 'HU_SPEECHREQUEST_AC_NO_REQUEST', 1: 'HU_SPEECHREQUEST_AC_ON', 2: 'HU_SPEECHREQUEST_AC_OFF', 3: 'HU_SPEECHREQUEST_AC_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'HU_SpeechRequest_AC', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 35, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_hu_speechrequest_ac(uint8_t* data, int hu_speechrequest_ac) {
    _hu_speechrequest_ac_x = hu_speechrequest_ac;
    Byte to_set(data + 4);
    to_set.set_value(_hu_speechrequest_ac_x, 3, 2);
}

Svb21a0* Svb21a0::set_lowbeam_request(int lowbeam_request) {
    _lowbeam_request = lowbeam_request;
    return this;
}

// config detail: {'name': 'LowBeam_request', 'enum': {0: 'LOWBEAM_REQUEST_NOREQUEST', 1: 'LOWBEAM_REQUEST_REQUEST', 2: 'LOWBEAM_REQUEST_RESERVED', 3: 'LOWBEAM_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_lowbeam_request(uint8_t* data, int lowbeam_request) {
    _lowbeam_request_x = lowbeam_request;
    Byte to_set(data + 2);
    to_set.set_value(_lowbeam_request_x, 4, 2);
}

Svb21a0* Svb21a0::set_horn_request(bool horn_request) {
    _horn_request = horn_request;
    return this;
}

// config detail: {'name': 'Horn_request', 'enum': {0: 'HORN_REQUEST_NO_REQUEST', 1: 'HORN_REQUEST_REQUEST'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 19, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_horn_request(uint8_t* data, bool horn_request) {
    _horn_request_x = horn_request;
    Byte to_set(data + 2);
    to_set.set_value(_horn_request_x, 3, 1);
}

Svb21a0* Svb21a0::set_highbeam_request(int highbeam_request) {
    _highbeam_request = highbeam_request;
    return this;
}

// config detail: {'name': 'HighBeam_request', 'enum': {0: 'HIGHBEAM_REQUEST_NOREQUEST', 1: 'HIGHBEAM_REQUEST_REQUEST', 2: 'HIGHBEAM_REQUEST_RESERVED', 3: 'HIGHBEAM_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 22, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_highbeam_request(uint8_t* data, int highbeam_request) {
    _highbeam_request_x = highbeam_request;
    Byte to_set(data + 2);
    to_set.set_value(_highbeam_request_x, 6, 2);
}

Svb21a0* Svb21a0::set_fog_request(int fog_request) {
    _fog_request = fog_request;
    return this;
}

// config detail: {'name': 'Fog_request', 'enum': {0: 'FOG_REQUEST_NO_REQUEST', 1: 'FOG_REQUEST_REQUEST_FRONT_FOG', 2: 'FOG_REQUEST_REQUEST_REAR_FOG', 3: 'FOG_REQUEST_REQUEST_FRONT_FOG_AND_REAR_FOG', 4: 'FOG_REQUEST_RESERVED', 5: 'FOG_REQUEST_RESERVED', 6: 'FOG_REQUEST_RESERVED', 7: 'FOG_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 3, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 16, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_fog_request(uint8_t* data, int fog_request) {
    _fog_request_x = fog_request;
    Byte to_set(data + 2);
    to_set.set_value(_fog_request_x, 0, 3);
}

Svb21a0* Svb21a0::set_livecounter_svb_2(int livecounter_svb_2) {
    _livecounter_svb_2 = livecounter_svb_2;
    return this;
}

// config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Livecounter_SVB_2', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
void Svb21a0::set_p_livecounter_svb_2(uint8_t* data, int livecounter_svb_2) {
    livecounter_svb_2 = ProtocolData::BoundedValue(0, 15, livecounter_svb_2);
    _livecounter_svb_2_x = livecounter_svb_2;
    Byte to_set(data + 7);
    to_set.set_value(_livecounter_svb_2_x, 4, 4);
}

Svb21a0* Svb21a0::set_svb_doorrequest(int svb_doorrequest) {
    _svb_doorrequest = svb_doorrequest;
    return this;
}

// config detail: {'description': 'Door request from SVB.', 'enum': {0: 'SVB_DOORREQUEST_NO_REQUEST', 1: 'SVB_DOORREQUEST_LOCK', 2: 'SVB_DOORREQUEST_RESERVED', 3: 'SVB_DOORREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_DoorRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_svb_doorrequest(uint8_t* data, int svb_doorrequest) {
    _svb_doorrequest_x = svb_doorrequest;
    Byte to_set(data + 3);
    to_set.set_value(_svb_doorrequest_x, 0, 2);
}

Svb21a0* Svb21a0::set_svb_hazardlightrequest(int svb_hazardlightrequest) {
    _svb_hazardlightrequest = svb_hazardlightrequest;
    return this;
}

// config detail: {'description': 'Hazard warning light request from SVB.', 'enum': {0: 'SVB_HAZARDLIGHTREQUEST_NO_REQUEST', 1: 'SVB_HAZARDLIGHTREQUEST_OFF', 2: 'SVB_HAZARDLIGHTREQUEST_ON', 3: 'SVB_HAZARDLIGHTREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_HazardLightRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 14, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_svb_hazardlightrequest(uint8_t* data, int svb_hazardlightrequest) {
    _svb_hazardlightrequest_x = svb_hazardlightrequest;
    Byte to_set(data + 1);
    to_set.set_value(_svb_hazardlightrequest_x, 6, 2);
}

Svb21a0* Svb21a0::set_svb_rightturninglightrequest(int svb_rightturninglightrequest) {
    _svb_rightturninglightrequest = svb_rightturninglightrequest;
    return this;
}

// config detail: {'description': 'Right turning light request from SVB.', 'enum': {0: 'SVB_RIGHTTURNINGLIGHTREQUEST_NO_REQUEST', 1: 'SVB_RIGHTTURNINGLIGHTREQUEST_OFF', 2: 'SVB_RIGHTTURNINGLIGHTREQUEST_ON', 3: 'SVB_RIGHTTURNINGLIGHTREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_RightTurningLightRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 12, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_svb_rightturninglightrequest(uint8_t* data, int svb_rightturninglightrequest) {
    _svb_rightturninglightrequest_x = svb_rightturninglightrequest;
    Byte to_set(data + 1);
    to_set.set_value(_svb_rightturninglightrequest_x, 4, 2);
}

Svb21a0* Svb21a0::set_svb_leftturninglightrequest(int svb_leftturninglightrequest) {
    _svb_leftturninglightrequest = svb_leftturninglightrequest;
    return this;
}

// config detail: {'description': 'Left turning light request from SVB.', 'enum': {0: 'SVB_LEFTTURNINGLIGHTREQUEST_NO_REQUEST', 1: 'SVB_LEFTTURNINGLIGHTREQUEST_OFF', 2: 'SVB_LEFTTURNINGLIGHTREQUEST_ON', 3: 'SVB_LEFTTURNINGLIGHTREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_LeftTurningLightRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 10, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_svb_leftturninglightrequest(uint8_t* data, int svb_leftturninglightrequest) {
    _svb_leftturninglightrequest_x = svb_leftturninglightrequest;
    Byte to_set(data + 1);
    to_set.set_value(_svb_leftturninglightrequest_x, 2, 2);
}

Svb21a0* Svb21a0::set_checksum_svb_2(int checksum_svb_2) {
    _checksum_svb_2 = checksum_svb_2;
    return this;
}

// config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_SVB_2', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
void Svb21a0::set_p_checksum_svb_2(uint8_t* data, int checksum_svb_2) {
    checksum_svb_2 = ProtocolData::BoundedValue(0, 255, checksum_svb_2);
    _checksum_svb_2_x = checksum_svb_2;
    Byte to_set(data + 0);
    to_set.set_value(_checksum_svb_2_x, 0, 8);
}

int Svb21a0::reserved_svb2_03(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(2, 2);
  
    int ret = x;
    return ret;
}

int Svb21a0::reserved_svb2_02(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(7, 1);
  
    int ret = x;
    return ret;
}

int Svb21a0::reserved_svb2(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(0, 2);
  
    int ret = x;
    return ret;
}

int Svb21a0::request_turnoff_lowvoltage(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(0, 2);
  
    int ret = x;
    return ret;
}

SVB_2_1a0::SVB_EPBRequestType Svb21a0::svb_epbrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(6, 2);
  
    SVB_2_1a0::SVB_EPBRequestType ret = static_cast<SVB_2_1a0::SVB_EPBRequestType>(x);
    return ret;
}

SVB_2_1a0::Position_light_requestType Svb21a0::position_light_request(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(0, 3);
  
    SVB_2_1a0::Position_light_requestType ret = static_cast<SVB_2_1a0::Position_light_requestType>(x);
    return ret;
}

double Svb21a0::hu_speechrequest_setlefttemp(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(0, 6);
  
    double ret = x * 0.500000 + 15.000000;
    return ret;
}

int Svb21a0::hu_speechrequest_setleftblw(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(5, 3);
  
    int ret = x;
    return ret;
}

SVB_2_1a0::HU_SpeechRequest_LeftTempAdjSwType Svb21a0::hu_speechrequest_lefttempadjsw(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(2, 3);
  
    SVB_2_1a0::HU_SpeechRequest_LeftTempAdjSwType ret = static_cast<SVB_2_1a0::HU_SpeechRequest_LeftTempAdjSwType>(x);
    return ret;
}

SVB_2_1a0::HU_SpeechRequest_FRSRECType Svb21a0::hu_speechrequest_frsrec(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(5, 2);
  
    SVB_2_1a0::HU_SpeechRequest_FRSRECType ret = static_cast<SVB_2_1a0::HU_SpeechRequest_FRSRECType>(x);
    return ret;
}

SVB_2_1a0::HU_SpeechRequest_FRseatHeatLevelType Svb21a0::hu_speechrequest_frseatheatlevel(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 5);
    int32_t x = t0.get_byte(5, 3);
  
    SVB_2_1a0::HU_SpeechRequest_FRseatHeatLevelType ret = static_cast<SVB_2_1a0::HU_SpeechRequest_FRseatHeatLevelType>(x);
    return ret;
}

SVB_2_1a0::HU_SpeechRequest_FLseatHeatLevelType Svb21a0::hu_speechrequest_flseatheatlevel(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 5);
    int32_t x = t0.get_byte(2, 3);
  
    SVB_2_1a0::HU_SpeechRequest_FLseatHeatLevelType ret = static_cast<SVB_2_1a0::HU_SpeechRequest_FLseatHeatLevelType>(x);
    return ret;
}

SVB_2_1a0::HU_SpeechRequest_BlwlevelAdjSwType Svb21a0::hu_speechrequest_blwleveladjsw(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 5);
    int32_t x = t0.get_byte(0, 2);
  
    SVB_2_1a0::HU_SpeechRequest_BlwlevelAdjSwType ret = static_cast<SVB_2_1a0::HU_SpeechRequest_BlwlevelAdjSwType>(x);
    return ret;
}

SVB_2_1a0::HU_SpeechRequest_ACType Svb21a0::hu_speechrequest_ac(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(3, 2);
  
    SVB_2_1a0::HU_SpeechRequest_ACType ret = static_cast<SVB_2_1a0::HU_SpeechRequest_ACType>(x);
    return ret;
}

SVB_2_1a0::LowBeam_requestType Svb21a0::lowbeam_request(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(4, 2);
  
    SVB_2_1a0::LowBeam_requestType ret = static_cast<SVB_2_1a0::LowBeam_requestType>(x);
    return ret;
}

SVB_2_1a0::Horn_requestType Svb21a0::horn_request(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(3, 1);
  
    SVB_2_1a0::Horn_requestType ret = static_cast<SVB_2_1a0::Horn_requestType>(x);
    return ret;
}

SVB_2_1a0::HighBeam_requestType Svb21a0::highbeam_request(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(6, 2);
  
    SVB_2_1a0::HighBeam_requestType ret = static_cast<SVB_2_1a0::HighBeam_requestType>(x);
    return ret;
}

SVB_2_1a0::Fog_requestType Svb21a0::fog_request(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(0, 3);
  
    SVB_2_1a0::Fog_requestType ret = static_cast<SVB_2_1a0::Fog_requestType>(x);
    return ret;
}

int Svb21a0::livecounter_svb_2(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(4, 4);
  
    int ret = x;
    return ret;
}

SVB_2_1a0::SVB_DoorRequestType Svb21a0::svb_doorrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(0, 2);
  
    SVB_2_1a0::SVB_DoorRequestType ret = static_cast<SVB_2_1a0::SVB_DoorRequestType>(x);
    return ret;
}

SVB_2_1a0::SVB_HazardLightRequestType Svb21a0::svb_hazardlightrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(6, 2);
  
    SVB_2_1a0::SVB_HazardLightRequestType ret = static_cast<SVB_2_1a0::SVB_HazardLightRequestType>(x);
    return ret;
}

SVB_2_1a0::SVB_RightTurningLightRequestType Svb21a0::svb_rightturninglightrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(4, 2);
  
    SVB_2_1a0::SVB_RightTurningLightRequestType ret = static_cast<SVB_2_1a0::SVB_RightTurningLightRequestType>(x);
    return ret;
}

SVB_2_1a0::SVB_LeftTurningLightRequestType Svb21a0::svb_leftturninglightrequest(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(2, 2);
  
    SVB_2_1a0::SVB_LeftTurningLightRequestType ret = static_cast<SVB_2_1a0::SVB_LeftTurningLightRequestType>(x);
    return ret;
}

int Svb21a0::checksum_svb_2(const std::uint8_t* bytes, const int32_t length) const {
    Byte t0(bytes + 0);
    int32_t x = t0.get_byte(0, 8);
  
    int ret = x;
    return ret;
}
}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
