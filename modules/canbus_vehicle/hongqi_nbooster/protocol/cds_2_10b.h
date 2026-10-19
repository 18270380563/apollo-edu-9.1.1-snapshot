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

class Cds210b : public ::apollo::drivers::canbus::ProtocolData<
            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;
    Cds210b();
    void Parse(const std::uint8_t* bytes, int32_t length,
                Hongqi_nbooster* chassis) const override;

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 2.0, 'len': 2, 'name': 'Reserved_CDS_2_1', 'is_signed_var': False, 'physical_range': '[0|6]', 'bit': 14, 'type': 'double', 'order': 'intel', 'physical_unit': ''}
    double reserved_cds_2_1(const std::uint8_t* bytes, int length) const;

    // config detail: {'name': 'HCU_BatteryPackSOC', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 16, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int hcu_batterypacksoc(const std::uint8_t* bytes, int length) const;

    // config detail: {'name': 'DCDC2_Status', 'enum': {0: 'DCDC2_STATUS_NOT_WORKING', 1: 'DCDC2_STATUS_WORKING', 2: 'DCDC2_STATUS_NORMAL_ERROR', 3: 'DCDC2_STATUS_CRITICAL_ERROR'}, 'precision': 1.0, 'len': 2, 'is_signed_var': True, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 12, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    CDS_2_10b::DCDC2_StatusType dcdc2_status(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'LiveCounter_CDS1', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'LiveCounter_CDS2', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 8, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    int livecounter_cds2(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'checksum of message CDS_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_CDS2', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int checksum_cds2(const std::uint8_t* bytes, int length) const;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
