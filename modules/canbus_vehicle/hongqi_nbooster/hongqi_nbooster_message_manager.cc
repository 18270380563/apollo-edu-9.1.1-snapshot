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

#include "modules/canbus_vehicle/hongqi_nbooster/hongqi_nbooster_message_manager.h"

#include "modules/canbus_vehicle/hongqi_nbooster/protocol/cds_1_10a.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/hcu_torque_status_tonbs_191.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/nbs_status_1_10d.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/svb_1_108.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/svb_2_1a0.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/svb_3_2c0.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/svb_4_2c2.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/svb_nbs_req_109.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/abs_1_b4.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/hcu_14_248.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/sas_1_96.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/epb_1_104.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/cds_2_10b.h"
#include "modules/canbus_vehicle/hongqi_nbooster/protocol/hcu_4_122.h"



namespace apollo {
namespace canbus {
namespace hongqi_nbooster {

Hongqi_nboosterMessageManager::Hongqi_nboosterMessageManager() {
  // Control Messages
  AddSendProtocolData<Svbnbsreq109, true>();
  AddSendProtocolData<Svb42c2, true>();
  AddSendProtocolData<Svb32c0, true>();
  AddSendProtocolData<Svb21a0, true>();
  AddSendProtocolData<Svb1108, true>();

  // Report Messages
  AddRecvProtocolData<Cds110a, true>();
  AddRecvProtocolData<Hcutorquestatustonbs191, true>();
  AddRecvProtocolData<Nbsstatus110d, true>();
  AddRecvProtocolData<Abs1b4, true>();

  AddRecvProtocolData<Hcu14248, true>();
  AddRecvProtocolData<Sas196, true>();
  AddRecvProtocolData<Epb1104, true>();
  AddRecvProtocolData<Cds210b, true>();

  AddRecvProtocolData<Hcu4122, true>();
  
}

Hongqi_nboosterMessageManager::~Hongqi_nboosterMessageManager() {}

}  // namespace hongqi_nbooster
}  // namespace canbus
}  // namespace apollo
