load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake_variant")

package(
    default_visibility = ["//visibility:public"],
)

filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
    visibility = ["//:__pkg__"],
)

cmake_variant(
    name = "libpqxx",
    build_args = [
        "--verbose",
    ],
    cache_entries = {
        "CMAKE_C_FLAGS": "-fPIC",
        "PostgreSQL_TYPE_INCLUDE_DIR": "/usr/include/postgresql",
        "PostgreSQL_INCLUDE_DIR": "/usr/include/postgresql",
        "PostgreSQL_LIBRARY": "/usr/lib/x86_64-linux-gnu/libpq.so",
    },
    generate_args = [
        "-G Ninja",
    ],
    lib_source = ":all_srcs",
    out_static_libs = [
        "libpqxx.a",
    ],
    toolchain = "@rules_foreign_cc//toolchains:preinstalled_ninja_toolchain",
)
