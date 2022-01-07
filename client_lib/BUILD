cc_library(
    name = "client_lib",
    srcs = [
        "client_lib.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//asphr:asphr_lib",
        "@com_google_absl//absl/hash",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@libsodium",
    ],
)

cc_test(
    name = "client_lib_test",
    size = "small",
    srcs = ["client_lib_test.cc"],
    deps = ["@com_google_googletest//:gtest_main"],
)
