#pragma once

#include "cyber/plugin_manager/plugin_manager.h"
#include "modules/planning/planning_interface_base/scenario_base/scenario.h"

namespace apollo {
namespace planning {

class AccLaneFollowScenario : public Scenario {
 public:
  ScenarioContext* GetContext() override { return nullptr; }

  bool IsTransferable(const Scenario* other_scenario,
                      const Frame& frame) override;
};

CYBER_PLUGIN_MANAGER_REGISTER_PLUGIN(apollo::planning::AccLaneFollowScenario,
                                     Scenario)

}  // namespace planning
}  // namespace apollo
