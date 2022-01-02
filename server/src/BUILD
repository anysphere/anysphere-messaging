load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "as_server",
    srcs = [
        "account_manager.h",
        "as_server.cc",
        "messenger_impl.h",
        "pir/fastpir/fastpir.h",
        "pir/fastpir/fastpir_config.h",
        "pir/nonprivate/nonprivate_pir.cc",
        "pir/nonprivate/nonprivate_pir.h",
        "pir/pir_common.h",
        "pir/sealpir/sealpir.h",
    ],
    defines = select({
        "//conditions:default": ["USE_POSTGRES_DB"],
        ":in_memory_db": ["USE_MEMORY_DB"],
    }),
    linkopts = ["-lpq"],
    deps = [
        "//schema:messenger_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@libpqxx",
        "@seal",
    ],
)

config_setting(
    name = "in_memory_db",
    values = {"define": "memdb=true"},
)
