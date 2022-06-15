load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_doc")

rust_binary(
    name = "hello_world",
    srcs = ["src/main.rs", "src/greeter.rs"],
)

rust_doc(
    name = "hello_world_doc",
    crate = ":hello_world",
)