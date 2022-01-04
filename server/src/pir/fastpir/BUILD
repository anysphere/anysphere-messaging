load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "fastpir_lib",
    hdrs = [
        "fastpir.h",
        "fastpir_client.h",
        "fastpir_config.h",
    ],
    visibility = [
        "//client/daemon:__pkg__",
        "//server/src:__pkg__",
        "//server/test:__pkg__",
    ],
    deps = [
        "//anysphere:utils",
        "@seal",
    ],
)
