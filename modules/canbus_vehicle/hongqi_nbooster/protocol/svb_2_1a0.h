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

 #pragma once

 #include "modules/canbus_vehicle/hongqi_nbooster/proto/hongqi_nbooster.pb.h"
 
 #include "modules/drivers/canbus/can_comm/protocol_data.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

class Svb21a0 : public ::apollo::drivers::canbus::ProtocolData<
            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;

    Svb21a0();

    uint32_t GetPeriod() const override;

    void Parse(const std::uint8_t* bytes, int32_t length,
        Hongqi_nbooster* chassis) const override;

    void UpdateData_Heartbeat(uint8_t* data) override;

    void UpdateData(uint8_t* data) override;

    void UpdateMore(uint8_t* data);
    
    void Reset() override;

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 2, 'name': 'Reserved_SVB2_03', 'is_signed_var': False, 'physical_range': '[0|3]', 'bit': 58, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_reserved_svb2_03(int reserved_svb2_03);

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'Reserved_SVB2_02', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 39, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_reserved_svb2_02(bool reserved_svb2_02);

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 2, 'name': 'Reserved_SVB2', 'is_signed_var': False, 'physical_range': '[0|3]', 'bit': 8, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_reserved_svb2(int reserved_svb2);

    // config detail: {'name': 'Request_turnoff_lowvoltage', 'offset': 0.0, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 56, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_request_turnoff_lowvoltage(int request_turnoff_lowvoltage);

    // config detail: {'description': 'SVB request to EPB.', 'enum': {0: 'SVB_EPBREQUEST_NO_REQUEST', 1: 'SVB_EPBREQUEST_RELEASING', 2: 'SVB_EPBREQUEST_LOCKING', 3: 'SVB_EPBREQUEST_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_EPBRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 54, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_svb_epbrequest(int svb_epbrequest);

    // config detail: {'name': 'Position_light_request', 'enum': {0: 'POSITION_LIGHT_REQUEST_NO_REQUEST', 1: 'POSITION_LIGHT_REQUEST_REQUEST_FRONT_POSITION_LIGHT', 2: 'POSITION_LIGHT_REQUEST_REQUEST_REAR_POSITION_LIGHT', 3: 'POSITION_LIGHT_REQUEST_REQUEST_BOTH_POSITION_LIGHT', 4: 'POSITION_LIGHT_REQUEST_RESERVED', 5: 'POSITION_LIGHT_REQUEST_RESERVED', 6: 'POSITION_LIGHT_REQUEST_RESERVED', 7: 'POSITION_LIGHT_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 3, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 32, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_position_light_request(int position_light_request);

    // config detail: {'description': 'Speech request of setting left temperature.', 'offset': 15.0, 'precision': 0.5, 'len': 6, 'name': 'HU_SpeechRequest_SetLeftTemp', 'is_signed_var': False, 'physical_range': '[15|40]', 'bit': 48, 'type': 'double', 'order': 'intel', 'physical_unit': '\xa1\xe6'}
    Svb21a0* set_hu_speechrequest_setlefttemp(double hu_speechrequest_setlefttemp);

    // config detail: {'description': 'Speech request of setting left blower.', 'offset': 0.0, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_SetLeftBLW', 'is_signed_var': False, 'physical_range': '[0|7]', 'bit': 29, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_hu_speechrequest_setleftblw(int hu_speechrequest_setleftblw);

    // config detail: {'description': 'Speech request of LeftTempAdjSwitch.', 'enum': {0: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_NO_REQUEST', 1: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_ADD_ONE_STEP', 2: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_ADD_TWO_STEPS', 3: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_RESERVED', 4: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_MINUS_ONE_STEP', 5: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_MINUS_TWO_STEPS', 6: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_RESERVED', 7: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_INVALID_VALUE'}, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_LeftTempAdjSw', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 26, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_hu_speechrequest_lefttempadjsw(int hu_speechrequest_lefttempadjsw);

    // config detail: {'description': 'Speech request of FRSREC', 'enum': {0: 'HU_SPEECHREQUEST_FRSREC_NO_REQUEST', 1: 'HU_SPEECHREQUEST_FRSREC_FRS', 2: 'HU_SPEECHREQUEST_FRSREC_REC', 3: 'HU_SPEECHREQUEST_FRSREC_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'HU_SpeechRequest_FRSREC', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 37, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_hu_speechrequest_frsrec(int hu_speechrequest_frsrec);

    // config detail: {'description': 'Speech request of FRseatHeatLevel', 'enum': {0: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_NO_REQUEST', 1: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_OFF', 2: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_1', 3: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_2', 4: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_3', 5: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_4', 6: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_5', 7: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_FRseatHeatLevel', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 45, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_hu_speechrequest_frseatheatlevel(int hu_speechrequest_frseatheatlevel);

    // config detail: {'description': 'Speech request of FlseatHeatLevel', 'enum': {0: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_NO_REQUEST', 1: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_OFF', 2: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_1', 3: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_2', 4: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_3', 5: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_4', 6: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_5', 7: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_FLseatHeatLevel', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 42, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_hu_speechrequest_flseatheatlevel(int hu_speechrequest_flseatheatlevel);

    // config detail: {'description': 'Speech request of BlwlevelAdjSwitch', 'enum': {0: 'HU_SPEECHREQUEST_BLWLEVELADJSW_NO_REQUEST', 1: 'HU_SPEECHREQUEST_BLWLEVELADJSW_UP', 2: 'HU_SPEECHREQUEST_BLWLEVELADJSW_DOWN', 3: 'HU_SPEECHREQUEST_BLWLEVELADJSW_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'HU_SpeechRequest_BlwlevelAdjSw', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 40, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_hu_speechrequest_blwleveladjsw(int hu_speechrequest_blwleveladjsw);

    // config detail: {'description': 'Speech request of AC', 'enum': {0: 'HU_SPEECHREQUEST_AC_NO_REQUEST', 1: 'HU_SPEECHREQUEST_AC_ON', 2: 'HU_SPEECHREQUEST_AC_OFF', 3: 'HU_SPEECHREQUEST_AC_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'HU_SpeechRequest_AC', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 35, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_hu_speechrequest_ac(int hu_speechrequest_ac);

    // config detail: {'name': 'LowBeam_request', 'enum': {0: 'LOWBEAM_REQUEST_NOREQUEST', 1: 'LOWBEAM_REQUEST_REQUEST', 2: 'LOWBEAM_REQUEST_RESERVED', 3: 'LOWBEAM_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_lowbeam_request(int lowbeam_request);

    // config detail: {'name': 'Horn_request', 'enum': {0: 'HORN_REQUEST_NO_REQUEST', 1: 'HORN_REQUEST_REQUEST'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 19, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_horn_request(bool horn_request);

    // config detail: {'name': 'HighBeam_request', 'enum': {0: 'HIGHBEAM_REQUEST_NOREQUEST', 1: 'HIGHBEAM_REQUEST_REQUEST', 2: 'HIGHBEAM_REQUEST_RESERVED', 3: 'HIGHBEAM_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 22, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_highbeam_request(int highbeam_request);

    // config detail: {'name': 'Fog_request', 'enum': {0: 'FOG_REQUEST_NO_REQUEST', 1: 'FOG_REQUEST_REQUEST_FRONT_FOG', 2: 'FOG_REQUEST_REQUEST_REAR_FOG', 3: 'FOG_REQUEST_REQUEST_FRONT_FOG_AND_REAR_FOG', 4: 'FOG_REQUEST_RESERVED', 5: 'FOG_REQUEST_RESERVED', 6: 'FOG_REQUEST_RESERVED', 7: 'FOG_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 3, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 16, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_fog_request(int fog_request);

    // config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Livecounter_SVB_2', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    Svb21a0* set_livecounter_svb_2(int livecounter_svb_2);

    // config detail: {'description': 'Door request from SVB.', 'enum': {0: 'SVB_DOORREQUEST_NO_REQUEST', 1: 'SVB_DOORREQUEST_LOCK', 2: 'SVB_DOORREQUEST_RESERVED', 3: 'SVB_DOORREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_DoorRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_svb_doorrequest(int svb_doorrequest);

    // config detail: {'description': 'Hazard warning light request from SVB.', 'enum': {0: 'SVB_HAZARDLIGHTREQUEST_NO_REQUEST', 1: 'SVB_HAZARDLIGHTREQUEST_OFF', 2: 'SVB_HAZARDLIGHTREQUEST_ON', 3: 'SVB_HAZARDLIGHTREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_HazardLightRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 14, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_svb_hazardlightrequest(int svb_hazardlightrequest);

    // config detail: {'description': 'Right turning light request from SVB.', 'enum': {0: 'SVB_RIGHTTURNINGLIGHTREQUEST_NO_REQUEST', 1: 'SVB_RIGHTTURNINGLIGHTREQUEST_OFF', 2: 'SVB_RIGHTTURNINGLIGHTREQUEST_ON', 3: 'SVB_RIGHTTURNINGLIGHTREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_RightTurningLightRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 12, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_svb_rightturninglightrequest(int svb_rightturninglightrequest);

    // config detail: {'description': 'Left turning light request from SVB.', 'enum': {0: 'SVB_LEFTTURNINGLIGHTREQUEST_NO_REQUEST', 1: 'SVB_LEFTTURNINGLIGHTREQUEST_OFF', 2: 'SVB_LEFTTURNINGLIGHTREQUEST_ON', 3: 'SVB_LEFTTURNINGLIGHTREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_LeftTurningLightRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 10, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_svb_leftturninglightrequest(int svb_leftturninglightrequest);

    // config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_SVB_2', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svb21a0* set_checksum_svb_2(int checksum_svb_2);
