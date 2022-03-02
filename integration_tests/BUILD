cc_library(
    name = "test_helpers",
    hdrs = ["test_helpers.hpp"],
    deps = [
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/src:as_server_lib",
        "@client//daemon:daemon_lib",
        "@client//daemon:transmitter_lib",
    ],
)

cc_test(
    name = "daemon_test",
    size = "large",
    srcs = [
        "daemon_test.cc",
    ],
    deps = [
        ":test_helpers",
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/src:as_server_lib",
        "@client//daemon:daemon_lib",
        "@client//daemon:transmitter_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "daemon_fast_test",
    srcs = [
        "daemon_fast_test.cc",
    ],
    deps = [
        ":test_helpers",
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/src:as_server_lib",
        "@client//daemon:daemon_lib",
        "@client//daemon:transmitter_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "multiple_friends_test",
    size = "large",
    srcs = [
        "multiple_friends_test.cc",
    ],
    deps = [
        ":test_helpers",
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/src:as_server_lib",
        "@client//daemon:daemon_lib",
        "@client//daemon:transmitter_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
