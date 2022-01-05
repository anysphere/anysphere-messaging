load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "fastpir_lib",
    hdrs = [
        "fastpir.h",
        "fastpir_client.h",
    ],
    visibility = [
        "//client/daemon:__pkg__",
        "//server/src:__pkg__",
        "//server/test:__pkg__",
    ],
    deps = [
        "//asphr:asphr_lib",
        "//asphr/pir/fast_pir:fast_pir_lib",
        "@seal",
    ],
)
