#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

load("@rules_cc//cc:defs.bzl", "cc_binary")

# public visibility needed for integration_tests
package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "base_lib",
    srcs = [
        "config.cc",
        "crypto.cc",
        "friend.cc",
        "msgstore.cc",
    ],
    hdrs = [
        "config.hpp",
        "constants.hpp",
        "crypto.hpp",
        "friend.hpp",
        "msgstore.hpp",
    ],
    linkstatic = True,
    deps = [
        "//client_lib",
        "@asphr//asphr:asphr_lib",
        "@asphr//pir/fast_pir:fast_pir_lib",
        "@asphr//schema:message_proto_cc",
        "@asphr//third_party/libsodium",
    ],
)

cc_test(
    name = "crypto_test",
    size = "small",
    srcs = ["crypto_test.cc"],
    deps = [
        ":base_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "daemon_lib",
    srcs = [
        "daemon_rpc.cc",
    ],
    hdrs = [
        "daemon_rpc.hpp",
    ],
    linkstatic = True,
    deps = [
        ":base_lib",
        "@asphr//asphr:asphr_lib",
        "@asphr//pir/fast_pir:fast_pir_lib",
        "@asphr//schema:daemon_cc_grpc",
        "@asphr//schema:message_proto_cc",
        "@asphr//schema:server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_library(
    name = "transmitter_lib",
    srcs = [
        "inbox.cc",
        "outbox.cc",
        "transmitter.cc",
    ],
    hdrs = [
        "inbox.hpp",
        "outbox.hpp",
        "transmitter.hpp",
    ],
    linkstatic = True,
    deps = [
        ":base_lib",
        "@asphr//asphr:asphr_lib",
        "@asphr//pir/fast_pir:fast_pir_lib",
        "@asphr//schema:message_proto_cc",
        "@asphr//schema:server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "daemon",
    srcs = [
        "anysphere.cc",
        "anysphere.hpp",
    ],
    linkstatic = True,
    visibility = ["//:__pkg__"],
    deps = [
        ":daemon_lib",
        ":transmitter_lib",
        "//client_lib",
    ],
)

cc_test(
    name = "test",
    size = "small",
    srcs = glob(["**/*_test.cc"]),
    linkstatic = True,
    deps = [
        ":daemon_lib",
        ":transmitter_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
