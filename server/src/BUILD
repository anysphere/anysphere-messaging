load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
    name = "as_server_lib",
    srcs = [
        "pir/nonprivate/nonprivate_pir.cc",
    ],
    hdrs = [
        "account_manager.h",
        "messenger_impl.h",
        "pir/fastpir/fastpir.h",
        "pir/fastpir/fastpir_client.h",
        "pir/fastpir/fastpir_config.h",
        "pir/nonprivate/nonprivate_pir.h",
        "pir/pir_common.h",
        "pir/sealpir/sealpir.h",
    ],
    defines = select({
        "//conditions:default": ["USE_POSTGRES_DB"],
        ":in_memory_db": ["USE_MEMORY_DB"],
    }),
    linkopts = ["-lpq"],
    visibility = [
        "//server/src:__pkg__",
        "//server/test:__pkg__",
    ],
    deps = [
        "//anysphere:utils",
        "//schema:messenger_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@libpqxx",
        "@seal",
    ],
)

cc_binary(
    name = "as_server",
    srcs = [
        "as_server.cc",
    ],
    deps = [
        ":as_server_lib",
    ],
)

config_setting(
    name = "in_memory_db",
    values = {"define": "memdb=true"},
)
