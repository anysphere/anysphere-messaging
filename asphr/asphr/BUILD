#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

cc_library(
    name = "asphr_lib",
    srcs = [
        "asphr.hpp",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        ":utils",
        "//third_party/json",
        "@com_google_absl//absl/hash",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "utils",
    srcs = [
        "utils.cc",
    ],
    hdrs = [
        "utils.hpp",
    ],
    linkstatic = True,
    deps = [],
)

cc_test(
    name = "utils_test",
    size = "small",
    srcs = ["utils_test.cc"],
    linkstatic = True,
    deps = [
        ":utils",
        "@com_google_googletest//:gtest_main",
    ],
)
