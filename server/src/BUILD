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
        ":libpqxx",
    ],
)

cmake(
    name = "libpqxx",
    cache_entries = {
        "CMAKE_C_FLAGS": "-fPIC",
        "PostgreSQL_TYPE_INCLUDE_DIR": "/usr/include/postgresql",
        "PostgreSQL_INCLUDE_DIR": "/usr/include/postgresql",
        "PostgreSQL_LIBRARY": "/usr/lib/x86_64-linux-gnu/libpq.so",
    },
    lib_source = "@libpqxx//:all_srcs",
    out_static_libs = [
        "libpqxx.a",
    ],
    build_args = [
        "--verbose",
        "--",  # <- Pass remaining options to the native tool.
        "-j 16",
    ],
)