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

class Svbnbsreq109 : public ::apollo::drivers::canbus::ProtocolData<
                ::apollo::canbus::Hongqi_nbooster> {

public:
  static const int32_t ID;

  Svbnbsreq109();

  uint32_t GetPeriod() const override;

  void Parse(const std::uint8_t* bytes, int32_t length,
                      Hongqi_nbooster* chassis) const override;

  void UpdateData_Heartbeat(uint8_t* data) override;

  void UpdateData(uint8_t* data) override;

  void UpdateMore(uint8_t* data);

  void Reset() override;

  // config detail: {'description': 'Requested recuperative brake torque', 'enum': {0: 'SVB_TARACCEPEDALPOSITION_REQST_NO_REQUEST', 1: 'SVB_TARACCEPEDALPOSITION_REQST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_TarAccePedalPosition_ReqSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 39, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svbnbsreq109* set_svb_taraccepedalposition_reqst(bool svb_taraccepedalposition_reqst);

  // config detail: {'description': 'Acceleration pedal position', 'offset': 0.0, 'precision': 0.4, 'len': 8, 'name': 'SVB_TarAccePedalPosition_Req', 'is_signed_var': False, 'physical_range': '[0|100]', 'bit': 40, 'type': 'double', 'order': 'intel', 'physical_unit': '%'}
    Svbnbsreq109* set_svb_taraccepedalposition_req(double svb_taraccepedalposition_req);

  // config detail: {'name': 'SVB_RecuBrakeTorqueFunc_Switch', 'enum': {0: 'SVB_RECUBRAKETORQUEFUNC_SWITCH_ON', 1: 'SVB_RECUBRAKETORQUEFUNC_SWITCH_OFF'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 38, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svbnbsreq109* set_svb_recubraketorquefunc_switch(bool svb_recubraketorquefunc_switch); 

  // config detail: {'description': 'This signal is target brake pedal position request for wheel brake.', 'enum': {16383: 'SVB_BRKPELPOS_DECREQUEST_INVALID'}, 'precision': 0.01, 'len': 14, 'name': 'SVB_BrkPelPos_DecRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|100]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': '%'}
    Svbnbsreq109* set_svb_brkpelpos_decrequest(double svb_brkpelpos_decrequest);

  // config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2,\xad15, 0, 1 \xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Livecounter_SVB_NBS', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    Svbnbsreq109* set_livecounter_svb_nbs(int livecounter_svb_nbs);

  // config detail: {'description': 'This signal indicates the status of deceleration request status of SVB.', 'enum': {0: 'SVB_DECMODE_REQUEST_NO_REQUEST', 1: 'SVB_DECMODE_REQUEST_BRKPELPOS_REQUEST', 2: 'SVB_DECMODE_REQUEST_TARDEC_REQUEST', 3: 'SVB_DECMODE_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_DecMode_Request', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 18, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svbnbsreq109* set_svb_decmode_request(int svb_decmode_request);

  // config detail: {'description': 'This signal is deceleration request for wheel brake.', 'enum': {1023: 'SVB_TARACC_DECREQUEST_INVALID'}, 'precision': 0.01, 'len': 10, 'name': 'SVB_TarACC_DecRequest', 'is_signed_var': False, 'offset': -10.0, 'physical_range': '[-10|0]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': 'm/s2'}
    Svbnbsreq109* set_svb_taracc_decrequest(double svb_taracc_decrequest);

  // config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_SVB_NBS', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svbnbsreq109* set_checksum_svb_nbs(int checksum_svb_nbs);

   // config detail: {'name': 'SVB_BrkReq_Level', 'enum': {0: 'SVB_BRKREQ_LEVEL_NORMAL_BRAKE', 1: 'SVB_BRKREQ_LEVEL_EMERGENCY_BRAKE', 2: 'SVB_BRKREQ_LEVEL_RESERVED', 3: 'SVB_BRKREQ_LEVEL_RESERVED'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svbnbsreq109* set_svb_brkreq_level(int svb_brkreq_level);
private:
  // config detail: {'description': 'Requested recuperative brake torque', 'enum': {0: 'SVB_TARACCEPEDALPOSITION_REQST_NO_REQUEST', 1: 'SVB_TARACCEPEDALPOSITION_REQST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_TarAccePedalPosition_ReqSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 39, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_taraccepedalposition_reqst(uint8_t* data, bool svb_taraccepedalposition_reqst);

  // config detail: {'description': 'Acceleration pedal position', 'offset': 0.0, 'precision': 0.4, 'len': 8, 'name': 'SVB_TarAccePedalPosition_Req', 'is_signed_var': False, 'physical_range': '[0|100]', 'bit': 40, 'type': 'double', 'order': 'intel', 'physical_unit': '%'}
    void set_p_svb_taraccepedalposition_req(uint8_t* data, double svb_taraccepedalposition_req);

  // config detail: {'name': 'SVB_RecuBrakeTorqueFunc_Switch', 'enum': {0: 'SVB_RECUBRAKETORQUEFUNC_SWITCH_ON', 1: 'SVB_RECUBRAKETORQUEFUNC_SWITCH_OFF'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 38, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_recubraketorquefunc_switch(uint8_t* data, bool svb_recubraketorquefunc_switch);

  // config detail: {'description': 'This signal is target brake pedal position request for wheel brake.', 'enum': {16383: 'SVB_BRKPELPOS_DECREQUEST_INVALID'}, 'precision': 0.01, 'len': 14, 'name': 'SVB_BrkPelPos_DecRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|100]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': '%'}
    void set_p_svb_brkpelpos_decrequest(uint8_t* data, double svb_brkpelpos_decrequest);

  // config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2,\xad15, 0, 1 \xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Livecounter_SVB_NBS', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    void set_p_livecounter_svb_nbs(uint8_t* data, int livecounter_svb_nbs);

  // config detail: {'description': 'This signal indicates the status of deceleration request status of SVB.', 'enum': {0: 'SVB_DECMODE_REQUEST_NO_REQUEST', 1: 'SVB_DECMODE_REQUEST_BRKPELPOS_REQUEST', 2: 'SVB_DECMODE_REQUEST_TARDEC_REQUEST', 3: 'SVB_DECMODE_REQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_DecMode_Request', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 18, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_decmode_request(uint8_t* data, int svb_decmode_request);

  // config detail: {'description': 'This signal is deceleration request for wheel brake.', 'enum': {1023: 'SVB_TARACC_DECREQUEST_INVALID'}, 'precision': 0.01, 'len': 10, 'name': 'SVB_TarACC_DecRequest', 'is_signed_var': False, 'offset': -10.0, 'physical_range': '[-10|0]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': 'm/s2'}
    void set_p_svb_taracc_decrequest(uint8_t* data, double svb_taracc_decrequest);

  // config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_SVB_NBS', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_checksum_svb_nbs(uint8_t* data, int checksum_svb_nbs);

  // config detail: {'name': 'SVB_BrkReq_Level', 'enum': {0: 'SVB_BRKREQ_LEVEL_NORMAL_BRAKE', 1: 'SVB_BRKREQ_LEVEL_EMERGENCY_BRAKE', 2: 'SVB_BRKREQ_LEVEL_RESERVED', 3: 'SVB_BRKREQ_LEVEL_RESERVED'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_brkreq_level(uint8_t* data, int svb_brkreq_level);

    SVB_NBS_REQ_109::SVB_TarAccePedalPosition_ReqStType svb_taraccepedalposition_reqst(const std::uint8_t* bytes, const int32_t length) const;

    double svb_taraccepedalposition_req(const std::uint8_t* bytes, const int32_t length) const;

    SVB_NBS_REQ_109::SVB_RecuBrakeTorqueFunc_SwitchType svb_recubraketorquefunc_switch(const std::uint8_t* bytes, const int32_t length) const;

    double svb_brkpelpos_decrequest(const std::uint8_t* bytes, const int32_t length) const;

    int livecounter_svb_nbs(const std::uint8_t* bytes, const int32_t length) const;

    SVB_NBS_REQ_109::SVB_DecMode_RequestType svb_decmode_request(const std::uint8_t* bytes, const int32_t length) const;

    double svb_taracc_decrequest(const std::uint8_t* bytes, const int32_t length) const;

    int checksum_svb_nbs(const std::uint8_t* bytes, const int32_t length) const;

    SVB_NBS_REQ_109::SVB_BrkReq_LevelType svb_brkreq_level(const std::uint8_t* bytes, const int32_t length) const;
private:
    int _svb_taraccepedalposition_reqst;
    int _svb_taraccepedalposition_reqst_x;
    double _svb_taraccepedalposition_req;
    int _svb_taraccepedalposition_req_x;    
    double _svb_brkpelpos_decrequest;
    int _svb_brkpelpos_decrequest_x;
    int _livecounter_svb_nbs;
    int _livecounter_svb_nbs_x;
    int _svb_decmode_request;
    int _svb_decmode_request_x;
    double _svb_taracc_decrequest;
    int _svb_taracc_decrequest_x;
    bool _svb_recubraketorquefunc_switch;
    int _svb_recubraketorquefunc_switch_x;
    int _checksum_svb_nbs;
    int _checksum_svb_nbs_x;
    int _svb_brkreq_level;
    int _svb_brkreq_level_x;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
