filegroup(
    name = "clang_tidy_config",
    data = [".clang-tidy"],
    visibility = ["//visibility:public"],
)

platform(
    name = "macos-arm64",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:arm64",
    ],
)

platform(
    name = "macos-x86_64",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
    ],
)

platform(
    name = "linux-x86_64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
)

filegroup(
    name = "asphr-release",
    srcs = [
        "//client/cli:asphr",
        "//client/daemon",
    ],
)
