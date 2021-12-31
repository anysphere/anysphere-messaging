load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "as_server",
    srcs = [
        "messenger_impl.h",
        "pir_db.h",
        "as_server.cc"
    ],
    deps = [
        "//schema:messenger_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
