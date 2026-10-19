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

class Svb1108 : public ::apollo::drivers::canbus::ProtocolData<
            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;

    Svb1108();

    uint32_t GetPeriod() const override;

    void Parse(const std::uint8_t* bytes, int32_t length,
        Hongqi_nbooster* chassis) const override;

    void UpdateData_Heartbeat(uint8_t* data) override;

    void UpdateData(uint8_t* data) override;

    void UpdateMore(uint8_t* data);
    
    void Reset() override;

    // config detail: {'description': 'ndicate the Max speed limit', 'enum': {0: 'SVB_MAXSPEEDLIMIT_LIMIT', 1: 'SVB_MAXSPEEDLIMIT_NO_LIMIT'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_MaxSpeedLimit', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 54, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_svb_maxspeedlimit(int svb_maxspeedlimit);

    // config detail: {'description': 'Steering wheel speed request from SVB.The maximum speed shall bedetermined in calibrating phase.', 'enum': {511: 'SVB_STEERINGSPEEDREQUEST_INVALID'}, 'precision': 1.0, 'len': 9, 'name': 'SVB_SteeringSpeedRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|500]', 'bit': 39, 'type': 'enum', 'order': 'intel', 'physical_unit': 'deg/s'}
    Svb1108* set_svb_steeringspeedrequest(double svb_steeringspeedrequest);

    // config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Livecounter_SVB_1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    Svb1108* set_livecounter_svb_1(int livecounter_svb_1);

    // config detail: {'description': 'Gear request from SVB. SVB shall keep sending requested gear signalvalue in automatically mode.', 'enum': {0: 'SVB_GEARREQUEST_NO_REQUEST', 1: 'SVB_GEARREQUEST_P', 2: 'SVB_GEARREQUEST_R', 3: 'SVB_GEARREQUEST_N', 4: 'SVB_GEARREQUEST_D', 5: 'SVB_GEARREQUEST_RESERVED', 6: 'SVB_GEARREQUEST_RESERVED', 7: 'SVB_GEARREQUEST_INVALID_VALUE'}, 'precision': 1.0, 'len': 3, 'name': 'SVB_GearRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 57, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_svb_gearrequest(int svb_gearrequest);

    // config detail: {'description': 'This signal is the status of acceleration request for driving.', 'enum': {0: 'SVB_ACCELERATIONREQUESTST_NO_REQUEST', 1: 'SVB_ACCELERATIONREQUESTST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_AccelerationRequestSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 56, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_svb_accelerationrequestst(bool svb_accelerationrequestst);

    // config detail: {'description': 'This signal indicates the taking over request from SVB.When the requestfrom SVB could not be actuated correctly(including message lost, receivedsignal value unexpected) , this signal shall be set to ask driver taking over.', 'enum': {0: 'SVB_TAKEOVERREQUEST_NO_REQUEST', 1: 'SVB_TAKEOVERREQUEST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_TakeOverRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 55, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_svb_takeoverrequest(bool svb_takeoverrequest);

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'Reserved_SVB1', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 54, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_reserved_svb1(bool reserved_svb1);

    // config detail: {'description': 'Acceleration request from SVB.', 'enum': {63: 'SVB_ACCELERATIONREQUEST_INVALID'}, 'precision': 0.1, 'len': 6, 'name': 'SVB_AccelerationRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|6.3]', 'bit': 48, 'type': 'enum', 'order': 'intel', 'physical_unit': 'm/s2'}
    Svb1108* set_svb_accelerationrequest(double svb_accelerationrequest);

    // config detail: {'description': 'This signal indicates the stand-still request. This signal is used for comfortbraking when vehicle speed is lower than 2km/h(calibrated value).', 'enum': {0: 'SVB_STANDSTILLREQUEST_NO_REQUEST', 1: 'SVB_STANDSTILLREQUEST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_StandStillRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 38, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_svb_standstillrequest(bool svb_standstillrequest);

    // config detail: {'description': 'Steering wheel speed sign request from SVB.', 'enum': {0: 'SVB_STEERINGSPEEDSIGNREQUEST_NO_REQUEST', 1: 'SVB_STEERINGSPEEDSIGNREQUEST_REQUEST', 2: 'SVB_STEERINGSPEEDSIGNREQUEST_RESERVED', 3: 'SVB_STEERINGSPEEDSIGNREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_SteeringSpeedSignRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 36, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_svb_steeringspeedsignrequest(int svb_steeringspeedsignrequest);

    // config detail: {'description': 'Steering wheel angle sign request from SVB.', 'enum': {0: 'SVB_STEERINGANGLESIGNREQUEST_NO_REQUEST', 1: 'SVB_STEERINGANGLESIGNREQUEST_LEFT_POSITIVE', 2: 'SVB_STEERINGANGLESIGNREQUEST_RIGHT_NEGATIVE', 3: 'SVB_STEERINGANGLESIGNREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_SteeringAngleSignRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 34, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_svb_steeringanglesignrequest(int svb_steeringanglesignrequest);

    // config detail: {'description': 'Steering wheel angle request from SVB. The maximum angle shall bedetermined in calibrating phase.', 'enum': {1023: 'SVB_STEERINGANGLEREQUEST_INVALID'}, 'precision': 1.0, 'len': 10, 'name': 'SVB_SteeringAngleRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1022]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': 'deg'}
    Svb1108* set_svb_steeringanglerequest(double svb_steeringanglerequest);

    // config detail: {'description': 'This signal is the status of steering wheel request for driving.', 'enum': {0: 'SVB_STEERINGREQUESTST_NO_REQUEST', 1: 'SVB_STEERINGREQUESTST_REQUEST', 2: 'SVB_STEERINGREQUESTST_RESERVED', 3: 'SVB_STEERINGREQUESTST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_SteeringRequestSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 22, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_svb_steeringrequestst(int svb_steeringrequestst);

    // config detail: {'description': 'This signal indicates the status of SVB system. When SVB detected its ownfault, this signal shall be set.', 'enum': {0: 'SVB_FAILURESTATUS_NO_ERROR', 1: 'SVB_FAILURESTATUS_ERROR', 2: 'SVB_FAILURESTATUS_RESERVED', 3: 'SVB_FAILURESTATUS_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_FailureStatus', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_svb_failurestatus(int svb_failurestatus);

    // config detail: {'description': 'This signal indicates the status of deceleration request status of SVB.', 'enum': {0: 'SVB_DECELERATIONREQUESTST_NO_REQUEST', 1: 'SVB_DECELERATIONREQUESTST_ACC_REQUEST', 2: 'SVB_DECELERATIONREQUESTST_AEB_REQUEST', 3: 'SVB_DECELERATIONREQUESTST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_DecelerationRequestSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 18, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_svb_decelerationrequestst(int svb_decelerationrequestst);

    // config detail: {'description': 'This signal is deceleration request for wheel brake.', 'enum': {1023: 'SVB_DECELERATIONREQUEST_INVALID'}, 'precision': 0.01, 'len': 10, 'name': 'SVB_DecelerationRequest', 'is_signed_var': False, 'offset': -10.0, 'physical_range': '[-10|0]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': 'm/s2'}
    Svb1108* set_svb_decelerationrequest(double svb_decelerationrequest);

    // config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_SVB_1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svb1108* set_checksum_svb_1(int checksum_svb_1);

