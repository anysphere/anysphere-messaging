#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

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
    visibility = ["//client:__pkg__"],
    deps = [
        "//client/client_lib",
        "//asphr/asphr:asphr_lib",
        "//asphr/schema:daemon_cc_grpc",
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
