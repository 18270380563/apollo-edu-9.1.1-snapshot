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

class Svb32c0 : public ::apollo::drivers::canbus::ProtocolData<
            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;

    Svb32c0();

    uint32_t GetPeriod() const override;

    void Parse(const std::uint8_t* bytes, int32_t length,
                       Hongqi_nbooster* chassis) const override;
  
    void UpdateData_Heartbeat(uint8_t* data) override;
  
    void UpdateData(uint8_t* data) override;

    void UpdateMore(uint8_t* data);
  
    void Reset() override;


    // config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'SVB3_Livecounter', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    Svb32c0* set_svb3_livecounter(int svb3_livecounter);

    // config detail: {'description': 'Vehicle speed output from ABS(derived from raw data)', 'offset': 0.0, 'precision': 0.01, 'len': 15, 'name': 'SVB3_ABS_VehicleSpeed', 'is_signed_var': False, 'physical_range': '[0|327.67]', 'bit': 24, 'type': 'double', 'order': 'intel', 'physical_unit': 'Km/h'}
    Svb32c0* set_svb3_abs_vehiclespeed(double svb3_abs_vehiclespeed);

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'SVB3_Reserved1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 20, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svb32c0* set_svb3_reserved1(int svb3_reserved1);

    // config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 21, 'name': 'SVB3_Reserved2', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 39, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svb32c0* set_svb3_reserved2(int svb3_reserved2);

    // config detail: {'description': 'Longitudinal acceleration(raw data;sign depends on mounting position)front of vehicle=positive,back of vehicle=negative', 'offset': -20.0, 'precision': 0.01, 'len': 12, 'name': 'SVB3_AI', 'is_signed_var': False, 'physical_range': '[-20|20]', 'bit': 8, 'type': 'double', 'order': 'intel', 'physical_unit': 'm/s2'}
    Svb32c0* set_svb3_ai(double svb3_ai);

    // config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'SVB3_Checksum', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svb32c0* set_svb3_checksum(int svb3_checksum);

private:

    // config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'SVB3_Livecounter', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    void set_p_svb3_livecounter(uint8_t* data, int svb3_livecounter);

    // config detail: {'description': 'Vehicle speed output from ABS(derived from raw data)', 'offset': 0.0, 'precision': 0.01, 'len': 15, 'name': 'SVB3_ABS_VehicleSpeed', 'is_signed_var': False, 'physical_range': '[0|327.67]', 'bit': 24, 'type': 'double', 'order': 'intel', 'physical_unit': 'Km/h'}
    void set_p_svb3_abs_vehiclespeed(uint8_t* data, double svb3_abs_vehiclespeed);

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'SVB3_Reserved1', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 20, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb3_reserved1(uint8_t* data, int svb3_reserved1);

    // config detail: {'description': 'Reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 21, 'name': 'SVB3_Reserved2', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 39, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb3_reserved2(uint8_t* data, int svb3_reserved2);

    // config detail: {'description': 'Longitudinal acceleration(raw data;sign depends on mounting position)front of vehicle=positive,back of vehicle=negative', 'offset': -20.0, 'precision': 0.01, 'len': 12, 'name': 'SVB3_AI', 'is_signed_var': False, 'physical_range': '[-20|20]', 'bit': 8, 'type': 'double', 'order': 'intel', 'physical_unit': 'm/s2'}
    void set_p_svb3_ai(uint8_t* data, double svb3_ai);

    // config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'SVB3_Checksum', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb3_checksum(uint8_t* data, int svb3_checksum);

    int svb3_livecounter(const std::uint8_t* bytes, const int32_t length) const;

    double svb3_abs_vehiclespeed(const std::uint8_t* bytes, const int32_t length) const;
    
    int svb3_reserved1(const std::uint8_t* bytes, const int32_t length) const;
    
    int svb3_reserved2(const std::uint8_t* bytes, const int32_t length) const;
    
    double svb3_ai(const std::uint8_t* bytes, const int32_t length) const;
    
    int svb3_checksum(const std::uint8_t* bytes, const int32_t length) const;

private:
    int _svb3_livecounter = 0;
    int _svb3_livecounter_x = 0;
    double _svb3_abs_vehiclespeed = 0.0;
    int _svb3_abs_vehiclespeed_x = 0;
    int _svb3_reserved1 = 0;
    int _svb3_reserved1_x = 0;
    int _svb3_reserved2 = 0;
    int _svb3_reserved2_x = 0;
    double _svb3_ai = 0.0;
    int _svb3_ai_x = 0;
    int _svb3_checksum = 0;
    int _svb3_checksum_x = 0;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
