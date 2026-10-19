
load("//tools/install:install.bzl", "install", "install_src_files")

package(
    default_visibility = ["//visibility:public"],
)


install(
    name = "mock_install",
    type = "neo",
    deps = ["//modules/control/control_component:install"],
)


install_src_files(
    name = "mock_install_src",
    type = "neo",
    deps = ["//modules/control/control_component:install_src"],
)
