load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "anysphere",
    srcs = ["anysphere.cc",
            "nlohmann_json.h",
            "ui_urgent.h",
            "ui_msg.h",
            "common.h"],
    deps = [
        "//schema:messenger_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
