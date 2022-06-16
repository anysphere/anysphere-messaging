load("@rules_rust//rust:defs.bzl", "rust_binary")
load("@cxx.rs//tools/bazel:rust_cxx_bridge.bzl", "rust_cxx_bridge")

rust_binary(
    name = "hello_world",
    srcs = ["src/main.rs", "src/greeter.rs", "src/models.rs", "src/schema.rs"],
    deps = ["@crate_index//:diesel", "@cxx.rs//:cxx", ":bridge", ":cpp-imp"]
)

rust_cxx_bridge(
    name = "bridge",
    src = "src/main.rs",
    deps = [":cpp-include"],
)

cc_library(
    name = "lib-cpp",
    srcs = ["src/lib.hpp"],
    deps = [
        ":bridge",
    ]
)

cc_library(
    name = "cpp-imp",
    srcs = ["src/test.cc"],
    deps = [
        ":cpp-include",
        ":bridge",
        ":lib-cpp"
    ],
)

cc_library(
    name = "cpp-include",
    hdrs = ["src/test.hpp"],
    deps = ["@cxx.rs//:core"],
)