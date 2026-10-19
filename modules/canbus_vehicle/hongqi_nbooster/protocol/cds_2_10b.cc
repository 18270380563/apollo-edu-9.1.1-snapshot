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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/cds_2_10b.h"

 #include "glog/logging.h"
 
 #include "modules/drivers/canbus/common/byte.h"
 #include "modules/drivers/canbus/common/canbus_consts.h"
 
 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

const int32_t Cds210b::ID = 0x10B;

Cds210b::Cds210b() {}

void Cds210b::Parse(const std::uint8_t* bytes, int32_t length, Hongqi_nbooster* chassis) const {
    chassis->mutable_cds_2_10b()->set_reserved_cds_2_1(reserved_cds_2_1(bytes, length));
    chassis->mutable_cds_2_10b()->set_dcdc2_status(dcdc2_status(bytes, length));
    chassis->mutable_cds_2_10b()->set_livecounter_cds2(livecounter_cds2(bytes, length));
    chassis->mutable_cds_2_10b()->set_checksum_cds2(checksum_cds2(bytes, length));
    chassis->mutable_cds_2_10b()->set_hcu_batterypacksoc(hcu_batterypacksoc(bytes, length));
    // car_status->mutable_safety()->set_dcdc2_system_fault(dcdc2_status(bytes, length));
}

// config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 2.0, 'len': 2, 'name': 'reserved_cds_2_1', 'is_signed_var': False, 'physical_range': '[0|6]', 'bit': 14, 'type': 'double', 'order': 'intel', 'physical_unit': ''}
double Cds210b::reserved_cds_2_1(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(6, 2);
    double ret = x * 2.000000;
    return  ret;
}

// config detail: {'name': 'hcu_batterypacksoc', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 16, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Cds210b::hcu_batterypacksoc(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(0, 8);
    int ret = x;
    return  ret;
}

// config detail: {'name': 'dcdc2_status', 'enum': {0: 'DCDC2_STATUS_NOT_WORKING', 1: 'DCDC2_STATUS_WORKING', 2: 'DCDC2_STATUS_NORMAL_ERROR', 3: 'DCDC2_STATUS_CRITICAL_ERROR'}, 'precision': 1.0, 'len': 2, 'is_signed_var': True, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 12, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
CDS_2_10b::DCDC2_StatusType Cds210b::dcdc2_status(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(4, 2);
    x <<= 30;
    x >>= 30;
    CDS_2_10b::DCDC2_StatusType ret = static_cast<CDS_2_10b::DCDC2_StatusType>(x);
    return  ret;
}

// config detail: {'description': 'LiveCounter_CDS1', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'livecounter_cds2', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 8, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
int Cds210b::livecounter_cds2(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 1);
    int32_t x = t0.get_byte(0, 4);
    int ret = x;
    return  ret;
}

// config detail: {'description': 'checksum of message CDS_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'checksum_cds2', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Cds210b::checksum_cds2(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 0);
    int32_t x = t0.get_byte(0, 8);
    int ret = x;
    return  ret;
}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
