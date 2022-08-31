#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

load("@cxx.rs//tools/bazel:rust_cxx_bridge.bzl", "rust_cxx_bridge")
load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_static_library", "rust_test")

rust_binary(
    name = "main",
    srcs = [
        "cli.rs",
        "main.rs",
    ],
    visibility = ["//visibility:public"],  # for release
    deps = [
        ":main_bridge",
        ":main_lib",
        "@cxx.rs//:cxx",
    ],
)

filegroup(
    name = "migrations",
    srcs = glob(["migrations/**/*.sql"]),
)

rust_cxx_bridge(
    name = "main_bridge",
    src = "main.rs",
    deps = [
        ":cpp_include",
    ],
)

rust_cxx_bridge(
    name = "db_bridge",
    src = "db/db.rs",
    deps = [":cpp_include"],
)

# static because depended upon by c++!
# NOTE: this includes chunk_handler_lib because the db_test needs it. I know :(
# TODO(sualeh): remove the dependency on chunk_handler_lib once we can get modular parts to work.
rust_static_library(
    name = "db_lib",
    srcs = [
        "db/db.rs",
        "db/lib.rs",
        "db/schema.rs",
        "db/tests.rs",
    ],
    compile_data = [":migrations"],
    deps = [
        ":chunk_handler_lib",
        ":db_bridge",
        "@crate_index//:anyhow",
        "@crate_index//:diesel",
        "@crate_index//:diesel_migrations",
        "@crate_index//:libsqlite3-sys",
        "@crate_index//:rand",
        "@cxx.rs//:cxx",
    ],
)

cc_library(
    name = "core_lib",
    srcs = ["global.cc"],
    hdrs = [
        "global.hpp",
    ],
    linkstatic = True,
    deps = [
        ":chunk_handler_lib",
        ":cpp_db_lib",
        ":cpp_include",
        ":db_bridge",
        ":db_lib",
        "//client_lib",
        "//daemon/crypto:crypto_lib",
        "//daemon/identifier",
        "@asphr//asphr:asphr_lib",
    ],
)

cc_library(
    name = "rpc_lib",
    srcs = [
        "rpc/daemon_rpc.cc",
    ],
    hdrs = [
        "rpc/daemon_rpc.hpp",
    ],
    linkstatic = True,
    deps = [
        ":core_lib",
        "@asphr//pir/fast_pir:fast_pir_lib",
        "@asphr//schema:daemon_cc_grpc",
        "@asphr//schema:message_proto_cc",
        "@asphr//schema:server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_library(
    name = "transmitter_lib",
    srcs = [
        "transmitter/transmitter.cc",
    ],
    hdrs = [
        "transmitter/transmitter.hpp",
    ],
    linkstatic = True,
    deps = [
        ":core_lib",
        "@asphr//pir/fast_pir:fast_pir_lib",
    ],
)

cc_library(
    name = "main_lib",
    srcs = [
        "main.cc",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"],  # for integration tests
    deps = [
        ":main_bridge/include",
        ":rpc_lib",
        ":transmitter_lib",
    ],
)

cc_library(
    name = "cpp_db_lib",
    hdrs = ["db/db.hpp"],
    linkstatic = True,
    deps = [
        ":cpp_include",
        ":db_bridge",
    ],
)

cc_library(
    name = "chunk_handler_lib",
    srcs = [
        "chunk_handler/chunk_handler.cc",
    ],
    hdrs = [
        "chunk_handler/chunk_handler.hpp",
    ],
    linkstatic = True,
    deps = [
        ":cpp_db_lib",
        ":db_bridge",
        "@asphr//asphr:asphr_lib",
        "@asphr//schema:message_proto_cc",
    ],
)

cc_library(
    name = "cpp_include",
    hdrs = [
        "chunk_handler/chunk_handler.hpp",
        "main.hpp",
        "util.hpp",
    ],
    linkstatic = True,
    deps = [
        "@asphr//asphr:asphr_lib",
        "@cxx.rs//:core",
    ],
)

rust_test(
    name = "db_test",
    crate = ":db_lib",
)
