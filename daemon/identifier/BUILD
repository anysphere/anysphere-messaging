#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "base58",
    srcs = ["base58.cc"],
    hdrs = ["base58.hpp"],
    deps = [
        "@asphr//asphr:asphr_lib",
    ],
)

cc_test(
    name = "base58_test",
    srcs = ["base58_test.cc"],
    deps = [
        ":base58",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "identifier",
    srcs = ["identifier.cc"],
    hdrs = [
        "identifier.hpp",
        "wordlist.hpp",
    ],
    visibility = ["//daemon:__subpackages__"],
    deps = [
        ":base58",
        "//daemon/crypto:crypto_lib",
        "@asphr//asphr:asphr_lib",
        "@asphr//schema:identifier_proto_cc",
    ],
)

cc_library(
    name = "identifier_helpers_private_do_not_depend_on_except_in_this_test",
    srcs = ["identifier.cc"],
    hdrs = [
        "identifier.hpp",
        "identifier_helpers_private.hpp",
        "wordlist.hpp",
    ],
    deps = [
        ":base58",
        "//daemon/crypto:crypto_lib",
        "@asphr//asphr:asphr_lib",
        "@asphr//schema:identifier_proto_cc",
    ],
)

cc_test(
    name = "identifier_test",
    srcs = [
        "identifier_test.cc",
    ],
    deps = [
        ":identifier",
        ":identifier_helpers_private_do_not_depend_on_except_in_this_test",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "identifier_manual_test",
    srcs = ["identifier_manual_test.cc"],
    deps = [
        ":identifier",
    ],
)
