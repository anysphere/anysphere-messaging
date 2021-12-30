load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "as_server",
    srcs = ["as_server.cc"],
    deps = [
        "//schema:messenger_cc_proto",
        "@com_github_grpc_grpc//:grpcpp",
    ],
)
