load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "anysphere",
    srcs = [
        "anysphere.cc",
        "anysphere.h",
        "ui_msg.h",
        "ui_urgent.h",
    ],
    deps = [
        "//asphr:asphr_lib",
        "//client/client_lib",
        "//schema:messenger_cc_grpc",
        "//server/pir/fast_pir:server_fast_pir_lib",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_test(
    name = "anysphere_test",
    size = "small",
    srcs = ["anysphere_test.cc"],
    deps = ["@com_google_googletest//:gtest_main"],
)
