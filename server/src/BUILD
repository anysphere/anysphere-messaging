load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
# load("@io_bazel_rules_docker//cc:image.bzl", "cc_image")
# load(
#     "@io_bazel_rules_docker//container:container.bzl",
# )

package(
    default_visibility = ["//integration_tests:__pkg__"],
)

cc_library(
    name = "beta_key_lib",
    hdrs = ["beta_key_auth.hpp"],
    defines = select({
        ":hn_beta_key": ["USE_HN_BETA_KEY"],
        "//conditions:default": [],
    }),
    deps = ["//asphr/asphr:asphr_lib"]
)

cc_library(
    name = "as_server_lib",
    srcs = [
        "async_invitation_database.cc",
    ],
    hdrs = [
        "account_manager.hpp",
        "async_invitation_database.hpp",
        "server_constants.hpp",
        "server_rpc.cc",
        "server_rpc.hpp",
    ],
    defines = select({
        ":in_memory_db": ["USE_MEMORY_DB"],
        "//conditions:default": ["USE_POSTGRES_DB"],
    }),
    linkopts = select({
        ":in_memory_db": [],
        "//conditions:default": ["-lpq"],
    }),
    linkstatic = True,
    deps = [
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/pir/nonprivate:nonprivate_pir_lib",
        ":beta_key_lib",
        "//asphr/asphr:asphr_lib",
        "//asphr/schema:server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ] + select({
        ":in_memory_db": [],
        "//conditions:default": ["@libpqxx"],
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
        ":postgres_tests": ["ENABLE_POSTGRES_TESTS"],
        "//conditions:default": [],
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

config_setting(
    name = "hn_beta_key",
    values = {"define": "hn_beta=true"},
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
