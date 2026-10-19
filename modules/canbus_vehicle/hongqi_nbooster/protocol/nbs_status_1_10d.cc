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

 #include "modules/canbus_vehicle/hongqi_nbooster/protocol/nbs_status_1_10d.h"
 
 #include "glog/logging.h"
 
 #include "modules/drivers/canbus/common/byte.h"
 #include "modules/drivers/canbus/common/canbus_consts.h"

 namespace apollo {
 namespace canbus {
 namespace hongqi_nbooster {

using ::apollo::drivers::canbus::Byte;

Nbsstatus110d::Nbsstatus110d() {}
const int32_t Nbsstatus110d::ID = 0x10D;
const double max_cyl_stroke = 30.0;

void Nbsstatus110d::Parse(const std::uint8_t* bytes, int32_t length,
    Hongqi_nbooster* chassis) const {
    // //chassis->mutable_brake()->set_brake_output(nbs_brkpelposact(bytes, length));
    // double brake_percentage = 
    //         std::min(100.0, std::max(0.0, nbs_mastercylinderstroke(bytes, length) / max_cyl_stroke));
    // chassis->mutable_brake()->set_brake_output(nbs_brktorqact_percent(bytes, length));
    // // chassis->mutable_brake()->set_brake_output(brake_percentage);
    // chassis->mutable_brake()->set_master_cylinder_stroke(brake_percentage * max_cyl_stroke);
    // chassis->mutable_brake()->set_is_brake_by_driver(nbs_brakepedalappliedst(bytes, length));
    // if (nbs_brkautocontrol_st(bytes, length) == 1) {
    //     chassis->mutable_handshake()->set_is_esp_online(true);
    // } else {
    //     chassis->mutable_handshake()->set_is_esp_online(false);
    // }
    
    // if (nbs_brkpelposact(bytes, length) > 0.05) {
    //     if (_is_first_driver_override && cybertron::Time::Now().ToSecond() - _driver_override_start_time > 0.1) {
    //         chassis->mutable_brake()->set_driver_override(true);
    //     } else if (!_is_first_driver_override) {
    //         _is_first_driver_override = true;
    //         _driver_override_start_time = cybertron::Time::Now().ToSecond();
    //     } 
    // } else {
    //     chassis->mutable_brake()->set_driver_override(false);
    //     _is_first_driver_override = false;
    // }

    // chassis->mutable_brake()->set_driver_override(nbs_decoverride_status(bytes, length));
    // chassis->mutable_safety()->set_nbs_fault_status(nbs_faultst(bytes, length));
    // chassis->mutable_safety()->set_nbs_brkautocontrol_status(nbs_brkautocontrol_st(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_nbs_brktorqact_percent(nbs_brktorqact_percent(bytes, length));
    // chassis->mutable_nbs_status_1_10d()->set_nbs_decoverride_status(nbs_decoverride_status(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_nbs_brkautocontrol_st(nbs_brkautocontrol_st(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_nbs_mastercylinderstrokest(nbs_mastercylinderstrokest(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_nbs_mastercylinderstroke(nbs_mastercylinderstroke(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_livecounter_nbs(livecounter_nbs(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_checksum_nbs(checksum_nbs(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_nbsdiagactive(nbsdiagactive(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_nbs_brakepedalapplied_validity(nbs_brakepedalapplied_validity(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_nbs_brakepedalappliedst(nbs_brakepedalappliedst(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_nbs_driverinputrodstrokestrokest(nbs_driverinputrodstrokestrokest(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_nbs_driverinputrodstroke(nbs_driverinputrodstroke(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_nbs_brkpelposact(nbs_brkpelposact(bytes, length));
    chassis->mutable_nbs_status_1_10d()->set_nbs_brkpelposactvd(nbs_brkpelposactvd(bytes, length));
    // chassis->mutable_nbs_status_1_10d()->set_nbs_faultst(nbs_faultst(bytes, length));
}

// config detail: {'name': 'nbs_decoverride_status', 'enum': {0: 'NBS_DECOVERRIDE_STATUS_NOT_OVERRIDE', 1: 'NBS_DECOVERRIDE_STATUS_OVERRIDE'}, 'precision': 1.0, 'len': 1, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 40, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
NBS_STATUS_1_10d::NBS_DecOverride_StatusType Nbsstatus110d::nbs_decoverride_status(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 5); 
    int32_t x = t0.get_byte(0, 1); 
    
    NBS_STATUS_1_10d::NBS_DecOverride_StatusType ret = static_cast<NBS_STATUS_1_10d::NBS_DecOverride_StatusType>(x);
    return ret;
}

// config detail: {'name': 'nbs_brkautocontrol_st', 'enum': {0: 'NBS_BRKAUTOCONTROL_ST_MANUEL_MODE', 1: 'NBS_BRKAUTOCONTROL_ST_BRKPELPOS_CONTROL_ACTIVE', 2: 'NBS_BRKAUTOCONTROL_ST_TARDEC_CONTROL_ACTIVE', 3: 'NBS_BRKAUTOCONTROL_ST_ERROR_MODE'}, 'precision': 1.0, 'len': 2, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 38, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
NBS_STATUS_1_10d::NBS_BrkAutoControl_StType Nbsstatus110d::nbs_brkautocontrol_st(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 4); 
    int32_t x = t0.get_byte(6, 2); 

    NBS_STATUS_1_10d::NBS_BrkAutoControl_StType ret = static_cast<NBS_STATUS_1_10d::NBS_BrkAutoControl_StType>(x);
    return ret;
}

// config detail: {'description': 'Status of driver door', 'enum': {0: 'NBS_MASTERCYLINDERSTROKEST_NOTINITIALIZED', 1: 'NBS_MASTERCYLINDERSTROKEST_NORMAL', 2: 'NBS_MASTERCYLINDERSTROKEST_FAULTY', 3: 'NBS_MASTERCYLINDERSTROKEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'nbs_mastercylinderstrokest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 36, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
NBS_STATUS_1_10d::NBS_MasterCylinderStrokeStType Nbsstatus110d::nbs_mastercylinderstrokest(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(4, 2);

    NBS_STATUS_1_10d::NBS_MasterCylinderStrokeStType ret = static_cast<NBS_STATUS_1_10d::NBS_MasterCylinderStrokeStType>(x);
    return ret;
}

// config detail: {'description': 'Driver Input Rod Stroke', 'offset': -5.0, 'precision': 0.015625, 'len': 12, 'name': 'nbs_mastercylinderstroke', 'is_signed_var': False, 'physical_range': '[-5|58.984375]', 'bit': 24, 'type': 'double', 'order': 'intel', 'physical_unit': 'mm'}
double Nbsstatus110d::nbs_mastercylinderstroke(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 4);
    int32_t x = t0.get_byte(0, 4);
    Byte t1(bytes + 3);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;

    double ret = x * 0.015625 + -5.000000;
    return ret;
}

