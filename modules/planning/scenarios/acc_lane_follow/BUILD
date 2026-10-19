load("//tools:cpplint.bzl", "cpplint")
load("//tools:apollo_package.bzl", "apollo_package", "apollo_plugin")

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "runtime_files",
    srcs = glob(["conf/**"]),
)

apollo_plugin(
    name = "libacc_lane_follow_scenario.so",
    srcs = ["acc_lane_follow_scenario.cc"],
    hdrs = ["acc_lane_follow_scenario.h"],
    copts = ["-DMODULE_NAME=\\\"planning\\\""],
    description = ":plugins.xml",
    deps = [
        "//cyber",
        "//modules/planning/planning_base:apollo_planning_planning_base",
        "//modules/planning/planning_interface_base:apollo_planning_planning_interface_base",
    ],
)

apollo_package()

cpplint()
