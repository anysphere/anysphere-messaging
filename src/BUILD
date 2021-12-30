load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "anysphere",
    srcs = ["anysphere.cc"],
    deps = [
        "//schema:messenger_cc_proto",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
