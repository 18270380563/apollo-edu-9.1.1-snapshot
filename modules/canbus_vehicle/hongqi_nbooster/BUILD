load("//tools:apollo_package.bzl", "apollo_cc_binary", "apollo_cc_library", "apollo_cc_test", "apollo_component", "apollo_package")
load("//tools:cpplint.bzl", "cpplint")

package(default_visibility = ["//visibility:public"])

CANBUS_COPTS = ["-DMODULE_NAME=\\\"canbus\\\""]

apollo_cc_library(
    name = "apollo_canbus_vehicle_hongqi_nbooster",
    srcs = [
        "hongqi_nbooster_controller.cc",
        "hongqi_nbooster_message_manager.cc",
        "hongqi_nbooster_vehicle_factory.cc",
        "protocol/abs_1_b4.cc",
        "protocol/cds_1_10a.cc",
        "protocol/cds_2_10b.cc",
        "protocol/epb_1_104.cc",
        "protocol/hcu_14_248.cc",
        "protocol/hcu_torque_status_tonbs_191.cc",
        "protocol/nbs_status_1_10d.cc",
        "protocol/sas_1_96.cc",
        "protocol/svb_1_108.cc",
        "protocol/svb_2_1a0.cc",
        "protocol/svb_3_2c0.cc",
        "protocol/svb_4_2c2.cc",
        "protocol/svb_nbs_req_109.cc",
        "protocol/hcu_4_122.cc",
    ],
    hdrs = [
        "hongqi_nbooster_controller.h",
        "hongqi_nbooster_message_manager.h",
        "hongqi_nbooster_vehicle_factory.h",
        "protocol/abs_1_b4.h",
        "protocol/cds_1_10a.h",
        "protocol/cds_2_10b.h",
        "protocol/epb_1_104.h",
        "protocol/hcu_14_248.h",
        "protocol/hcu_torque_status_tonbs_191.h",
        "protocol/nbs_status_1_10d.h",
        "protocol/sas_1_96.h",
        "protocol/svb_1_108.h",
        "protocol/svb_2_1a0.h",
        "protocol/svb_3_2c0.h",
        "protocol/svb_4_2c2.h",
        "protocol/svb_nbs_req_109.h",
        "protocol/hcu_4_122.h",
    ],
    copts = CANBUS_COPTS,
    deps = [
        "//modules/canbus:apollo_canbus",
        "//modules/canbus/proto:canbus_conf_cc_proto",
        "//modules/canbus/proto:vehicle_parameter_cc_proto",
        "//modules/canbus_vehicle/hongqi_nbooster/proto:hongqi_nbooster_proto",
        "//modules/common/adapters:adapter_gflags",
        "//modules/common/status",
        "//modules/common_msgs/basic_msgs:error_code_cc_proto",
        "//modules/common_msgs/chassis_msgs:chassis_cc_proto",
        "//modules/common_msgs/control_msgs:control_cmd_cc_proto",
        "//modules/drivers/canbus:apollo_drivers_canbus",
    ],
)

apollo_component(
    name = "libhongqi_nbooster_vehicle_factory_lib.so",
    deps = [":apollo_canbus_vehicle_hongqi_nbooster"],
)

filegroup(
    name = "runtime_data",
    srcs = glob([
        "testdata/**",
    ]),
)

apollo_package()
cpplint()
