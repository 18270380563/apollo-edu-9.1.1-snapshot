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

class Hcu14248 : public ::apollo::drivers::canbus::ProtocolData<
            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;
    Hcu14248();
    void Parse(const std::uint8_t* bytes, int32_t length,
                Hongqi_nbooster* chassis) const override;
private:
    // config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 20, 'name': 'HCU_Reserved_2', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 44, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int hcu_reserved_2(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Lever info for display', 'enum': {0: 'HCU_LEVERINFO_P', 1: 'HCU_LEVERINFO_R', 2: 'HCU_LEVERINFO_N', 3: 'HCU_LEVERINFO_D', 4: 'HCU_LEVERINFO_RESERVED', 5: 'HCU_LEVERINFO_RESERVED', 6: 'HCU_LEVERINFO_RESERVED', 7: 'HCU_LEVERINFO_NO_CONNECTION', 8: 'HCU_LEVERINFO_RESERVED', 9: 'HCU_LEVERINFO_RESERVED', 10: 'HCU_LEVERINFO_RESERVED', 11: 'HCU_LEVERINFO_RESERVED', 12: 'HCU_LEVERINFO_RESERVED', 13: 'HCU_LEVERINFO_RESERVED', 14: 'HCU_LEVERINFO_RESERVED', 15: 'HCU_LEVERINFO_INVALID_VALUE'}, 'precision': 1.0, 'len': 4, 'name': 'HCU_LeverInfo', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|15]', 'bit': 40, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    HCU_14_248::HCU_LeverInfoType hcu_leverinfo(const std::uint8_t* bytes, int length) const;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
