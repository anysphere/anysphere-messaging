load("@rules_cc//cc:defs.bzl", "cc_binary")
# load("@io_bazel_rules_docker//cc:image.bzl", "cc_image")
# load(
#     "@io_bazel_rules_docker//container:container.bzl",
# )

package(
    default_visibility = ["//integration_tests:__pkg__"],
)

cc_library(
    name = "as_server_lib",
    hdrs = [
        "account_manager.hpp",
        "beta_key_auth.hpp",
        "server_rpc.cc",
        "server_rpc.hpp",
    ],
    defines = select({
        "//conditions:default": ["USE_POSTGRES_DB"],
        ":in_memory_db": ["USE_MEMORY_DB"],
    }),
    linkopts = select({
        "//conditions:default": ["-lpq"],
        ":in_memory_db": [],
    }),
    linkstatic = True,
    deps = [
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/pir/nonprivate:nonprivate_pir_lib",
        "@asphr//asphr:asphr_lib",
        "@asphr//schema:server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ] + select({
        "//conditions:default": ["@libpqxx"],
        ":in_memory_db": [],
    }),
)

cc_binary(
    name = "as_server",
    srcs = [
        "as_server.cc",
    ],
    deps = [
        ":as_server_lib",
    ],
)

cc_test(
    name = "test",
    srcs = glob(["**/*_test.cc"]),
    defines = select({
        "//conditions:default": [],
        ":postgres_tests": ["ENABLE_POSTGRES_TESTS"],
    }),
    deps = [
        ":as_server_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

config_setting(
    name = "in_memory_db",
    values = {"define": "memdb=true"},
)

config_setting(
    name = "postgres_tests",
    values = {"define": "postgres_tests=true"},
)

# container_image(
#     name = "cc_libpq_image",
#     base = "@cc_image_base//image",
# )

# cc_image(
#     name = "as_server_image",
#     binary = ":as_server",
# )

# container_image(
#     name = "as_server_image_prod",
#     base = ":as_server_image",
#     cmd = [
#         "-d",
#         "127.0.0.1",
#     ],
# )
