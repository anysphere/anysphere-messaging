load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
    name = "as_server_lib",
    hdrs = [
        "account_manager.h",
        "server_rpc.cc",
        "server_rpc.hpp",
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
        "//asphr:asphr_lib",
        "//schema:server_cc_grpc",
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/pir/nonprivate:nonprivate_pir_lib",
        "@com_github_grpc_grpc//:grpc++",
        "@libpqxx",
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

cc_test(
    name = "test",
    srcs = glob(["**/*_test.cc"]),
    deps = [
        ":as_server_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
