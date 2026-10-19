load("//third_party/eigen3:init.bzl", apollo_neo_3rd_eigen3_repo = "init")
load("//third_party/grpc:init.bzl", apollo_neo_3rd_grpc_repo = "init")
load("//third_party/bazel_skylib:init.bzl", apollo_neo_3rd_bazel_skylib_repo = "init")
load("//third_party/gpus:init.bzl", apollo_neo_3rd_gpus_repo = "init")
load("//third_party/cpplint:init.bzl", apollo_neo_3rd_cpplint_repo = "init")
load("//third_party/rules_python:init.bzl", apollo_neo_3rd_rules_python_repo = "init")
load("//third_party/protobuf:init.bzl", apollo_neo_3rd_protobuf_repo = "init")
load("//third_party/nlohmann_json:init.bzl", apollo_neo_3rd_nlohmann_json_repo = "init")
load("//third_party/py:init.bzl", apollo_neo_3rd_py_repo = "init")
load("//third_party/gtest:init.bzl", apollo_neo_3rd_gtest_repo = "init")
load("//third_party/rules_proto:init.bzl", apollo_neo_3rd_rules_proto_repo = "init")
def clean_dep(dep):
    return str(Label(dep))
def apollo_neo_3rd_osqp_repo():
    native.new_local_repository(
        name = "osqp",
        build_file = clean_dep("//dev/bazel:3rd-osqp.BUILD"),
        path = "/opt/apollo/neo/packages/3rd-osqp/latest",
    )
def apollo_neo_3rd_boost_repo():
    native.new_local_repository(
        name = "boost",
        build_file = clean_dep("//dev/bazel:3rd-boost.BUILD"),
        path = "/opt/apollo/neo/packages/3rd-boost/latest",
    )
def system_libncurses5_dev_repo():
    native.new_local_repository(
        name = "ncurses5",
        build_file = clean_dep("//dev/bazel:libncurses5-dev.BUILD"),
        path = "/usr/include",
    )
def apollo_neo_3rd_absl_repo():
    native.new_local_repository(
        name = "com_google_absl",
        build_file = clean_dep("//dev/bazel:3rd-absl.BUILD"),
        path = "/opt/apollo/neo/packages/3rd-absl/latest",
    )
def apollo_neo_3rd_glog_repo():
    native.new_local_repository(
        name = "com_github_google_glog",
        build_file = clean_dep("//dev/bazel:3rd-glog.BUILD"),
        path = "/opt/apollo/neo/packages/3rd-glog/latest",
    )
def apollo_neo_3rd_fastrtps_repo():
    native.new_local_repository(
        name = "fastrtps",
        build_file = clean_dep("//dev/bazel:3rd-fastrtps.BUILD"),
        path = "/opt/apollo/neo/packages/3rd-fastrtps/latest",
    )
def system_gperftools_repo():
    native.new_local_repository(
        name = "gperftools",
        build_file = clean_dep("//dev/bazel:gperftools.BUILD"),
        path = "/usr/include",
    )
def system_libunwind_dev_repo():
    native.new_local_repository(
        name = "libunwind-dev",
        build_file = clean_dep("//dev/bazel:libunwind-dev.BUILD"),
        path = "/usr/include",
    )
def system_libuuid1_repo():
    native.new_local_repository(
        name = "uuid",
        build_file = clean_dep("//dev/bazel:libuuid1.BUILD"),
        path = "/usr/include",
    )
def system_libsqlite3_dev_repo():
    native.new_local_repository(
        name = "sqlite3",
        build_file = clean_dep("//dev/bazel:libsqlite3-dev.BUILD"),
        path = "/usr/include",
    )
def apollo_neo_3rd_gflags_repo():
    native.new_local_repository(
        name = "com_github_gflags_gflags",
        build_file = clean_dep("//dev/bazel:3rd-gflags.BUILD"),
        path = "/opt/apollo/neo/packages/3rd-gflags/latest",
    )
def system_libtinyxml2_dev_repo():
    native.new_local_repository(
        name = "tinyxml2",
        build_file = clean_dep("//dev/bazel:libtinyxml2-dev.BUILD"),
        path = "/usr/include",
    )
def system_bvar_repo():
    native.new_local_repository(
        name = "bvar",
        build_file = clean_dep("//dev/bazel:bvar.BUILD"),
        path = "/usr/include",
    )
def apollo_neo_common_msgs_repo():
    native.new_local_repository(
        name = "common-msgs",
        build_file = clean_dep("//dev/bazel:common-msgs.BUILD"),
        path = "/opt/apollo/neo",
    )
def apollo_neo_cyber_repo():
    native.new_local_repository(
        name = "cyber",
        build_file = clean_dep("//dev/bazel:cyber.BUILD"),
        path = "/opt/apollo/neo",
    )
def apollo_neo_common_repo():
    native.new_local_repository(
        name = "common",
        build_file = clean_dep("//dev/bazel:common.BUILD"),
        path = "/opt/apollo/neo",
    )
def init_deps():
    apollo_neo_3rd_eigen3_repo()
    apollo_neo_3rd_grpc_repo()
    apollo_neo_3rd_bazel_skylib_repo()
    apollo_neo_3rd_gpus_repo()
    apollo_neo_3rd_cpplint_repo()
    apollo_neo_3rd_rules_python_repo()
    apollo_neo_3rd_protobuf_repo()
    apollo_neo_3rd_nlohmann_json_repo()
    apollo_neo_3rd_py_repo()
    apollo_neo_3rd_gtest_repo()
    apollo_neo_3rd_rules_proto_repo()
    apollo_neo_3rd_osqp_repo()
    apollo_neo_3rd_boost_repo()
    system_libncurses5_dev_repo()
    apollo_neo_3rd_absl_repo()
    apollo_neo_3rd_glog_repo()
    apollo_neo_3rd_fastrtps_repo()
    system_gperftools_repo()
    system_libunwind_dev_repo()
    system_libuuid1_repo()
    system_libsqlite3_dev_repo()
    apollo_neo_3rd_gflags_repo()
    system_libtinyxml2_dev_repo()
    system_bvar_repo()
    apollo_neo_common_msgs_repo()
    apollo_neo_cyber_repo()
    apollo_neo_common_repo()