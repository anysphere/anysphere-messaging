load("@cxx.rs//tools/bazel:rust_cxx_bridge.bzl", "rust_cxx_bridge")
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_static_library", "rust_library", "rust_test")

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
        ":main_lib",  # TODO: shouldnt be this??
    ],
)

rust_cxx_bridge(
    name = "db_bridge",
    src = "db/db.rs",
    deps = [":cpp_include"],
)

# static because depended upon by c++!
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
        ":db_bridge",
        "@crate_index//:diesel",
        "@crate_index//:diesel_migrations",
        "@crate_index//:libsqlite3-sys",
        "@crate_index//:rand",
        "@cxx.rs//:cxx",
    ],
)

cc_library(
    name = "crypto_lib",
    srcs = ["crypto/crypto.cc"],
    hdrs = [
        "constants.hpp",
        "crypto/crypto.hpp",
    ],
    linkstatic = True,
    deps = [
        "@asphr//asphr:asphr_lib",
        "@asphr//schema:daemon_cc_grpc",
        "@asphr//schema:message_proto_cc",
        "@asphr//schema:server_cc_grpc",
        "@asphr//third_party/libsodium",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_library(
    name = "core_lib",
    srcs = ["global.cc"],
    hdrs = [
        "global.hpp",
    ],
    deps = [
        ":cpp_db_lib",
        ":cpp_include",
        ":crypto_lib",
        ":db_bridge",
        ":db_lib",
        "//client_lib",
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
    name = "cpp_include",
    hdrs = [
        "main.hpp",
        "util.hpp",
    ],
    linkstatic = True,
    deps = [
        "@asphr//asphr:asphr_lib",
        "@cxx.rs//:core",
    ],
)

cc_test(
    name = "crypto_test",
    srcs = [
        "crypto/crypto_test.cc",
    ],
    deps = [
        ":crypto_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

rust_test(
    name = "db_test",
    crate = ":db_lib",
)
