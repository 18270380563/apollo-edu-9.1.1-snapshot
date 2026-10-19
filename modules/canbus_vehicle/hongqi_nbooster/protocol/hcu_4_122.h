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

class Hcu4122 : public ::apollo::drivers::canbus::ProtocolData<
            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;
    Hcu4122();
    void Parse(const std::uint8_t* bytes, int32_t length, Hongqi_nbooster* chassis) const override;

    // config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 9, 'name': 'HCU_Reserved_8', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 55, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int hcu_reserved_8(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 21, 'name': 'HCU_Reserved_7', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 32, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int hcu_reserved_7(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'signal from brake pedal sended by HCU; harmonized brake switch status', 'enum': {0: 'HCU_BRAKESWITCH_BRAKE_NOT_ACTUATED', 1: 'HCU_BRAKESWITCH_BRAKE_ACTUATED', 2: 'HCU_BRAKESWITCH_RESERVED', 3: 'HCU_BRAKESWITCH_INVALID_VALUE'}, 'precision': 1.0, 'len': 2, 'name': 'HCU_BrakeSwitch', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 53, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    HCU_4_122::HCU_BrakeSwitchType hcu_brakeswitch(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Acceleration pedal position', 'offset': 0.0, 'precision': 0.4, 'len': 8, 'name': 'HCU_AccelerationPedalPosition', 'is_signed_var': False, 'physical_range': '[0|100]', 'bit': 24, 'type': 'double', 'order': 'intel', 'physical_unit': '%'}
    double hcu_accelerationpedalposition(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 24, 'name': 'HCU_Reserved_6', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int hcu_reserved_6(const std::uint8_t* bytes, int length) const;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
