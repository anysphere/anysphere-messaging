load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "as_server",
    srcs = [
        "messenger_impl.h",
        "pir/nonprivate/nonprivate_pir.h",
        "pir/nonprivate/nonprivate_pir.cc",
        "pir/sealpir/seal_test.h",
        "pir/pir_common.h",
        "account_manager.h",
        "as_server.cc"
    ],
    linkopts = ["-lpq"],
    deps = [
        "//schema:messenger_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@libpqxx//:libpqxx",
        "@seal//:seal",
    ],
    defines = select({
        "//conditions:default": ["USE_POSTGRES_DB"],
        ":in_memory_db": ["USE_MEMORY_DB"],
    })
    
)

config_setting(
    name = "in_memory_db",
    values = {"define": "memdb=true"}
)