load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
)
cmake(
    name = "libpqxx",
    cache_entries = {
        "CMAKE_C_FLAGS": "-fPIC",
        "PostgreSQL_TYPE_INCLUDE_DIR": "/usr/include/postgresql",
        "PostgreSQL_INCLUDE_DIR": "/usr/include/postgresql",
        "PostgreSQL_LIBRARY": "/usr/lib/x86_64-linux-gnu/libpq.so",
    },
    lib_source = ":all_srcs",
    out_static_libs = [
        "libpqxx.a",
    ],
    build_args = [
        "--verbose",
        "--",  # <- Pass remaining options to the native tool.
        "-j 16",
    ],
    visibility = ["//visibility:public"],
)