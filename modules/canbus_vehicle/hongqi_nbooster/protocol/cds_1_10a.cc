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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/cds_1_10a.h"

 #include "glog/logging.h"
 
 #include "modules/drivers/canbus/common/byte.h"
 #include "modules/drivers/canbus/common/canbus_consts.h"
  
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

const int32_t Cds110a::ID = 0x10A;

Cds110a::Cds110a() {}

void Cds110a::Parse(const std::uint8_t* bytes, int32_t length,
                    Hongqi_nbooster* chassis) const {
    //     if (cds_steeringautocontrolst(bytes, length) == 1
    //     && !cds_steeringtakeoverst(bytes, length)) {
    //     chassis->mutable_handshake()->set_is_eps_online(true);
    // } else {
    //     chassis->mutable_handshake()->set_is_eps_online(false);
    //     LOG_ERROR << "cds_steeringautocontrolst: " << cds_steeringautocontrolst(bytes, length)
    //             << ", cds_steeringtakeoverst: " << cds_steeringtakeoverst(bytes, length);
    // }
    chassis->mutable_cds_1_10a()->set_livecounter_cds1(livecounter_cds1(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_systemcontrolst(cds_systemcontrolst(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_preparing_for_auto_mode(cds_preparing_for_auto_mode(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_limited_driving_acc(cds_limited_driving_acc(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_steeringtakeoverst(cds_steeringtakeoverst(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_steeringfailurest(cds_steeringfailurest(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_steeringautocontrolst(cds_steeringautocontrolst(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_systemfailurest(cds_systemfailurest(bytes, length));
    //chassis->mutable_cds_1_10a()->set_reserved_cds_1_1(reserved_cds_1_1(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_drivingtakeoverst(cds_drivingtakeoverst(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_brakingtakeoverst(cds_brakingtakeoverst(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_drivingfailurest(cds_drivingfailurest(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_brakingfailurest(cds_brakingfailurest(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_drivingautocontrolst(cds_drivingautocontrolst(bytes, length));
    chassis->mutable_cds_1_10a()->set_cds_brakingautocontrolst(cds_brakingautocontrolst(bytes, length));
    chassis->mutable_cds_1_10a()->set_checksum_cds1(checksum_cds1(bytes, length));
}

// config detail: {'name': 'cds_systemcontrolst', 'enum': {0: 'CDS_SYSTEMCONTROLST_INACTIVE', 1: 'CDS_SYSTEMCONTROLST_SVB1_ENABLE', 2: 'CDS_SYSTEMCONTROLST_SVB2_ENABLE', 3: 'CDS_SYSTEMCONTROLST_RESERVED'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 21, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_1_10a::CDS_SystemControlStType Cds110a::cds_systemcontrolst(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(5, 2);
    
    CDS_1_10a::CDS_SystemControlStType ret = static_cast<CDS_1_10a::CDS_SystemControlStType>(x);
    return  ret;
}

// config detail: {'description': 'LiveCounter_CDS1', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'livecounter_cds1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
int Cds110a::livecounter_cds1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(4, 4);

    int ret = x;
    return  ret;
}

CDS_1_10a::CDS_PreparingForAutoModeType Cds110a::cds_preparing_for_auto_mode(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(0, 1);

    CDS_1_10a::CDS_PreparingForAutoModeType ret = static_cast<CDS_1_10a::CDS_PreparingForAutoModeType>(x);
    return  ret;
}

CDS_1_10a::CDS_LimitedDrivingAccType Cds110a::cds_limited_driving_acc(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(1, 1);

    CDS_1_10a::CDS_LimitedDrivingAccType ret = static_cast<CDS_1_10a::CDS_LimitedDrivingAccType>(x);
    return  ret;
}

// config detail: {'description': 'This signal indicates the driving system is under automatical control ornot.Requirement: After it receives [Inactive], HAD will send [No Request] forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_STEERINGTAKEOVERST_INACTIVE', 1: 'CDS_STEERINGTAKEOVERST_ACTIVE'}, 'precision': 1.0, 'len': 1, 'name': 'cds_steeringtakeoverst', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 30, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_1_10a::CDS_SteeringTakeOverStType Cds110a::cds_steeringtakeoverst(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(6, 1);

    CDS_1_10a::CDS_SteeringTakeOverStType ret = static_cast<CDS_1_10a::CDS_SteeringTakeOverStType>(x);
    return  ret;
}

// config detail: {'description': 'This signal indicates the braking system is in fault status and can notresponse automatic control request.', 'enum': {0: 'CDS_STEERINGFAILUREST_NO_FAIL', 1: 'CDS_STEERINGFAILUREST_EPS_FAIL', 2: 'CDS_STEERINGFAILUREST_SAS_FAIL', 3: 'CDS_STEERINGFAILUREST_STEER_TEMP_FAIL', 4: 'CDS_STEERINGFAILUREST_RESERVED', 5: 'CDS_STEERINGFAILUREST_RESERVED', 6: 'CDS_STEERINGFAILUREST_RESERVED', 7: 'CDS_STEERINGFAILUREST_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'cds_steeringfailurest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 27, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_1_10a::CDS_SteeringFailureStType Cds110a::cds_steeringfailurest(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(3, 3);

    CDS_1_10a::CDS_SteeringFailureStType ret = static_cast<CDS_1_10a::CDS_SteeringFailureStType>(x);
    return  ret;
}

// config detail: {'description': 'This signal indicates the braking system is under automatical control or not.Requirement: After it receives [Inactive], HAD will send [No Request]forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_STEERINGAUTOCONTROLST_INACTIVE', 1: 'CDS_STEERINGAUTOCONTROLST_ACTIVE', 2: 'CDS_STEERINGAUTOCONTROLST_DEGRADE', 3: 'CDS_STEERINGAUTOCONTROLST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'cds_steeringautocontrolst', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 19, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_1_10a::CDS_SteeringAutoControlStType Cds110a::cds_steeringautocontrolst(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(3, 2);

    CDS_1_10a::CDS_SteeringAutoControlStType ret = static_cast<CDS_1_10a::CDS_SteeringAutoControlStType>(x);
    return  ret;
}

// config detail: {'description': 'This signal indicates the braking system is in fault status and can notresponse automatic control request.', 'enum': {0: 'CDS_SYSTEMFAILUREST_NO_FAIL', 1: 'CDS_SYSTEMFAILUREST_LOSE_SVB1', 2: 'CDS_SYSTEMFAILUREST_LOSE_SVB2', 3: 'CDS_SYSTEMFAILUREST_LOSE_SVB1_AND_SVB2', 4: 'CDS_SYSTEMFAILUREST_RESERED', 5: 'CDS_SYSTEMFAILUREST_RESERED', 6: 'CDS_SYSTEMFAILUREST_RESERED', 7: 'CDS_SYSTEMFAILUREST_RESERED'}, 'precision': 1.0, 'len': 3, 'name': 'cds_systemfailurest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_1_10a::CDS_SystemFailureStType Cds110a::cds_systemfailurest(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 3);
    int32_t x = t0.get_byte(0, 3);

    CDS_1_10a::CDS_SystemFailureStType ret = static_cast<CDS_1_10a::CDS_SystemFailureStType>(x);
    return  ret;
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 29, 'name': 'reserved_cds_1_1', 'is_signed_var': False, 'physical_range': '[0|536870911]', 'bit': 31, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Cds110a::reserved_cds_1_1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(2, 2);
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
    Byte t4(bytes + 3);
    t = t4.get_byte(7, 1);
    x <<= 1;
    x |= t;

    int ret = x;
    return  ret;
}

// config detail: {'description': 'This signal indicates the driving system is under automatical control ornot.Requirement: After it receives [Inactive], HAD will send [No Request] forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_DRIVINGTAKEOVERST_INACTIVE', 1: 'CDS_DRIVINGTAKEOVERST_ACTIVE'}, 'precision': 1.0, 'len': 1, 'name': 'cds_drivingtakeoverst', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 23, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_1_10a::CDS_DrivingTakeOverStType Cds110a::cds_drivingtakeoverst(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(7, 1);

    CDS_1_10a::CDS_DrivingTakeOverStType ret = static_cast<CDS_1_10a::CDS_DrivingTakeOverStType>(x);
    return  ret;
}

// config detail: {'description': 'This signal indicates the driving system is under automatical control ornot.Requirement: After it receives [Inactive], HAD will send [No Request] forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_BRAKINGTAKEOVERST_INACTIVE', 1: 'CDS_BRAKINGTAKEOVERST_ACTIVE'}, 'precision': 1.0, 'len': 1, 'name': 'cds_brakingtakeoverst', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 15, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_1_10a::CDS_BrakingTakeOverStType Cds110a::cds_brakingtakeoverst(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(7, 1);

    CDS_1_10a::CDS_BrakingTakeOverStType ret = static_cast<CDS_1_10a::CDS_BrakingTakeOverStType>(x);
    return  ret;
}

// config detail: {'description': 'This signal indicates the braking system is in fault status and can notresponse automatic control request.', 'enum': {0: 'CDS_DRIVINGFAILUREST_NO_FAIL', 1: 'CDS_DRIVINGFAILUREST_HCU_FAIL', 2: 'CDS_DRIVINGFAILUREST_SPD_SENSOR_FAIL', 3: 'CDS_DRIVINGFAILUREST_YRS_FAIL', 4: 'CDS_DRIVINGFAILUREST_RESERVED', 5: 'CDS_DRIVINGFAILUREST_RESERVED', 6: 'CDS_DRIVINGFAILUREST_RESERVED', 7: 'CDS_DRIVINGFAILUREST_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'cds_drivingfailurest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 16, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_1_10a::CDS_DrivingFailureStType Cds110a::cds_drivingfailurest(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(0, 3);

    CDS_1_10a::CDS_DrivingFailureStType ret = static_cast<CDS_1_10a::CDS_DrivingFailureStType>(x);
    return  ret;
}

// config detail: {'description': 'This signal indicates the braking system is in fault status and can notresponse automatic control request.', 'enum': {0: 'CDS_BRAKINGFAILUREST_NO_FAIL', 1: 'CDS_BRAKINGFAILUREST_ESP_FAIL', 2: 'CDS_BRAKINGFAILUREST_SPD_SENSOR_FAIL', 3: 'CDS_BRAKINGFAILUREST_YRS_FAIL', 4: 'CDS_BRAKINGFAILUREST_HCU_FAIL', 5: 'CDS_BRAKINGFAILUREST_ESP_AND_HCU_FAIL', 6: 'CDS_BRAKINGFAILUREST_RESERVED', 7: 'CDS_BRAKINGFAILUREST_RESERVED'}, 'precision': 1.0, 'len': 3, 'name': 'cds_brakingfailurest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 12, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_1_10a::CDS_BrakingFailureStType Cds110a::cds_brakingfailurest(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(4, 3);

    CDS_1_10a::CDS_BrakingFailureStType ret = static_cast<CDS_1_10a::CDS_BrakingFailureStType>(x);
    return  ret;
}

// config detail: {'description': 'This signal indicates the driving system is under automatical control ornot.Requirement: After it receives [Inactive], HAD will send [No Request] forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_DRIVINGAUTOCONTROLST_INACTIVE', 1: 'CDS_DRIVINGAUTOCONTROLST_ACTIVE', 2: 'CDS_DRIVINGAUTOCONTROLST_DEGRADE', 3: 'CDS_DRIVINGAUTOCONTROLST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'cds_drivingautocontrolst', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 10, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_1_10a::CDS_DrivingAutoControlStType Cds110a::cds_drivingautocontrolst(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(2, 2);

    CDS_1_10a::CDS_DrivingAutoControlStType ret = static_cast<CDS_1_10a::CDS_DrivingAutoControlStType>(x);
    return  ret;
}

// config detail: {'description': 'This signal indicates the braking system is under automatical control or not.Requirement: After it receives [Inactive], HAD will send [No Request]forsome cycles then send [Request], which shall be actuated correctly.', 'enum': {0: 'CDS_BRAKINGAUTOCONTROLST_INACTIVE', 1: 'CDS_BRAKINGAUTOCONTROLST_ACTIVE', 2: 'CDS_BRAKINGAUTOCONTROLST_DEGRADE', 3: 'CDS_BRAKINGAUTOCONTROLST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'cds_brakingautocontrolst', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_1_10a::CDS_BrakingAutoControlStType Cds110a::cds_brakingautocontrolst(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(0, 2);

    CDS_1_10a::CDS_BrakingAutoControlStType ret = static_cast<CDS_1_10a::CDS_BrakingAutoControlStType>(x);
    return  ret;
}

// config detail: {'description': 'checksum of message CDS_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'checksum_cds1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Cds110a::checksum_cds1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 0);
    int32_t x = t0.get_byte(0, 8);

    int ret = x;
    return  ret;
}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
