load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "anysphere-cli",
    srcs = ["as_cli.cc"],
    linkopts = ["-lpthread"],
    deps = [
        "@com_github_daniele77_cli//:cli_header_lib",
    ],
)