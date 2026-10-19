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

class Nbsstatus110d :  public ::apollo::drivers::canbus::ProtocolData<
                ::apollo::canbus::Hongqi_nbooster> {
public:
    static const int32_t ID;
    Nbsstatus110d();
    void Parse(const std::uint8_t* bytes, int32_t length,
                Hongqi_nbooster* chassis) const override;

    // config detail: {'name': 'NBS_DecOverride_Status', 'enum': {0: 'NBS_DECOVERRIDE_STATUS_NOT_OVERRIDE', 1: 'NBS_DECOVERRIDE_STATUS_OVERRIDE'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 40, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    NBS_STATUS_1_10d::NBS_DecOverride_StatusType nbs_decoverride_status(const std::uint8_t* bytes, int length) const;

    // config detail: {'name': 'NBS_BrkAutoControl_St', 'enum': {0: 'NBS_BRKAUTOCONTROL_ST_MANUEL_MODE', 1: 'NBS_BRKAUTOCONTROL_ST_BRKPELPOS_CONTROL_ACTIVE', 2: 'NBS_BRKAUTOCONTROL_ST_TARDEC_CONTROL_ACTIVE', 3: 'NBS_BRKAUTOCONTROL_ST_ERROR_MODE'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 38, 'type': 'enum', 'order': 'intel', 'physical_unit': ''} 
    NBS_STATUS_1_10d::NBS_BrkAutoControl_StType nbs_brkautocontrol_st(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Status of driver door', 'enum': {0: 'NBS_MASTERCYLINDERSTROKEST_NOTINITIALIZED', 1: 'NBS_MASTERCYLINDERSTROKEST_NORMAL', 2: 'NBS_MASTERCYLINDERSTROKEST_FAULTY', 3: 'NBS_MASTERCYLINDERSTROKEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'NBS_MasterCylinderStrokeSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 36, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    NBS_STATUS_1_10d::NBS_MasterCylinderStrokeStType nbs_mastercylinderstrokest(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Driver Input Rod Stroke', 'offset': -5.0, 'precision': 0.015625, 'len': 12, 'name': 'NBS_MasterCylinderStroke', 'is_signed_var': False, 'physical_range': '[-5|58.984375]', 'bit': 24, 'type': 'double', 'order': 'intel', 'physical_unit': 'mm'}
    double nbs_mastercylinderstroke(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2,\xad15, 0, 1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'Livecounter_NBS', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
    int livecounter_nbs(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'Checksum_NBS', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
    int checksum_nbs(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Nbooster Diag flag', 'enum': {0: 'NBSDIAGACTIVE_NBOOSTER_DIAGOSIS_INACTIVE', 1: 'NBSDIAGACTIVE_NBOOSTER_DIAGOSIS_ACTIVE'}, 'precision': 1.0, 'len': 1, 'name': 'NBSDiagActive', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 23, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    NBS_STATUS_1_10d::NBSDiagActiveType nbsdiagactive(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Status of driver door', 'enum': {0: 'NBS_BRAKEPEDALAPPLIED_VALIDITY_NOTINITIALIZED', 1: 'NBS_BRAKEPEDALAPPLIED_VALIDITY_NORMAL', 2: 'NBS_BRAKEPEDALAPPLIED_VALIDITY_FAULTY', 3: 'NBS_BRAKEPEDALAPPLIED_VALIDITY_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'NBS_BrakePedalApplied_Validity', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 44, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    NBS_STATUS_1_10d::NBS_BrakePedalApplied_ValidityType nbs_brakepedalapplied_validity(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Indicates the autoimatic driving is actived or not', 'enum': {0: 'NBS_BRAKEPEDALAPPLIEDST_NO_PRESSED', 1: 'NBS_BRAKEPEDALAPPLIEDST_PRESSED'}, 'precision': 1.0, 'len': 1, 'name': 'NBS_BrakePedalAppliedSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 22, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    NBS_STATUS_1_10d::NBS_BrakePedalAppliedStType nbs_brakepedalappliedst(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Status of driver door', 'enum': {0: 'NBS_DRIVERINPUTRODSTROKESTROKEST_NOTINITIALIZED', 1: 'NBS_DRIVERINPUTRODSTROKESTROKEST_NORMAL', 2: 'NBS_DRIVERINPUTRODSTROKESTROKEST_FAULTY', 3: 'NBS_DRIVERINPUTRODSTROKESTROKEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'NBS_DriverInputRodStrokeStrokeSt', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    int nbs_driverinputrodstrokestrokest(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Driver Input Rod Stroke', 'offset': -5.0, 'precision': 0.015625, 'len': 12, 'name': 'NBS_DriverInputRodStroke', 'is_signed_var': False, 'physical_range': '[-5|58.984375]', 'bit': 8, 'type': 'double', 'order': 'intel', 'physical_unit': 'mm'}
    double nbs_driverinputrodstroke(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'Acceleration pedal position', 'offset': 0.0, 'precision': 0.1, 'len': 10, 'name': 'NBS_BrkPelPosAct', 'is_signed_var': False, 'physical_range': '[0|100]', 'bit': 46, 'type': 'double', 'order': 'intel', 'physical_unit': '%'}
    double nbs_brkpelposact(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'NBS_BrkPelPosAct valid data', 'enum': {0: 'NBS_BRKPELPOSACTVD_INACTIVE', 1: 'NBS_BRKPELPOSACTVD_ACTIVE'}, 'precision': 1.0, 'len': 1, 'name': 'NBS_BrkPelPosActVD', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 56, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    NBS_STATUS_1_10d::NBS_BrkPelPosActVDType nbs_brkpelposactvd(const std::uint8_t* bytes, int length) const;

    // config detail: {'name': 'NBS_FaultSt', 'enum': {0: 'NBS_FAULTST_NO_FAULT', 1: 'NBS_FAULTST_FAULT_LEVEL_1', 2: 'NBS_FAULTST_FAULT_LEVEL_2', 3: 'NBS_FAULTST_FAULT_LEVEL_3', 4: 'NBS_FAULTST_FAULT_LEVEL_4', 5: 'NBS_FAULTST_FAULT_LEVEL_5', 6: 'NBS_FAULTST_FAULT_LEVEL_6', 7: 'NBS_FAULTST_RESERVED'}, 'precision': 1.0, 'len': 3, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 57, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
    NBS_STATUS_1_10d::NBS_FaultStType nbs_faultst(const std::uint8_t* bytes, int length) const;

    // config detail: {'description': 'This signal is target brake pedal position request for wheel brake.', 'enum': {16383: 'NBS_BRKTORQACT_PERCENT_INVALID'}, 'precision': 0.01, 'len': 14, 'name': 'NBS_BrkTorqAct_percent', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|100]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': '%'}
    double nbs_brktorqact_percent(const std::uint8_t* bytes, int length) const;

private:
    mutable double _driver_override_start_time = 0.0;
    mutable bool _is_first_driver_override = false;
};

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
