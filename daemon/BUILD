load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
    name = "crypto_lib",
    srcs = ["crypto.cc"],
    hdrs = ["crypto.hpp"],
    deps = [
        "//asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
    ],
)

cc_library(
    name = "daemon_lib",
    srcs = [
        "daemon_rpc.cc",
    ],
    hdrs = [
        "anysphere.h",
        "daemon_rpc.hpp",
        "ui_msg.h",
        "ui_urgent.h",
    ],
    deps = [
        ":crypto_lib",
        "//asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
        "//client/client_lib",
        "//schema:daemon_cc_grpc",
        "//schema:message_proto_cc",
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
