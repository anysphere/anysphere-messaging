load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "anysphere",
    srcs = [
        "anysphere.cc",
        "base64.h",
        "common.h",
        "nlohmann_json.h",
        "ui_msg.h",
        "ui_urgent.h",
    ],
    deps = [
        "//schema:messenger_cc_grpc",
        "//server/src/pir/fastpir:fastpir_lib",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
