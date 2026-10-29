## Offline export of the edu_debug channel from a cyber record.
##
## Plain bazel rules: this package is workspace-local and not registered in
## tools/package/dynamic_deps.bzl (see ../BUILD for details).
##
## The default gcc toolchain used by `bazel build --config=opt` lacks the
## non-standard linker search paths that buildtool's CUDA crosstool provides,
## so the prebuilt libs cyber depends on (bvar, fastrtps/fastcdr) and glog are
## resolved with explicit -L/-l linkopts here.

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "export_debug_record",
    srcs = ["export_debug_record.cc"],
    copts = ["-DMODULE_NAME=\\\"edu_debug\\\""],
    linkopts = [
        "-L/usr/local/lib",
        "-L/opt/apollo/neo/lib/3rd-fastrtps",
        "-lbvar",
        "-lglog",
        "-lgflags",
    ],
    deps = [
        "//modules/edu_debug/proto:debug_dump_proto",
        "@cyber//:cyber",
        "@cyber//:cyber_Ccyber",
        "@fastrtps//:fastrtps",
    ],
)