private:

    // config detail: {'description': 'ndicate the Max speed limit', 'enum': {0: 'SVB_MAXSPEEDLIMIT_LIMIT', 1: 'SVB_MAXSPEEDLIMIT_NO_LIMIT'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_MaxSpeedLimit', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 54, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_maxspeedlimit(uint8_t* data, int svb_maxspeedlimit);

    // config detail: {'description': 'Steering wheel speed request from SVB.The maximum speed shall bedetermined in calibrating phase.', 'enum': {511: 'SVB_STEERINGSPEEDREQUEST_INVALID'}, 'precision': 1.0, 'len': 9, 'name': 'SVB_SteeringSpeedRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|500]', 'bit': 39, 'type': 'enum', 'order': 'intel', 'physical_unit': 'deg/s'}
    void set_p_svb_steeringspeedrequest(uint8_t* data, double svb_steeringspeedrequest);

    // config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Livecounter_SVB_1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    void set_p_livecounter_svb_1(uint8_t* data, int livecounter_svb_1);

    // config detail: {'description': 'Gear request from SVB. SVB shall keep sending requested gear signalvalue in automatically mode.', 'enum': {0: 'SVB_GEARREQUEST_NO_REQUEST', 1: 'SVB_GEARREQUEST_P', 2: 'SVB_GEARREQUEST_R', 3: 'SVB_GEARREQUEST_N', 4: 'SVB_GEARREQUEST_D', 5: 'SVB_GEARREQUEST_RESERVED', 6: 'SVB_GEARREQUEST_RESERVED', 7: 'SVB_GEARREQUEST_INVALID_VALUE'}, 'precision': 1.0, 'len': 3, 'name': 'SVB_GearRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 57, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_gearrequest(uint8_t* data, int svb_gearrequest);

    // config detail: {'description': 'This signal is the status of acceleration request for driving.', 'enum': {0: 'SVB_ACCELERATIONREQUESTST_NO_REQUEST', 1: 'SVB_ACCELERATIONREQUESTST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_AccelerationRequestSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 56, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_accelerationrequestst(uint8_t* data, bool svb_accelerationrequestst);

    // config detail: {'description': 'This signal indicates the taking over request from SVB.When the requestfrom SVB could not be actuated correctly(including message lost, receivedsignal value unexpected) , this signal shall be set to ask driver taking over.', 'enum': {0: 'SVB_TAKEOVERREQUEST_NO_REQUEST', 1: 'SVB_TAKEOVERREQUEST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_TakeOverRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 55, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_takeoverrequest(uint8_t* data, bool svb_takeoverrequest);

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 1, 'name': 'Reserved_SVB1', 'is_signed_var': False, 'physical_range': '[0|1]', 'bit': 54, 'type': 'bool', 'order': 'intel', 'physical_unit': ''}
    void set_p_reserved_svb1(uint8_t* data, bool reserved_svb1);

    // config detail: {'description': 'Acceleration request from SVB.', 'enum': {63: 'SVB_ACCELERATIONREQUEST_INVALID'}, 'precision': 0.1, 'len': 6, 'name': 'SVB_AccelerationRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|6.3]', 'bit': 48, 'type': 'enum', 'order': 'intel', 'physical_unit': 'm/s2'}
    void set_p_svb_accelerationrequest(uint8_t* data, double svb_accelerationrequest);

    // config detail: {'description': 'This signal indicates the stand-still request. This signal is used for comfortbraking when vehicle speed is lower than 2km/h(calibrated value).', 'enum': {0: 'SVB_STANDSTILLREQUEST_NO_REQUEST', 1: 'SVB_STANDSTILLREQUEST_REQUEST'}, 'precision': 1.0, 'len': 1, 'name': 'SVB_StandStillRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 38, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_standstillrequest(uint8_t* data, bool svb_standstillrequest);

    // config detail: {'description': 'Steering wheel speed sign request from SVB.', 'enum': {0: 'SVB_STEERINGSPEEDSIGNREQUEST_NO_REQUEST', 1: 'SVB_STEERINGSPEEDSIGNREQUEST_REQUEST', 2: 'SVB_STEERINGSPEEDSIGNREQUEST_RESERVED', 3: 'SVB_STEERINGSPEEDSIGNREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_SteeringSpeedSignRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 36, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_steeringspeedsignrequest(uint8_t* data, int svb_steeringspeedsignrequest);

    // config detail: {'description': 'Steering wheel angle sign request from SVB.', 'enum': {0: 'SVB_STEERINGANGLESIGNREQUEST_NO_REQUEST', 1: 'SVB_STEERINGANGLESIGNREQUEST_LEFT_POSITIVE', 2: 'SVB_STEERINGANGLESIGNREQUEST_RIGHT_NEGATIVE', 3: 'SVB_STEERINGANGLESIGNREQUEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_SteeringAngleSignRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 34, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_steeringanglesignrequest(uint8_t* data, int svb_steeringanglesignrequest);

    // config detail: {'description': 'Steering wheel angle request from SVB. The maximum angle shall bedetermined in calibrating phase.', 'enum': {1023: 'SVB_STEERINGANGLEREQUEST_INVALID'}, 'precision': 1.0, 'len': 10, 'name': 'SVB_SteeringAngleRequest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1022]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': 'deg'}
    void set_p_svb_steeringanglerequest(uint8_t* data, double svb_steeringanglerequest);

    // config detail: {'description': 'This signal is the status of steering wheel request for driving.', 'enum': {0: 'SVB_STEERINGREQUESTST_NO_REQUEST', 1: 'SVB_STEERINGREQUESTST_REQUEST', 2: 'SVB_STEERINGREQUESTST_RESERVED', 3: 'SVB_STEERINGREQUESTST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_SteeringRequestSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 22, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_steeringrequestst(uint8_t* data, int svb_steeringrequestst);

    // config detail: {'description': 'This signal indicates the status of SVB system. When SVB detected its ownfault, this signal shall be set.', 'enum': {0: 'SVB_FAILURESTATUS_NO_ERROR', 1: 'SVB_FAILURESTATUS_ERROR', 2: 'SVB_FAILURESTATUS_RESERVED', 3: 'SVB_FAILURESTATUS_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_FailureStatus', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_failurestatus(uint8_t* data, int svb_failurestatus);

    // config detail: {'description': 'This signal indicates the status of deceleration request status of SVB.', 'enum': {0: 'SVB_DECELERATIONREQUESTST_NO_REQUEST', 1: 'SVB_DECELERATIONREQUESTST_ACC_REQUEST', 2: 'SVB_DECELERATIONREQUESTST_AEB_REQUEST', 3: 'SVB_DECELERATIONREQUESTST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB_DecelerationRequestSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 18, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb_decelerationrequestst(uint8_t* data, int svb_decelerationrequestst);

    // config detail: {'description': 'This signal is deceleration request for wheel brake.', 'enum': {1023: 'SVB_DECELERATIONREQUEST_INVALID'}, 'precision': 0.01, 'len': 10, 'name': 'SVB_DecelerationRequest', 'is_signed_var': False, 'offset': -10.0, 'physical_range': '[-10|0]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': 'm/s2'}
    void set_p_svb_decelerationrequest(uint8_t* data, double svb_decelerationrequest);

    // config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_SVB_1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_checksum_svb_1(uint8_t* data, int checksum_svb_1);

    SVB_1_108::SVB_MaxSpeedLimitType svb_maxspeedlimit(const std::uint8_t* bytes, const int32_t length) const;

    int svb_steeringspeedrequest(const std::uint8_t* bytes, const int32_t length) const;
    
    int livecounter_svb_1(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_1_108::SVB_GearRequestType svb_gearrequest(const std::uint8_t* bytes, const int32_t length) const;

    SVB_1_108::SVB_AccelerationRequestStType svb_accelerationrequestst(const std::uint8_t* bytes, const int32_t length) const;

    SVB_1_108::SVB_TakeOverRequestType svb_takeoverrequest(const std::uint8_t* bytes, const int32_t length) const;

    int reserved_svb1(const std::uint8_t* bytes, const int32_t length) const;

    double svb_accelerationrequest(const std::uint8_t* bytes, const int32_t length) const;

    SVB_1_108::SVB_StandStillRequestType svb_standstillrequest(const std::uint8_t* bytes, const int32_t length) const;

    SVB_1_108::SVB_SteeringSpeedSignRequestType svb_steeringspeedsignrequest(const std::uint8_t* bytes, const int32_t length) const;

    SVB_1_108::SVB_SteeringAngleSignRequestType svb_steeringanglesignrequest(const std::uint8_t* bytes, const int32_t length) const;

    int svb_steeringanglerequest(const std::uint8_t* bytes, const int32_t length) const;

    SVB_1_108::SVB_SteeringRequestStType svb_steeringrequestst(const std::uint8_t* bytes, const int32_t length) const;

    SVB_1_108::SVB_FailureStatusType svb_failurestatus(const std::uint8_t* bytes, const int32_t length) const;

    SVB_1_108::SVB_DecelerationRequestStType svb_decelerationrequestst(const std::uint8_t* bytes, const int32_t length) const;

    double svb_decelerationrequest(const std::uint8_t* bytes, const int32_t length) const;

    int checksum_svb_1(const std::uint8_t* bytes, const int32_t length) const;

private:
    double _svb_steeringspeedrequest = 0.0;
    int _svb_steeringspeedrequest_x = 0;
    int _livecounter_svb_1 = 0;
    int _livecounter_svb_1_x = 0;
    int _svb_gearrequest = 0;
    int _svb_gearrequest_x = 0;
    bool _svb_accelerationrequestst = false;
    int _svb_accelerationrequestst_x = 0;
    bool _svb_takeoverrequest = false;
    int _svb_takeoverrequest_x = 0;
    bool _reserved_svb1 = false;
    int _reserved_svb1_x = 0;
    double _svb_accelerationrequest = 0.0;
    int _svb_accelerationrequest_x = 0;
    bool _svb_standstillrequest = false;
    int _svb_standstillrequest_x = 0;
    int _svb_steeringspeedsignrequest = 0;
    int _svb_steeringspeedsignrequest_x = 0;
    int _svb_steeringanglesignrequest = 0;
    int _svb_steeringanglesignrequest_x = 0;
    double _svb_steeringanglerequest = 0.0;
    int _svb_steeringanglerequest_x = 0;
    int _svb_steeringrequestst = 0;
    int _svb_steeringrequestst_x = 0;
    int _svb_failurestatus = 0;
    int _svb_failurestatus_x = 0;
    int _svb_decelerationrequestst = 0;
    int _svb_decelerationrequestst_x = 0;
    double _svb_decelerationrequest = 0.0;
    int _svb_decelerationrequest_x = 0;
    int _checksum_svb_1 = 0;
    int _checksum_svb_1_x = 0;
    int _svb_maxspeedlimit = 0;
    int _svb_maxspeedlimit_x = 0;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo