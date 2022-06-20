load("@cxx.rs//tools/bazel:rust_cxx_bridge.bzl", "rust_cxx_bridge")
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library", "rust_static_library", "rust_test")

rust_binary(
    name = "main",
    srcs = [
        "cli.rs",
        "main.rs",
    ],
    deps = [
        ":cpp-imp",
        ":main-bridge",
        "@cxx.rs//:cxx",
    ],
)

filegroup(
    name = "migrations",
    srcs = glob(["migrations/**/*.sql"]),
)

rust_cxx_bridge(
    name = "main-bridge",
    src = "main.rs",
    deps = [
        ":cpp-imp",
    ],
)

rust_cxx_bridge(
    name = "bridge",
    src = "db/db.rs",
    deps = [":cpp-include"],
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
        ":bridge",
        "@crate_index//:diesel",
        "@crate_index//:diesel_migrations",
        "@crate_index//:libsqlite3-sys",
        "@crate_index//:rand",
        "@cxx.rs//:cxx",
    ],
)

cc_library(
    name = "crypto-lib",
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
    ],
)

cc_library(
    name = "cpp-imp",
    srcs = [
        "crypto/crypto.cc",
        "global.cc",
        "main.cc",
        "rpc/daemon_rpc.cc",
        "transmitter/transmitter.cc",
    ],
    hdrs = [
        "constants.hpp",
        "crypto/crypto.hpp",
        "global.hpp",
        "rpc/daemon_rpc.hpp",
        "transmitter/transmitter.hpp",
    ],
    linkstatic = True,
    deps = [
        ":bridge",
        ":cpp-db",
        ":cpp-include",
        ":db_lib",
        "//client_lib",
        "@asphr//pir/fast_pir:fast_pir_lib",
        "@asphr//schema:daemon_cc_grpc",
        "@asphr//schema:message_proto_cc",
        "@asphr//schema:server_cc_grpc",
        "@asphr//third_party/libsodium",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_library(
    name = "cpp-db",
    hdrs = ["db/db.hpp"],
    linkstatic = True,
    deps = [
        ":bridge",
        ":cpp-include",
    ],
)

cc_library(
    name = "cpp-include",
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
        ":crypto-lib",
        "@com_google_googletest//:gtest_main",
    ],
)

rust_test(
    name = "db_test",
    crate = ":db_lib",
)
