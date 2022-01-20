load("@rules_cc//cc:defs.bzl", "cc_binary")

package(
    default_visibility = ["//integration_tests:__pkg__"],
)

cc_library(
    name = "base_lib",
    srcs = ["config.cc", "crypto.cc"],
    hdrs = [
        "config.hpp",
        "constants.hpp",
        "crypto.hpp"
    ],
    linkstatic = True,
    deps = [
        "//asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
        "//client/client_lib",
        "//schema:message_proto_cc",
        "@libsodium",
    ],
)

cc_test(
    name = "crypto_test",
    size = "small",
    srcs = ["crypto_test.cc"],
    deps = [
        ":base_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "daemon_lib",
    srcs = [
        "daemon_rpc.cc",
        "utils.hpp",
    ],
    hdrs = [
        "daemon_rpc.hpp",
    ],
    linkstatic = True,
    deps = [
        ":base_lib",
        "//asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
        "//schema:daemon_cc_grpc",
        "//schema:message_proto_cc",
        "//schema:server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_library(
    name = "transmitter_lib",
    srcs = [
        "inbox.cc",
        "outbox.cc",
        "transmitter.cc",
    ],
    hdrs = [
        "inbox.hpp",
        "outbox.hpp",
        "transmitter.hpp",
    ],
    linkstatic = True,
    deps = [
        ":base_lib",
        "//asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
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
    linkstatic = True,
    visibility = ["//:__pkg__"],
    deps = [
        ":daemon_lib",
        ":transmitter_lib",
        "//client/client_lib",
    ],
)

cc_test(
    name = "test",
    size = "small",
    srcs = glob(["**/*_test.cc"]),
    linkstatic = True,
    deps = [
        ":daemon_lib",
        ":transmitter_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
