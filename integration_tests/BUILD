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

cc_library(
    name = "daemon_setup",
    hdrs = ["daemon_setup.hpp"],
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
    name = "daemon_config_test",
    srcs = [
        "daemon_tests/daemon_config_test.cc",
    ],
    deps = [
        ":daemon_setup",
    ],
)

cc_test(
    name = "daemon_friends_test",
    srcs = [
        "daemon_tests/daemon_friends_test.cc",
    ],
    deps = [
        ":daemon_setup",
    ],
)

cc_test(
    name = "daemon_long_messages_test",
    srcs = [
        "daemon_tests/daemon_long_messages_test.cc",
    ],
    deps = [
        ":daemon_setup",
    ],
)

cc_test(
    name = "daemon_multiple_messages_test",
    srcs = [
        "daemon_tests/daemon_multiple_messages_test.cc",
    ],
    deps = [
        ":daemon_setup",
    ],
)

cc_test(
    name = "daemon_persistence_test",
    srcs = [
        "daemon_tests/daemon_persistence_test.cc",
    ],
    deps = [
        ":daemon_setup",
    ],
)

cc_test(
    name = "daemon_seen_test",
    srcs = [
        "daemon_tests/daemon_seen_test.cc",
    ],
    deps = [
        ":daemon_setup",
    ],
)

cc_test(
    name = "daemon_send_messages_test",
    srcs = [
        "daemon_tests/daemon_send_messages_test.cc",
    ],
    deps = [
        ":daemon_setup",
    ],
)

cc_test(
    name = "daemon_fast_test",
    srcs = [
        "daemon_tests/daemon_fast_test.cc",
    ],
    deps = [
        ":daemon_setup",
    ],
)

cc_test(
    name = "multiple_friends_test",
    size = "large",
    srcs = [
        "multiple_friends_test.cc",
    ],
    deps = [
        ":daemon_setup",
    ],
)
