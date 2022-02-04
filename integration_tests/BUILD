cc_test(
    name = "daemon_test",
    srcs = [
        "daemon_test.cc",
    ],
    deps = [
        "//client/daemon:daemon_lib",
        "//client/daemon:transmitter_lib",
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/src:as_server_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "multiple_friends_test",
    srcs = [
        "multiple_friends_test.cc",
    ],
    deps = [
        "//client/daemon:daemon_lib",
        "//client/daemon:transmitter_lib",
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/src:as_server_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
