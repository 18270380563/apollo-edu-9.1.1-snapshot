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

class Cds110a : public ::apollo::drivers::canbus::ProtocolData<
            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;
    Cds110a();
    void Parse(const std::uint8_t* bytes, int32_t length,
                Hongqi_nbooster* chassis) const override;

    // config detail: {'name': 'CDS_SystemControlSt', 'enum': {0: 'CDS_SYSTEMCONTROLST_INACTIVE', 1: 'CDS_SYSTEMCONTROLST_SVB1_ENABLE', 2: 'CDS_SYSTEMCONTROLST_SVB2_ENABLE', 3: 'CDS_SYSTEMCONTROLST_RESERVED'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 21, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_1_10a::CDS_SystemControlStType cds_systemcontrolst(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'LiveCounter_CDS1', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'LiveCounter_CDS1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    int livecounter_cds1(const std::uint8_t* bytes, int length) const;

    CDS_1_10a::CDS_PreparingForAutoModeType cds_preparing_for_auto_mode(const std::uint8_t* bytes, int length) const;

    CDS_1_10a::CDS_LimitedDrivingAccType cds_limited_driving_acc(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'This signal indicates the driving system is under automatical control ornot.Requirement: After it receives [Inactive], HAD will send [No Request] forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_STEERINGTAKEOVERST_INACTIVE', 1: 'CDS_STEERINGTAKEOVERST_ACTIVE'}, 'precision': 1.0, 'len': 1, 'name': 'CDS_SteeringTakeOverSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 30, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_1_10a::CDS_SteeringTakeOverStType cds_steeringtakeoverst(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'This signal indicates the braking system is in fault status and can notresponse automatic control request.', 'enum': {0: 'CDS_STEERINGFAILUREST_NO_FAIL', 1: 'CDS_STEERINGFAILUREST_EPS_FAIL', 2: 'CDS_STEERINGFAILUREST_SAS_FAIL', 3: 'CDS_STEERINGFAILUREST_STEER_TEMP_FAIL', 4: 'CDS_STEERINGFAILUREST_RESERVED', 5: 'CDS_STEERINGFAILUREST_RESERVED', 6: 'CDS_STEERINGFAILUREST_RESERVED', 7: 'CDS_STEERINGFAILUREST_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'CDS_SteeringFailureSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 27, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_1_10a::CDS_SteeringFailureStType cds_steeringfailurest(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'This signal indicates the braking system is under automatical control or not.Requirement: After it receives [Inactive], HAD will send [No Request]forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_STEERINGAUTOCONTROLST_INACTIVE', 1: 'CDS_STEERINGAUTOCONTROLST_ACTIVE', 2: 'CDS_STEERINGAUTOCONTROLST_DEGRADE', 3: 'CDS_STEERINGAUTOCONTROLST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'CDS_SteeringAutoControlSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 19, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_1_10a::CDS_SteeringAutoControlStType cds_steeringautocontrolst(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'This signal indicates the braking system is in fault status and can notresponse automatic control request.', 'enum': {0: 'CDS_SYSTEMFAILUREST_NO_FAIL', 1: 'CDS_SYSTEMFAILUREST_LOSE_SVB1', 2: 'CDS_SYSTEMFAILUREST_LOSE_SVB2', 3: 'CDS_SYSTEMFAILUREST_LOSE_SVB1_AND_SVB2', 4: 'CDS_SYSTEMFAILUREST_RESERED', 5: 'CDS_SYSTEMFAILUREST_RESERED', 6: 'CDS_SYSTEMFAILUREST_RESERED', 7: 'CDS_SYSTEMFAILUREST_RESERED'}, 'precision': 1.0, 'len': 3, 'name': 'CDS_SystemFailureSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_1_10a::CDS_SystemFailureStType cds_systemfailurest(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 29, 'name': 'Reserved_CDS_1_1', 'is_signed_var': False, 'physical_range': '[0|536870911]', 'bit': 31, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int reserved_cds_1_1(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'This signal indicates the driving system is under automatical control ornot.Requirement: After it receives [Inactive], HAD will send [No Request] forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_DRIVINGTAKEOVERST_INACTIVE', 1: 'CDS_DRIVINGTAKEOVERST_ACTIVE'}, 'precision': 1.0, 'len': 1, 'name': 'CDS_DrivingTakeOverSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 23, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_1_10a::CDS_DrivingTakeOverStType cds_drivingtakeoverst(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'This signal indicates the driving system is under automatical control ornot.Requirement: After it receives [Inactive], HAD will send [No Request] forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_BRAKINGTAKEOVERST_INACTIVE', 1: 'CDS_BRAKINGTAKEOVERST_ACTIVE'}, 'precision': 1.0, 'len': 1, 'name': 'CDS_BrakingTakeOverSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 15, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_1_10a::CDS_BrakingTakeOverStType cds_brakingtakeoverst(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'This signal indicates the braking system is in fault status and can notresponse automatic control request.', 'enum': {0: 'CDS_DRIVINGFAILUREST_NO_FAIL', 1: 'CDS_DRIVINGFAILUREST_HCU_FAIL', 2: 'CDS_DRIVINGFAILUREST_SPD_SENSOR_FAIL', 3: 'CDS_DRIVINGFAILUREST_YRS_FAIL', 4: 'CDS_DRIVINGFAILUREST_RESERVED', 5: 'CDS_DRIVINGFAILUREST_RESERVED', 6: 'CDS_DRIVINGFAILUREST_RESERVED', 7: 'CDS_DRIVINGFAILUREST_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'CDS_DrivingFailureSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 16, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_1_10a::CDS_DrivingFailureStType cds_drivingfailurest(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'This signal indicates the braking system is in fault status and can notresponse automatic control request.', 'enum': {0: 'CDS_BRAKINGFAILUREST_NO_FAIL', 1: 'CDS_BRAKINGFAILUREST_ESP_FAIL', 2: 'CDS_BRAKINGFAILUREST_SPD_SENSOR_FAIL', 3: 'CDS_BRAKINGFAILUREST_YRS_FAIL', 4: 'CDS_BRAKINGFAILUREST_HCU_FAIL', 5: 'CDS_BRAKINGFAILUREST_ESP_AND_HCU_FAIL', 6: 'CDS_BRAKINGFAILUREST_RESERVED', 7: 'CDS_BRAKINGFAILUREST_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'CDS_BrakingFailureSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 12, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_1_10a::CDS_BrakingFailureStType cds_brakingfailurest(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'This signal indicates the driving system is under automatical control ornot.Requirement: After it receives [Inactive], HAD will send [No Request] forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_DRIVINGAUTOCONTROLST_INACTIVE', 1: 'CDS_DRIVINGAUTOCONTROLST_ACTIVE', 2: 'CDS_DRIVINGAUTOCONTROLST_DEGRADE', 3: 'CDS_DRIVINGAUTOCONTROLST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'CDS_DrivingAutoControlSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 10, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_1_10a::CDS_DrivingAutoControlStType cds_drivingautocontrolst(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'This signal indicates the braking system is under automatical control or not.Requirement: After it receives [Inactive], HAD will send [No Request]forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_BRAKINGAUTOCONTROLST_INACTIVE', 1: 'CDS_BRAKINGAUTOCONTROLST_ACTIVE', 2: 'CDS_BRAKINGAUTOCONTROLST_DEGRADE', 3: 'CDS_BRAKINGAUTOCONTROLST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'CDS_BrakingAutoControlSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_1_10a::CDS_BrakingAutoControlStType cds_brakingautocontrolst(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'checksum of message CDS_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_CDS1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int checksum_cds1(const std::uint8_t* bytes, int length) const;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
