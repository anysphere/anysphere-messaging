load("@rules_cc//cc:defs.bzl", "cc_library")

package(
    default_visibility = ["//integration_tests:__pkg__"],
)

cc_library(
    name = "server_fast_pir_lib",
    hdrs = [
        "fastpir.hpp",
    ],
    visibility = [
        "//integration_tests:__pkg__",
        "//server/src:__pkg__",
    ],
    deps = [
        "//asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
        "@seal",
    ],
)

cc_test(
    name = "server_fast_pir_test",
    srcs = [
        "fastpir_test.cc",
    ],
    deps = [
        ":server_fast_pir_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
