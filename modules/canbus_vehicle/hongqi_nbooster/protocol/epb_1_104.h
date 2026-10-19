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

class Epb1104 : public ::apollo::drivers::canbus::ProtocolData<
            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;
    Epb1104();
    void Parse(const std::uint8_t* bytes, int32_t length,
                Hongqi_nbooster* chassis) const override;

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 32, 'name': 'Reserved_EPB_1_3', 'is_signed_var': False, 'physical_range': '[0|4294967295]', 'bit': 32, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int reserved_epb_1_3(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'reserved for future extensions', 'enum': {0: 'EPB_SWITCH_NO_REQUEST', 1: 'EPB_SWITCH_LOCK', 2: 'EPB_SWITCH_RELEASE', 3: 'EPB_SWITCH_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'EPB_Switch', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 30, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    EPB_1_104::EPB_SwitchType epb_switch(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 9, 'name': 'Reserved_EPB_1_2', 'is_signed_var': False, 'physical_range': '[0|511]', 'bit': 21, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int reserved_epb_1_2(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Indicate the EGSM is error or not.', 'enum': {0: 'EPB_ERRORSTATUS_NO_ERROR', 1: 'EPB_ERRORSTATUS_ERROR'}, 'precision': 1.0, 'len': 1, 'name': 'EPB_ErrorStatus', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    EPB_1_104::EPB_ErrorStatusType epb_errorstatus(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'reserved for future extensions', 'enum': {0: 'EPB_STATUS_RELEASED', 1: 'EPB_STATUS_LOCKED', 2: 'EPB_STATUS_RELEASING', 3: 'EPB_STATUS_LOCKING', 4: 'EPB_STATUS_UNKOWN'}, 'precision': 1.0, 'len': 4, 'name': 'EPB_Status', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|15]', 'bit': 16, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    EPB_1_104::EPB_StatusType epb_status(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Reserved_EPB_1_1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 12, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int reserved_epb_1_1(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Live counter for EPB_1', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Livecounter_EPB1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 8, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    int livecounter_epb1(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Checksum of message EPB_1', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_EPB_1', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int checksum_epb_1(const std::uint8_t* bytes, int length) const;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
