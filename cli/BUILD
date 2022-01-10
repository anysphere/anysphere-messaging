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
        "as_cli.hpp",
        "root_menu.hpp",
        "friend_struct.cc",
        "message_struct.cc",
        "profile_struct.cc",
    ],
    linkopts = ["-lpthread"],
    deps = [
        ":scheduler_lib",
        "//asphr:asphr_lib",
        "//client/client_lib",
        "//schema:daemon_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_daniele77_cli//:cli_header_lib",
    ],
)

cc_test(
    name = "as_cli_test",
    size = "small",
    srcs = ["as_cli_test.cc"],
    deps = ["@com_google_googletest//:gtest_main"],
)
