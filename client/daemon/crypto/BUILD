#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: MIT
#

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "crypto_lib",
    srcs = ["crypto.cc"],
    hdrs = [
        "constants.hpp",
        "crypto.hpp",
    ],
    linkstatic = True,
    visibility = ["//client/daemon:__subpackages__"],
    deps = [
        "//asphr/asphr:asphr_lib",
        "//asphr/schema:asyncinvitation_proto_cc",
        "//asphr/schema:daemon_cc_grpc",
        "//asphr/schema:message_proto_cc",
        "//asphr/schema:server_cc_grpc",
        "//asphr/third_party/libsodium",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_test(
    name = "crypto_test",
    srcs = [
        "crypto_test.cc",
    ],
    deps = [
        ":crypto_lib",
        "//client/daemon/identifier",
        "@com_google_googletest//:gtest_main",
    ],
)