// config detail: {'description': 'The Livecounter is used to check the signal consistency of the messages . Ifthe signals are consecutive, the LiveCounter should also be sequential inthe way 0, 1, 2,\xad15, 0, 1\xad', 'offset': 0.0, 'precision': 1.0, 'len': 4, 'name': 'livecounter_nbs', 'is_signed_var': False, 'physical_range': '[0|15]', 'bit': 60, 'type': 'int', 'order': 'intel', 'physical_unit': 'Pulse'}
int Nbsstatus110d::livecounter_nbs(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(4, 4);

    int ret = x;
    return ret;
}

// config detail: {'description': 'The checksum is used to check whether the data from the CPU buffer areshifted without transmission errors to the CAN communication controllerbuffer and vice versa.', 'offset': 0.0, 'precision': 1.0, 'len': 8, 'name': 'checksum_nbs', 'is_signed_var': False, 'physical_range': '[0|255]', 'bit': 0, 'type': 'int', 'order': 'intel', 'physical_unit': ''}
int Nbsstatus110d::checksum_nbs(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 0);
    int32_t x = t0.get_byte(0, 8);

    int ret = x;
    return ret;
}

// config detail: {'description': 'Nbooster Diag flag', 'enum': {0: 'NBSDIAGACTIVE_NBOOSTER_DIAGOSIS_INACTIVE', 1: 'NBSDIAGACTIVE_NBOOSTER_DIAGOSIS_ACTIVE'}, 'precision': 1.0, 'len': 1, 'name': 'nbsdiagactive', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 23, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
NBS_STATUS_1_10d::NBSDiagActiveType Nbsstatus110d::nbsdiagactive(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(7, 1);

    NBS_STATUS_1_10d::NBSDiagActiveType ret = static_cast<NBS_STATUS_1_10d::NBSDiagActiveType>(x);
    return ret;
}

// config detail: {'description': 'Status of driver door', 'enum': {0: 'NBS_BRAKEPEDALAPPLIED_VALIDITY_NOTINITIALIZED', 1: 'NBS_BRAKEPEDALAPPLIED_VALIDITY_NORMAL', 2: 'NBS_BRAKEPEDALAPPLIED_VALIDITY_FAULTY', 3: 'NBS_BRAKEPEDALAPPLIED_VALIDITY_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'nbs_brakepedalapplied_validity', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 44, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
NBS_STATUS_1_10d::NBS_BrakePedalApplied_ValidityType Nbsstatus110d::nbs_brakepedalapplied_validity(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 5);
    int32_t x = t0.get_byte(4, 2);

    NBS_STATUS_1_10d::NBS_BrakePedalApplied_ValidityType ret = static_cast<NBS_STATUS_1_10d::NBS_BrakePedalApplied_ValidityType>(x);
    return ret;
}

