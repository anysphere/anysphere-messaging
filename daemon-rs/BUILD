load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library")
load("@cxx.rs//tools/bazel:rust_cxx_bridge.bzl", "rust_cxx_bridge")

rust_binary(
    name = "main",
    srcs = [
        "main.rs",
        "schema.rs",
    ],
    deps = [
        ":bridge",
        ":cpp-imp",
        "@crate_index//:diesel",
        "@cxx.rs//:cxx",
    ],
)

rust_cxx_bridge(
    name = "bridge",
    src = "main.rs",
    deps = [":cpp-include"],
)

cc_library(
    name = "cpp-imp",
    srcs = ["main.cc"],
    deps = [
        ":bridge",
        ":cpp-include",
    ],
)

cc_library(
    name = "cpp-include",
    hdrs = ["main.hpp"],
    deps = ["@cxx.rs//:core"],
)
