load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "ssl",
  # srcs = glob([
  #   "lib/*"
  # ]),
  includes=["include"],
  hdrs = glob([
    "include/openssl/**/*"
  ]),
  linkopts = [
    "-lssl",
  ],
  strip_include_prefix = "include",
)
