load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
    name = "daemon_lib",
    hdrs = [
        "anysphere.h",
        "crypto.h",
        "ui_msg.h",
        "ui_urgent.h",
    ],
    deps = [
        "//asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
        "//client/client_lib",
        "//schema:messenger_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@libsodium",
    ],
)

cc_binary(
    name = "daemon",
    srcs = [
        "anysphere.cc",
    ],
    deps = [
        ":daemon_lib",
    ],
)

cc_test(
    name = "test",
    size = "small",
    srcs = glob(["**/*_test.cc"]),
    deps = [
        ":daemon_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
