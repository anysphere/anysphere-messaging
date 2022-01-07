load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "scheduler_lib",
    srcs = ["as_scheduler.cc"],
    hdrs = ["as_scheduler.hpp"],
    deps = ["@com_github_daniele77_cli//:cli_header_lib"],
)

cc_binary(
    name = "anysphere-cli",
    srcs = [
        "as_cli.cc",
        "as_cli.h",
    ],
    linkopts = ["-lpthread"],
    deps = [
        "//asphr:asphr_lib",
        "//client/client_lib",
        "@com_github_daniele77_cli//:cli_header_lib",
        ":scheduler_lib",
    ],
)

cc_test(
    name = "as_cli_test",
    size = "small",
    srcs = ["as_cli_test.cc"],
    deps = ["@com_google_googletest//:gtest_main"],
)
