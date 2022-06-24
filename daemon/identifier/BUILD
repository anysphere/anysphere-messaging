cc_library(
    name = "base58",
    srcs = ["base58.cpp"],
    hdrs = ["base58.hpp"],
    deps = [
        "@asphr//asphr:asphr_lib",
    ],
)

cc_test(
    name = "base58_test",
    srcs = ["base58_test.cpp"],
    deps = [
        ":base58",
        "@com_google_googletest//:gtest_main",
    ],
)
