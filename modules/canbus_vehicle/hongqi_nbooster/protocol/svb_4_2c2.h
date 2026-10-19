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

class Svb42c2 : public ::apollo::drivers::canbus::ProtocolData<
            ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;

    Svb42c2();

    uint32_t GetPeriod() const override;

    void Parse(const std::uint8_t* bytes, int32_t length,
                       Hongqi_nbooster* chassis) const override;
  
    void UpdateData_Heartbeat(uint8_t* data) override;
  
    void UpdateData(uint8_t* data) override;

    void UpdateMore(uint8_t* data);
  
    void Reset() override;

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_9_NO', 1: 'SVB4_OBSTACLE_INFO_9_YES', 2: 'SVB4_OBSTACLE_INFO_9_RESERVED', 3: 'SVB4_OBSTACLE_INFO_9_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_9', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 30, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_obstacle_info_9(int svb4_obstacle_info_9);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_8_NO', 1: 'SVB4_OBSTACLE_INFO_8_YES', 2: 'SVB4_OBSTACLE_INFO_8_RESERVED', 3: 'SVB4_OBSTACLE_INFO_8_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_8', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 28, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_obstacle_info_8(int svb4_obstacle_info_8);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_7_NO', 1: 'SVB4_OBSTACLE_INFO_7_YES', 2: 'SVB4_OBSTACLE_INFO_7_RESERVED', 3: 'SVB4_OBSTACLE_INFO_7_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_7', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 26, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_obstacle_info_7(int svb4_obstacle_info_7);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_6_NO', 1: 'SVB4_OBSTACLE_INFO_6_YES', 2: 'SVB4_OBSTACLE_INFO_6_RESERVED', 3: 'SVB4_OBSTACLE_INFO_6_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_6', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_obstacle_info_6(int svb4_obstacle_info_6);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_5_NO', 1: 'SVB4_OBSTACLE_INFO_5_YES', 2: 'SVB4_OBSTACLE_INFO_5_RESERVED', 3: 'SVB4_OBSTACLE_INFO_5_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_5', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 22, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_obstacle_info_5(int svb4_obstacle_info_5);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_4_NO', 1: 'SVB4_OBSTACLE_INFO_4_YES', 2: 'SVB4_OBSTACLE_INFO_4_RESERVED', 3: 'SVB4_OBSTACLE_INFO_4_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_4', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_obstacle_info_4(int svb4_obstacle_info_4);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_3_NO', 1: 'SVB4_OBSTACLE_INFO_3_YES', 2: 'SVB4_OBSTACLE_INFO_3_RESERVED', 3: 'SVB4_OBSTACLE_INFO_3_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_3', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 18, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_obstacle_info_3(int svb4_obstacle_info_3);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_2_COPY_1_NO', 1: 'SVB4_OBSTACLE_INFO_2_COPY_1_YES', 2: 'SVB4_OBSTACLE_INFO_2_COPY_1_RESERVED', 3: 'SVB4_OBSTACLE_INFO_2_COPY_1_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_2_Copy_1', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 16, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_obstacle_info_2_copy_1(int svb4_obstacle_info_2_copy_1);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_10_NO', 1: 'SVB4_OBSTACLE_INFO_10_YES', 2: 'SVB4_OBSTACLE_INFO_10_RESERVED', 3: 'SVB4_OBSTACLE_INFO_10_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_10', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 32, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_obstacle_info_10(int svb4_obstacle_info_10);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_2_NO', 1: 'SVB4_OBSTACLE_INFO_2_YES', 2: 'SVB4_OBSTACLE_INFO_2_RESERVED', 3: 'SVB4_OBSTACLE_INFO_2_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_2', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 14, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_obstacle_info_2(int svb4_obstacle_info_2);

    // config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'SVB4_Livecounter', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    Svb42c2* set_svb4_livecounter(int svb4_livecounter);

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 26, 'name': 'SVB4_Reserved01', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 34, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_reserved01(int svb4_reserved01);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_1_NO', 1: 'SVB4_OBSTACLE_INFO_1_YES', 2: 'SVB4_OBSTACLE_INFO_1_RESERVED', 3: 'SVB4_OBSTACLE_INFO_1_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_1', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 12, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_obstacle_info_1(int svb4_obstacle_info_1);

    // config detail: {'description': 'Indicate the takeover request inforLamp off or on', 'enum': {0: 'SVB4_TAKEOVERREQUESTINFOLAMP_LAMP_OFF', 1: 'SVB4_TAKEOVERREQUESTINFOLAMP_LAMP_ON', 2: 'SVB4_TAKEOVERREQUESTINFOLAMP_RESERVED', 3: 'SVB4_TAKEOVERREQUESTINFOLAMP_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_TakeoverRequestInfoLamp', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 10, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_takeoverrequestinfolamp(int svb4_takeoverrequestinfolamp);

    // config detail: {'description': 'Indicate the system inforLamp status', 'enum': {0: 'SVB4_SYSTEMINFOLAMP_OFF', 1: 'SVB4_SYSTEMINFOLAMP_STANDBY', 2: 'SVB4_SYSTEMINFOLAMP_ACTIVE', 3: 'SVB4_SYSTEMINFOLAMP_ERROR'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_SystemInfoLamp', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_systeminfolamp(int svb4_systeminfolamp);

    // config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'SVB4_Checksum', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    Svb42c2* set_svb4_checksum(int svb4_checksum);

