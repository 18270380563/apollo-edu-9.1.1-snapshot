load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "sqlite3",
    includes = ["."],
    linkopts = [
        "-lsqlite3",
    ],
    visibility = ["//visibility:public"],
)