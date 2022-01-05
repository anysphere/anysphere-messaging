load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "fast_pir_lib",
    hdrs = [
        "fast_pir.h",
        "fast_pir_config.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//asphr:asphr_lib",
        "@seal",
    ],
)
