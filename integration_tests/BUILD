load("@rules_cc//cc:defs.bzl", "cc_library")
load(":helper.bzl", "all_tests_in_folder")

cc_library(
    name = "daemon_setup",
    hdrs = [
        "daemon_setup.hpp",
        "test_helpers.hpp",
    ],
    deps = [
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/src:as_server_lib",
        "@client//daemon:main_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

all_tests_in_folder(folder = "daemon_tests")

all_tests_in_folder(folder = "multiple_friends_tests")
