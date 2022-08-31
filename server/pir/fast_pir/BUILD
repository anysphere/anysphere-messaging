load("@rules_cc//cc:defs.bzl", "cc_library")

package(
    default_visibility = ["//integration_tests:__pkg__"],
)

cc_library(
    name = "server_fast_pir_lib",
    srcs = [
        "fastpir.cc",
    ],
    hdrs = [
        "fastpir.hpp",
    ],
    linkstatic = True,
    visibility = [
        "//integration_tests:__pkg__",
        "//server/src:__pkg__",
    ],
    deps = [
        "//asphr/asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
        "//asphr/third_party/seal",
    ],
)

cc_test(
    name = "server_fast_pir_test",
    srcs = [
        "fastpir_test.cc",
    ],
    linkstatic = True,
    deps = [
        ":server_fast_pir_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
