load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
    name = "config_lib",
    srcs = ["config.cc"],
    hdrs = [
        "config.hpp",
    ],
    deps = [
        "//asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
    ],
)

cc_library(
    name = "crypto_lib",
    srcs = ["crypto.cc"],
    hdrs = ["crypto.hpp"],
    deps = [
        ":config_lib",
        "//asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
        "//client/client_lib",
        "//schema:message_proto_cc",
        "@libsodium",
    ],
)

cc_library(
    name = "daemon_lib",
    srcs = [
        "daemon_rpc.cc",
    ],
    hdrs = [
        "daemon_rpc.hpp",
        "ui_msg.hpp",
        "utils.hpp",
    ],
    deps = [
        ":config_lib",
        ":crypto_lib",
        "//asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
        "//schema:daemon_cc_grpc",
        "//schema:message_proto_cc",
        "//schema:server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "daemon",
    srcs = [
        "anysphere.cc",
        "anysphere.hpp",
    ],
    deps = [
        ":daemon_lib",
        "//client/client_lib",
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