private:

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 2, 'name': 'Reserved_SVB2_03', 'is_signed_var': False, 'physical_range': '[0|3]', 'bit': 58, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_reserved_svb2_03(uint8_t* data, int reserved_svb2_03);

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'Reserved_SVB2_02', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 39, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
    void set_p_reserved_svb2_02(uint8_t* data, bool reserved_svb2_02);

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 2, 'name': 'Reserved_SVB2', 'is_signed_var': False, 'physical_range': '[0|3]', 'bit': 8, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_reserved_svb2(uint8_t* data, int reserved_svb2);

    // config detail: {'name': 'Request_turnoff_lowvoltage', 'offset': 0.0, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 56, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_request_turnoff_lowvoltage(uint8_t* data, int request_turnoff_lowvoltage);

    // config detail: {'description': 'SVB request to EPB.', 'enum': {0: 'SVB_EPBREQUEST_NO_REQUEST', 1: 'SVB_EPBREQUEST_RELEASING', 2: 'SVB_EPBREQUEST_LOCKING', 3: 'SVB_EPBREQUEST_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_EPBRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 54, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_epbrequest(uint8_t* data, int svb_epbrequest);

    // config detail: {'name': 'Position_light_request', 'enum': {0: 'POSITION_LIGHT_REQUEST_NO_REQUEST', 1: 'POSITION_LIGHT_REQUEST_REQUEST_FRONT_POSITION_LIGHT', 2: 'POSITION_LIGHT_REQUEST_REQUEST_REAR_POSITION_LIGHT', 3: 'POSITION_LIGHT_REQUEST_REQUEST_BOTH_POSITION_LIGHT', 4: 'POSITION_LIGHT_REQUEST_RESERVED', 5: 'POSITION_LIGHT_REQUEST_RESERVED', 6: 'POSITION_LIGHT_REQUEST_RESERVED', 7: 'POSITION_LIGHT_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 3, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 32, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_position_light_request(uint8_t* data, int position_light_request);

    // config detail: {'description': 'Speech request of setting left temperature.', 'offset': 15.0, 'precision': 0.5, 'len': 6, 'name': 'HU_SpeechRequest_SetLeftTemp', 'is_signed_var': False, 'physical_range': '[15|40]', 'bit': 48, 'type': 'double', 'order': 'intel', 'physical_unit': '\xa1\xe6'}
    void set_p_hu_speechrequest_setlefttemp(uint8_t* data, double hu_speechrequest_setlefttemp);

    // config detail: {'description': 'Speech request of setting left blower.', 'offset': 0.0, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_SetLeftBLW', 'is_signed_var': False, 'physical_range': '[0|7]', 'bit': 29, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_hu_speechrequest_setleftblw(uint8_t* data, int hu_speechrequest_setleftblw);

    // config detail: {'description': 'Speech request of LeftTempAdjSwitch.', 'enum': {0: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_NO_REQUEST', 1: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_ADD_ONE_STEP', 2: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_ADD_TWO_STEPS', 3: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_RESERVED', 4: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_MINUS_ONE_STEP', 5: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_MINUS_TWO_STEPS', 6: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_RESERVED', 7: 'HU_SPEECHREQUEST_LEFTTEMPADJSW_INVALID_VALUE'}, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_LeftTempAdjSw', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 26, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_hu_speechrequest_lefttempadjsw(uint8_t* data, int hu_speechrequest_lefttempadjsw);

    // config detail: {'description': 'Speech request of FRSREC', 'enum': {0: 'HU_SPEECHREQUEST_FRSREC_NO_REQUEST', 1: 'HU_SPEECHREQUEST_FRSREC_FRS', 2: 'HU_SPEECHREQUEST_FRSREC_REC', 3: 'HU_SPEECHREQUEST_FRSREC_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'HU_SpeechRequest_FRSREC', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 37, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_hu_speechrequest_frsrec(uint8_t* data, int hu_speechrequest_frsrec);

    // config detail: {'description': 'Speech request of FRseatHeatLevel', 'enum': {0: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_NO_REQUEST', 1: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_OFF', 2: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_1', 3: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_2', 4: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_3', 5: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_4', 6: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_LEVEL_5', 7: 'HU_SPEECHREQUEST_FRSEATHEATLEVEL_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_FRseatHeatLevel', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 45, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_hu_speechrequest_frseatheatlevel(uint8_t* data, int hu_speechrequest_frseatheatlevel);

    // config detail: {'description': 'Speech request of FlseatHeatLevel', 'enum': {0: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_NO_REQUEST', 1: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_OFF', 2: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_1', 3: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_2', 4: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_3', 5: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_4', 6: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_LEVEL_5', 7: 'HU_SPEECHREQUEST_FLSEATHEATLEVEL_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'HU_SpeechRequest_FLseatHeatLevel', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 42, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_hu_speechrequest_flseatheatlevel(uint8_t* data, int hu_speechrequest_flseatheatlevel);

    // config detail: {'description': 'Speech request of BlwlevelAdjSwitch', 'enum': {0: 'HU_SPEECHREQUEST_BLWLEVELADJSW_NO_REQUEST', 1: 'HU_SPEECHREQUEST_BLWLEVELADJSW_UP', 2: 'HU_SPEECHREQUEST_BLWLEVELADJSW_DOWN', 3: 'HU_SPEECHREQUEST_BLWLEVELADJSW_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'HU_SpeechRequest_BlwlevelAdjSw', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 40, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_hu_speechrequest_blwleveladjsw(uint8_t* data, int hu_speechrequest_blwleveladjsw);

    // config detail: {'description': 'Speech request of AC', 'enum': {0: 'HU_SPEECHREQUEST_AC_NO_REQUEST', 1: 'HU_SPEECHREQUEST_AC_ON', 2: 'HU_SPEECHREQUEST_AC_OFF', 3: 'HU_SPEECHREQUEST_AC_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'HU_SpeechRequest_AC', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 35, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_hu_speechrequest_ac(uint8_t* data, int hu_speechrequest_ac);

    // config detail: {'name': 'LowBeam_request', 'enum': {0: 'LOWBEAM_REQUEST_NOREQUEST', 1: 'LOWBEAM_REQUEST_REQUEST', 2: 'LOWBEAM_REQUEST_RESERVED', 3: 'LOWBEAM_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_lowbeam_request(uint8_t* data, int lowbeam_request);

    // config detail: {'name': 'Horn_request', 'enum': {0: 'HORN_REQUEST_NO_REQUEST', 1: 'HORN_REQUEST_REQUEST'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 19, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_horn_request(uint8_t* data, bool horn_request);

    // config detail: {'name': 'HighBeam_request', 'enum': {0: 'HIGHBEAM_REQUEST_NOREQUEST', 1: 'HIGHBEAM_REQUEST_REQUEST', 2: 'HIGHBEAM_REQUEST_RESERVED', 3: 'HIGHBEAM_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 22, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_highbeam_request(uint8_t* data, int highbeam_request);

    // config detail: {'name': 'Fog_request', 'enum': {0: 'FOG_REQUEST_NO_REQUEST', 1: 'FOG_REQUEST_REQUEST_FRONT_FOG', 2: 'FOG_REQUEST_REQUEST_REAR_FOG', 3: 'FOG_REQUEST_REQUEST_FRONT_FOG_AND_REAR_FOG', 4: 'FOG_REQUEST_RESERVED', 5: 'FOG_REQUEST_RESERVED', 6: 'FOG_REQUEST_RESERVED', 7: 'FOG_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 3, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|0]', 'bit': 16, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_fog_request(uint8_t* data, int fog_request);

    // config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Livecounter_SVB_2', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    void set_p_livecounter_svb_2(uint8_t* data, int livecounter_svb_2);

    // config detail: {'description': 'Door request from SVB.', 'enum': {0: 'SVB_DOORREQUEST_NO_REQUEST', 1: 'SVB_DOORREQUEST_LOCK', 2: 'SVB_DOORREQUEST_RESERVED', 3: 'SVB_DOORREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_DoorRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_doorrequest(uint8_t* data, int svb_doorrequest);

    // config detail: {'description': 'Hazard warning light request from SVB.', 'enum': {0: 'SVB_HAZARDLIGHTREQUEST_NO_REQUEST', 1: 'SVB_HAZARDLIGHTREQUEST_OFF', 2: 'SVB_HAZARDLIGHTREQUEST_ON', 3: 'SVB_HAZARDLIGHTREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_HazardLightRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 14, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_hazardlightrequest(uint8_t* data, int svb_hazardlightrequest);

    // config detail: {'description': 'Right turning light request from SVB.', 'enum': {0: 'SVB_RIGHTTURNINGLIGHTREQUEST_NO_REQUEST', 1: 'SVB_RIGHTTURNINGLIGHTREQUEST_OFF', 2: 'SVB_RIGHTTURNINGLIGHTREQUEST_ON', 3: 'SVB_RIGHTTURNINGLIGHTREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_RightTurningLightRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 12, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_rightturninglightrequest(uint8_t* data, int svb_rightturninglightrequest);

    // config detail: {'description': 'Left turning light request from SVB.', 'enum': {0: 'SVB_LEFTTURNINGLIGHTREQUEST_NO_REQUEST', 1: 'SVB_LEFTTURNINGLIGHTREQUEST_OFF', 2: 'SVB_LEFTTURNINGLIGHTREQUEST_ON', 3: 'SVB_LEFTTURNINGLIGHTREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_LeftTurningLightRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 10, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_leftturninglightrequest(uint8_t* data, int svb_leftturninglightrequest);

    // config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_SVB_2', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_checksum_svb_2(uint8_t* data, int checksum_svb_2);

    int reserved_svb2_03(const std::uint8_t* bytes, const int32_t length) const;

    int reserved_svb2_02(const std::uint8_t* bytes, const int32_t length) const;

    int reserved_svb2(const std::uint8_t* bytes, const int32_t length) const;

    int request_turnoff_lowvoltage(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::SVB_EPBRequestType svb_epbrequest(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::Position_light_requestType position_light_request(const std::uint8_t* bytes, const int32_t length) const;

    double hu_speechrequest_setlefttemp(const std::uint8_t* bytes, const int32_t length) const;

    int hu_speechrequest_setleftblw(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::HU_SpeechRequest_LeftTempAdjSwType hu_speechrequest_lefttempadjsw(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::HU_SpeechRequest_FRSRECType hu_speechrequest_frsrec(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::HU_SpeechRequest_FRseatHeatLevelType hu_speechrequest_frseatheatlevel(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::HU_SpeechRequest_FLseatHeatLevelType hu_speechrequest_flseatheatlevel(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::HU_SpeechRequest_BlwlevelAdjSwType hu_speechrequest_blwleveladjsw(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::HU_SpeechRequest_ACType hu_speechrequest_ac(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::LowBeam_requestType lowbeam_request(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::Horn_requestType horn_request(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::HighBeam_requestType highbeam_request(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::Fog_requestType fog_request(const std::uint8_t* bytes, const int32_t length) const;

    int livecounter_svb_2(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::SVB_DoorRequestType svb_doorrequest(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::SVB_HazardLightRequestType svb_hazardlightrequest(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::SVB_RightTurningLightRequestType svb_rightturninglightrequest(const std::uint8_t* bytes, const int32_t length) const;

    SVB_2_1a0::SVB_LeftTurningLightRequestType svb_leftturninglightrequest(const std::uint8_t* bytes, const int32_t length) const;

    int checksum_svb_2(const std::uint8_t* bytes, const int32_t length) const;

private:
    int _reserved_svb2_03 = 0;
    int _reserved_svb2_03_x = 0;
    bool _reserved_svb2_02 = false;
    int _reserved_svb2_02_x = 0;
    int _reserved_svb2 = 0;
    int _reserved_svb2_x = 0;
    int _request_turnoff_lowvoltage = 0;
    int _request_turnoff_lowvoltage_x = 0;
    int _svb_epbrequest = 0;
    int _svb_epbrequest_x = 0;
    int _position_light_request = 0;
    int _position_light_request_x = 0;
    double _hu_speechrequest_setlefttemp = 0.0;
    int _hu_speechrequest_setlefttemp_x = 0;
    int _hu_speechrequest_setleftblw = 0;
    int _hu_speechrequest_setleftblw_x = 0;
    int _hu_speechrequest_lefttempadjsw = 0;
    int _hu_speechrequest_lefttempadjsw_x = 0;
    int _hu_speechrequest_frsrec = 0;
    int _hu_speechrequest_frsrec_x = 0;
    int _hu_speechrequest_frseatheatlevel = 0;
    int _hu_speechrequest_frseatheatlevel_x = 0;
    int _hu_speechrequest_flseatheatlevel = 0;
    int _hu_speechrequest_flseatheatlevel_x = 0;
    int _hu_speechrequest_blwleveladjsw = 0;
    int _hu_speechrequest_blwleveladjsw_x = 0;
    int _hu_speechrequest_ac = 0;
    int _hu_speechrequest_ac_x = 0;
    int _lowbeam_request = 0;
    int _lowbeam_request_x = 0;
    bool _horn_request = false;
    int _horn_request_x = 0;
    int _highbeam_request = 0;
    int _highbeam_request_x = 0;
    int _fog_request = 0;
    int _fog_request_x = 0;
    int _livecounter_svb_2 = 0;
    int _livecounter_svb_2_x = 0;
    int _svb_doorrequest = 0;
    int _svb_doorrequest_x = 0;
    int _svb_hazardlightrequest = 0;
    int _svb_hazardlightrequest_x = 0;
    int _svb_rightturninglightrequest = 0;
    int _svb_rightturninglightrequest_x = 0;
    int _svb_leftturninglightrequest = 0;
    int _svb_leftturninglightrequest_x = 0;
    int _checksum_svb_2 = 0;
    int _checksum_svb_2_x = 0;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
