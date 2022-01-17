load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "fast_pir_lib",
    srcs = ["fast_pir_client.cc"],
    hdrs = [
        "fast_pir.hpp",
        "fast_pir_client.hpp",
        "fast_pir_config.hpp",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        "//asphr:asphr_lib",
        "@seal",
    ],
)
