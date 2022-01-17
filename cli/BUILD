load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "asphr",
    srcs = [
        "as_cli.cc",
        "as_cli.hpp",
        "friend_struct.cc",
        "inbox.cc",
        "inbox.hpp",
        "message_struct.cc",
        "parse_command_line.hpp",
        "profile_struct.cc",
    ],
    linkopts = ["-lpthread"],
    linkstatic = True,
    visibility = ["//:__pkg__"],
    deps = [
        "//asphr:asphr_lib",
        "//client/client_lib",
        "//schema:daemon_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_test(
    name = "as_cli_test",
    size = "small",
    srcs = ["as_cli_test.cc"],
    linkstatic = True,
    deps = ["@com_google_googletest//:gtest_main"],
)