private:

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_9_NO', 1: 'SVB4_OBSTACLE_INFO_9_YES', 2: 'SVB4_OBSTACLE_INFO_9_RESERVED', 3: 'SVB4_OBSTACLE_INFO_9_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_9', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 30, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_obstacle_info_9(uint8_t* data, int svb4_obstacle_info_9);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_8_NO', 1: 'SVB4_OBSTACLE_INFO_8_YES', 2: 'SVB4_OBSTACLE_INFO_8_RESERVED', 3: 'SVB4_OBSTACLE_INFO_8_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_8', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 28, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_obstacle_info_8(uint8_t* data, int svb4_obstacle_info_8);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_7_NO', 1: 'SVB4_OBSTACLE_INFO_7_YES', 2: 'SVB4_OBSTACLE_INFO_7_RESERVED', 3: 'SVB4_OBSTACLE_INFO_7_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_7', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 26, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_obstacle_info_7(uint8_t* data, int svb4_obstacle_info_7);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_6_NO', 1: 'SVB4_OBSTACLE_INFO_6_YES', 2: 'SVB4_OBSTACLE_INFO_6_RESERVED', 3: 'SVB4_OBSTACLE_INFO_6_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_6', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 24, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_obstacle_info_6(uint8_t* data, int svb4_obstacle_info_6);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_5_NO', 1: 'SVB4_OBSTACLE_INFO_5_YES', 2: 'SVB4_OBSTACLE_INFO_5_RESERVED', 3: 'SVB4_OBSTACLE_INFO_5_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_5', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 22, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_obstacle_info_5(uint8_t* data, int svb4_obstacle_info_5);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_4_NO', 1: 'SVB4_OBSTACLE_INFO_4_YES', 2: 'SVB4_OBSTACLE_INFO_4_RESERVED', 3: 'SVB4_OBSTACLE_INFO_4_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_4', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_obstacle_info_4(uint8_t* data, int svb4_obstacle_info_4);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_3_NO', 1: 'SVB4_OBSTACLE_INFO_3_YES', 2: 'SVB4_OBSTACLE_INFO_3_RESERVED', 3: 'SVB4_OBSTACLE_INFO_3_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_3', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 18, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_obstacle_info_3(uint8_t* data, int svb4_obstacle_info_3);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_2_COPY_1_NO', 1: 'SVB4_OBSTACLE_INFO_2_COPY_1_YES', 2: 'SVB4_OBSTACLE_INFO_2_COPY_1_RESERVED', 3: 'SVB4_OBSTACLE_INFO_2_COPY_1_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_2_Copy_1', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 16, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_obstacle_info_2_copy_1(uint8_t* data, int svb4_obstacle_info_2_copy_1);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_10_NO', 1: 'SVB4_OBSTACLE_INFO_10_YES', 2: 'SVB4_OBSTACLE_INFO_10_RESERVED', 3: 'SVB4_OBSTACLE_INFO_10_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_10', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 32, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_obstacle_info_10(uint8_t* data, int svb4_obstacle_info_10);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_2_NO', 1: 'SVB4_OBSTACLE_INFO_2_YES', 2: 'SVB4_OBSTACLE_INFO_2_RESERVED', 3: 'SVB4_OBSTACLE_INFO_2_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_2', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 14, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_obstacle_info_2(uint8_t* data, int svb4_obstacle_info_2);

    // config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2 \xa1\xad15, 0, 1 \xa1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'SVB4_Livecounter', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    void set_p_svb4_livecounter(uint8_t* data, int svb4_livecounter);

    // config detail: {'description': 'reserved for future extensions', 'offset': 0.0, 'precision': 1.0, 'len': 26, 'name': 'SVB4_Reserved01', 'is_signed_var': False, 'physical_range': '[0|0]', 'bit': 34, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_reserved01(uint8_t* data, int svb4_reserved01);

    // config detail: {'description': 'Indicate the obstacle yes or not', 'enum': {0: 'SVB4_OBSTACLE_INFO_1_NO', 1: 'SVB4_OBSTACLE_INFO_1_YES', 2: 'SVB4_OBSTACLE_INFO_1_RESERVED', 3: 'SVB4_OBSTACLE_INFO_1_DESCRIPTION_FOR_THE_VALUE_0X3'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_obstacle_info_1', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 12, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_obstacle_info_1(uint8_t* data, int svb4_obstacle_info_1);

    // config detail: {'description': 'Indicate the takeover request inforLamp off or on', 'enum': {0: 'SVB4_TAKEOVERREQUESTINFOLAMP_LAMP_OFF', 1: 'SVB4_TAKEOVERREQUESTINFOLAMP_LAMP_ON', 2: 'SVB4_TAKEOVERREQUESTINFOLAMP_RESERVED', 3: 'SVB4_TAKEOVERREQUESTINFOLAMP_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_TakeoverRequestInfoLamp', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 10, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_takeoverrequestinfolamp(uint8_t* data, int svb4_takeoverrequestinfolamp);

    // config detail: {'description': 'Indicate the system inforLamp status', 'enum': {0: 'SVB4_SYSTEMINFOLAMP_OFF', 1: 'SVB4_SYSTEMINFOLAMP_STANDBY', 2: 'SVB4_SYSTEMINFOLAMP_ACTIVE', 3: 'SVB4_SYSTEMINFOLAMP_ERROR'}, 'precision': 1.0, 'len': 2, 'name': 'SVB4_SystemInfoLamp', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_systeminfolamp(uint8_t* data, int svb4_systeminfolamp);

    // config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'SVB4_Checksum', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    void set_p_svb4_checksum(uint8_t* data, int svb4_checksum);

    SVB_4_2c2::SVB4_obstacle_info_9Type svb4_obstacle_info_9(const std::uint8_t* bytes, const int32_t length) const;

    SVB_4_2c2::SVB4_obstacle_info_8Type svb4_obstacle_info_8(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_4_2c2::SVB4_obstacle_info_7Type svb4_obstacle_info_7(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_4_2c2::SVB4_obstacle_info_6Type svb4_obstacle_info_6(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_4_2c2::SVB4_obstacle_info_5Type svb4_obstacle_info_5(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_4_2c2::SVB4_obstacle_info_4Type svb4_obstacle_info_4(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_4_2c2::SVB4_obstacle_info_3Type svb4_obstacle_info_3(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_4_2c2::SVB4_obstacle_info_2_Copy_1Type svb4_obstacle_info_2_copy_1(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_4_2c2::SVB4_obstacle_info_10Type svb4_obstacle_info_10(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_4_2c2::SVB4_obstacle_info_2Type svb4_obstacle_info_2(const std::uint8_t* bytes, const int32_t length) const;
    
    int svb4_livecounter(const std::uint8_t* bytes, const int32_t length) const;
    
    int svb4_reserved01(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_4_2c2::SVB4_obstacle_info_1Type svb4_obstacle_info_1(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_4_2c2::SVB4_TakeoverRequestInfoLampType svb4_takeoverrequestinfolamp(const std::uint8_t* bytes, const int32_t length) const;
    
    SVB_4_2c2::SVB4_SystemInfoLampType svb4_systeminfolamp(const std::uint8_t* bytes, const int32_t length) const;
    
    int svb4_checksum(const std::uint8_t* bytes, const int32_t length) const;

private:
    int _svb4_obstacle_info_9 = 0;
    int _svb4_obstacle_info_9_x = 0;
    int _svb4_obstacle_info_8 = 0;
    int _svb4_obstacle_info_8_x = 0;
    int _svb4_obstacle_info_7 = 0;
    int _svb4_obstacle_info_7_x = 0;
    int _svb4_obstacle_info_6 = 0;
    int _svb4_obstacle_info_6_x = 0;
    int _svb4_obstacle_info_5 = 0;
    int _svb4_obstacle_info_5_x = 0;
    int _svb4_obstacle_info_4 = 0;
    int _svb4_obstacle_info_4_x = 0;
    int _svb4_obstacle_info_3 = 0;
    int _svb4_obstacle_info_3_x = 0;
    int _svb4_obstacle_info_2_copy_1 = 0;
    int _svb4_obstacle_info_2_copy_1_x = 0;
    int _svb4_obstacle_info_10 = 0;
    int _svb4_obstacle_info_10_x = 0;
    int _svb4_obstacle_info_2 = 0;
    int _svb4_obstacle_info_2_x = 0;
    int _svb4_livecounter = 0;
    int _svb4_livecounter_x = 0;
    int _svb4_reserved01 = 0;
    int _svb4_reserved01_x = 0;
    int _svb4_obstacle_info_1 = 0;
    int _svb4_obstacle_info_1_x = 0;
    int _svb4_takeoverrequestinfolamp = 0;
    int _svb4_takeoverrequestinfolamp_x = 0;
    int _svb4_systeminfolamp = 0;
    int _svb4_systeminfolamp_x = 0;
    int _svb4_checksum = 0;
    int _svb4_checksum_x = 0;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