// config detail: {'description': 'Indicates the autoimatic driving is actived or not', 'enum': {0: 'NBS_BRAKEPEDALAPPLIEDST_NO_PRESSED', 1: 'NBS_BRAKEPEDALAPPLIEDST_PRESSED'}, 'precision': 1.0, 'len': 1, 'name': 'nbs_brakepedalappliedst', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 22, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
NBS_STATUS_1_10d::NBS_BrakePedalAppliedStType Nbsstatus110d::nbs_brakepedalappliedst(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(6, 1);

    NBS_STATUS_1_10d::NBS_BrakePedalAppliedStType ret = static_cast<NBS_STATUS_1_10d::NBS_BrakePedalAppliedStType>(x);
    return ret;
}

// config detail: {'description': 'Status of driver door', 'enum': {0: 'NBS_DRIVERINPUTRODSTROKESTROKEST_NOTINITIALIZED', 1: 'NBS_DRIVERINPUTRODSTROKESTROKEST_NORMAL', 2: 'NBS_DRIVERINPUTRODSTROKESTROKEST_FAULTY', 3: 'NBS_DRIVERINPUTRODSTROKESTROKEST_RESERVED'}, 'precision': 1.0, 'len': 2, 'name': 'nbs_driverinputrodstrokestrokest', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|3]', 'bit': 20, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
int Nbsstatus110d::nbs_driverinputrodstrokestrokest(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(4, 2);
    
    int ret = x;
    return ret;
}

// config detail: {'description': 'Driver Input Rod Stroke', 'offset': -5.0, 'precision': 0.015625, 'len': 12, 'name': 'nbs_driverinputrodstroke', 'is_signed_var': False, 'physical_range': '[-5|58.984375]', 'bit': 8, 'type': 'double', 'order': 'intel', 'physical_unit': 'mm'}
double Nbsstatus110d::nbs_driverinputrodstroke(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(0, 4);
    Byte t1(bytes + 1);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;

    double ret = x * 0.015625 + -5.000000;
    return ret;
}

// config detail: {'description': 'Acceleration pedal position', 'offset': 0.0, 'precision': 0.1, 'len': 10, 'name': 'nbs_brkpelposact', 'is_signed_var': False, 'physical_range': '[0|100]', 'bit': 46, 'type': 'double', 'order': 'intel', 'physical_unit': '%'}
double Nbsstatus110d::nbs_brkpelposact(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 6);
    int32_t x = t0.get_byte(0, 8);
    Byte t1(bytes + 5);
    int32_t t = t1.get_byte(6, 2);
    x <<= 2;
    x |= t;

    double ret = x * 0.100000;
    return ret;
}

// config detail: {'description': 'NBS_BrkPelPosAct valid data', 'enum': {0: 'NBS_BRKPELPOSACTVD_INACTIVE', 1: 'NBS_BRKPELPOSACTVD_ACTIVE'}, 'precision': 1.0, 'len': 1, 'name': 'nbs_brkpelposactvd', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|1]', 'bit': 56, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
NBS_STATUS_1_10d::NBS_BrkPelPosActVDType Nbsstatus110d::nbs_brkpelposactvd(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(0, 1);

    NBS_STATUS_1_10d::NBS_BrkPelPosActVDType ret = static_cast<NBS_STATUS_1_10d::NBS_BrkPelPosActVDType>(x);
    return ret;
}

// config detail: {'name': 'nbs_faultst', 'enum': {0: 'NBS_FAULTST_NO_FAULT', 1: 'NBS_FAULTST_FAULT_LEVEL_1', 2: 'NBS_FAULTST_FAULT_LEVEL_2', 3: 'NBS_FAULTST_FAULT_LEVEL_3', 4: 'NBS_FAULTST_FAULT_LEVEL_4', 5: 'NBS_FAULTST_FAULT_LEVEL_5', 6: 'NBS_FAULTST_FAULT_LEVEL_6', 7: 'NBS_FAULTST_RESERVED'}, 'precision': 1.0, 'len': 3, 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|7]', 'bit': 57, 'type': 'enum', 'order': 'intel', 'physical_unit': ''}
NBS_STATUS_1_10d::NBS_FaultStType Nbsstatus110d::nbs_faultst(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 7);
    int32_t x = t0.get_byte(1, 3);

    NBS_STATUS_1_10d::NBS_FaultStType ret = static_cast<NBS_STATUS_1_10d::NBS_FaultStType>(x);
    return  ret;
}

// config detail: {'description': 'This signal is target brake pedal position request for wheel brake.', 'enum': {16383: 'NBS_BRKTORQACT_PERCENT_INVALID'}, 'precision': 0.01, 'len': 14, 'name': 'NBS_BrkTorqAct_percent', 'is_signed_var': False, 'offset': 0.0, 'physical_range': '[0|100]', 'bit': 8, 'type': 'enum', 'order': 'intel', 'physical_unit': '%'}
double Nbsstatus110d::nbs_brktorqact_percent(const std::uint8_t* bytes, int length) const {
    Byte t0(bytes + 2);
    int32_t x = t0.get_byte(0, 6);
    Byte t1(bytes + 1);
    int32_t t = t1.get_byte(0, 8);
    x <<= 8;
    x |= t;
    // int ret = static_cast<int>(x);
    double ret = x * 0.01;
    return ret;
}
}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
