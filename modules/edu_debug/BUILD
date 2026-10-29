## Shared debug dumper used by the planning and control components.
##
## This package is workspace-local and is NOT registered in the buildtool
## generated tools/package/dynamic_deps.bzl (only packages installed under
## /opt/apollo/neo appear there).  The apollo_* macros therefore treat it as a
## prebuilt binary package and drop its deps (apollo_package.bzl:
## _auto_padding_deps returns an empty list when source_pkg == None), which
## breaks the build.  Plain bazel rules are used instead, with the real
## external labels that tools/package/dynamic_deps.bzl would have produced:
##   //cyber  ->  @cyber//:cyber          (headers only, strip_include_prefix)
##              + @cyber//:cyber_Ccyber   (libcyber.so and its deps)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "edu_debug_logger",
    srcs = ["debug_logger.cc"],
    hdrs = [
        "debug_logger.h",
        "sha1.h",
    ],
    copts = ["-DMODULE_NAME=\\\"edu_debug\\\""],
    deps = [
        "//modules/edu_debug/proto:debug_dump_proto",
        "@com_github_gflags_gflags//:gflags",
        "@cyber//:cyber",
        "@cyber//:cyber_Ccyber",
        "@fastrtps//:fastrtps",
    ],
)
