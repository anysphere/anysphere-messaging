load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library")
load("@cxx.rs//tools/bazel:rust_cxx_bridge.bzl", "rust_cxx_bridge")

rust_binary(
    name = "main",
    srcs = [
        "main.rs",
        "db/schema.rs",
        "db/mod.rs"
    ],
    compile_data = [":migrations"],
    deps = [
        ":bridge",
        ":main-bridge",
        ":cpp-imp",
        "@crate_index//:chrono",
        "@crate_index//:diesel",
        "@crate_index//:diesel_migrations",
        "@crate_index//:rand",
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
    src = "db/mod.rs",
    deps = [":cpp-include"],
)

cc_library(
    name = "crypto-lib",
    srcs = ["crypto/crypto.cc"],
    hdrs = ["crypto/crypto.hpp",
            "constants.hpp",],
    deps = [
        "@asphr//third_party/libsodium",
        "@asphr//schema:daemon_cc_grpc",
        "@asphr//schema:message_proto_cc",
        "@asphr//schema:server_cc_grpc",
        "@asphr//asphr:asphr_lib",
    ]
)

cc_library(
    name = "cpp-imp",
    srcs = [
        "crypto/crypto.cc",
        "main.cc",
        # "rpc/daemon_rpc.cc",
        "transmitter/transmitter.cc",
        "global.cc",
    ],
    hdrs = [
        "constants.hpp",
        "crypto/crypto.hpp",
        "global.hpp",
        # "rpc/daemon_rpc.hpp",
        "transmitter/transmitter.hpp",
    ],
    linkstatic = True,
    deps = [
        ":bridge",
        ":cpp-db",
        ":cpp-include",
        "//client_lib",
        "@asphr//asphr:asphr_lib",
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
    hdrs = ["main.hpp"],
    linkstatic = True,
    deps = ["@cxx.rs//:core"],
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
