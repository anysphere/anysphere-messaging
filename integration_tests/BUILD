#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: MIT
#

load("@rules_cc//cc:defs.bzl", "cc_library")
load(":helper.bzl", "all_tests_in_folder")

cc_library(
    name = "daemon_setup",
    hdrs = [
        "daemon_setup.hpp",
        "test_helpers.hpp",
    ],
    linkstatic = True,
    deps = [
        "//client/daemon:main_lib",
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/src:as_server_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "attack_setup",
    hdrs = [
        "daemon_setup.hpp",
        "test_helpers.hpp",
        "//integration_tests/attack_tests:attacker_template.hpp",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        "//client/daemon:main_lib",
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/src:as_server_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

all_tests_in_folder(folder = "daemon_tests")

all_tests_in_folder(folder = "multiple_friends_tests")

all_tests_in_folder(folder = "invitation_tests")

#all_tests_in_folder(folder = "playground_tests")
