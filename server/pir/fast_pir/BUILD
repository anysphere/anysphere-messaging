load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "server_fast_pir_lib",
    hdrs = [
        "fast_pir_client.h",
        "fastpir.h",
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
