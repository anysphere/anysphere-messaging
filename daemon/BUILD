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
        "//server/src/pir/fastpir:fastpir_lib",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
