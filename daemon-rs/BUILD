load("@rules_rust//rust:defs.bzl", "rust_binary")
load("@cxx.rs//tools/bazel:rust_cxx_bridge.bzl", "rust_cxx_bridge")

rust_binary(
    name = "hello_world",
    srcs = ["main.rs", "greeter.rs", "schema.rs"],
    deps = ["@crate_index//:diesel", "@cxx.rs//:cxx", ":bridge", ":cpp-imp"]
)

rust_cxx_bridge(
    name = "bridge",
    src = "main.rs",
    deps = [":cpp-include"],
)

cc_library(
    name = "lib-cpp",
    srcs = ["lib.hpp"],
    deps = [
        ":bridge",
    ]
)

cc_library(
    name = "cpp-imp",
    srcs = ["test.cc"],
    deps = [
        ":cpp-include",
        ":bridge",
        ":lib-cpp"
    ],
)

cc_library(
    name = "cpp-include",
    hdrs = ["test.hpp"],
    deps = ["@cxx.rs//:core"],
)