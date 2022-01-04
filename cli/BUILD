load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "anysphere-cli",
    srcs = [
        "as_cli.cc",
        "common.h",
    ],
    linkopts = ["-lpthread"],
    deps = [
        "//asphr:asphr_lib",
        "@com_github_daniele77_cli//:cli_header_lib",
        
    ],
)
