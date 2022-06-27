#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

cc_library(
    name = "client_lib",
    srcs = [
        "client_lib.cc",
    ],
    hdrs = [
        "client_lib.hpp",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@asphr//asphr:asphr_lib",
        "@asphr//third_party/libsodium",
        "@com_google_absl//absl/hash",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
    ],
)

cc_test(
    name = "client_lib_test",
    size = "small",
    srcs = ["client_lib_test.cc"],
    deps = ["@com_google_googletest//:gtest_main"],
)
