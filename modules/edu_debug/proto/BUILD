## Debug dump message used by planning/control instrumentation.
##
## Workspace-local package (not registered in tools/package/dynamic_deps.bzl),
## so the apollo proto macro is not used: it would emit a shared library
## (lib<name>_mep_bin.so) that has no install rule and therefore breaks the
## install step of every module that links it.  Plain rules produce a static
## library that is linked straight into libapollo_{control,planning}_lib.so.
## The message has no dependency on common_msgs, so no proto deps are needed.

load("@rules_proto//proto:defs.bzl", "proto_library")

package(default_visibility = ["//visibility:public"])

proto_library(
    name = "debug_dump_pt",
    srcs = ["debug_dump.proto"],
)

cc_proto_library(
    name = "debug_dump_proto",
    deps = [":debug_dump_pt"],
)
