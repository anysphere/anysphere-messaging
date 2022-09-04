#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: MIT
#

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "asphr_lib",
    srcs = [
        "asphr.hpp",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        ":assert",
        ":log",
        ":utils",
        "//asphr/third_party/json",
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

cc_library(
    name = "assert",
    hdrs = [
        "assert.hpp",
    ],
    linkstatic = True,
    deps = select({
        ":opt_build": [],
        "//conditions:default": ["@boost//:stacktrace"],
    }),
)

cc_binary(
    name = "assert_manual_test",
    srcs = ["assert_manual_test.cc"],
    linkstatic = True,
    deps = [
        ":assert",
    ],
)

cc_library(
    name = "log",
    hdrs = [
        "foreach.hpp",
        "log.hpp",
    ],
    defines = select({
        ":log_level_dbg": ["ASPHR_LOGLEVEL_DBG"],
        ":log_level_err": ["ASPHR_LOGLEVEL_ERR"],
        ":log_level_info": ["ASPHR_LOGLEVEL_INFO"],
        ":log_level_none": ["ASPHR_LOGLEVEL_NONE"],
        ":log_level_warn": ["ASPHR_LOGLEVEL_WARN"],
        "//conditions:default": [],
    }),
    linkstatic = True,
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
    ],
)

cc_binary(
    name = "log_manual_test",
    srcs = ["log_manual_test.cc"],
    linkstatic = True,
    deps = [
        ":log",
    ],
)

config_setting(
    name = "opt_build",
    values = {
        "compilation_mode": "opt",
    },
)

config_setting(
    name = "log_level_dbg",
    values = {"define": "log_level=dbg"},
)

config_setting(
    name = "log_level_info",
    values = {"define": "log_level=info"},
)

config_setting(
    name = "log_level_warn",
    values = {"define": "log_level=warn"},
)

config_setting(
    name = "log_level_err",
    values = {"define": "log_level=err"},
)

config_setting(
    name = "log_level_none",
    values = {"define": "log_level=none"},
)
