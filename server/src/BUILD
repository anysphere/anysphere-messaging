load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")


cc_binary(
    name = "as_server",
    srcs = [
        "messenger_impl.h",
        "nonprivate_pir.h",
        "nonprivate_pir.cc",
        "pir_common.h",
        "account_manager.h",
        "as_server.cc"
    ],
    linkopts = ["-lpq"],
    deps = [
        "//schema:messenger_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@libpqxx//:libpqxx",
    ],
)