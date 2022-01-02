load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "anysphere-cli",
    srcs = ["as_cli.cc", 
            "common.h",
            "nlohmann_json.h"],
    linkopts = ["-lpthread"],
    deps = [
        "@com_github_daniele77_cli//:cli_header_lib",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/strings", 
    ],
)
