load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@io_bazel_rules_docker//cc:image.bzl", "cc_image")
load(
    "@io_bazel_rules_docker//container:container.bzl",
    "container_image",
)

package(
    default_visibility = ["//integration_tests:__pkg__"],
)

cc_library(
    name = "as_server_lib",
    hdrs = [
        "account_manager.hpp",
        "server_rpc.cc",
        "server_rpc.hpp",
        "beta_key_auth.hpp",
    ],
    defines = select({
        "//conditions:default": ["USE_POSTGRES_DB"],
        ":in_memory_db": ["USE_MEMORY_DB"],
    }),
    linkopts = ["-lpq"],
    linkstatic = True,
    deps = [
        "//asphr:asphr_lib",
        "//schema:server_cc_grpc",
        "//server/pir/fast_pir:server_fast_pir_lib",
        "//server/pir/nonprivate:nonprivate_pir_lib",
        "@com_github_grpc_grpc//:grpc++",
        "@libpqxx",
    ],
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

config_setting(
    name = "in_memory_db",
    values = {"define": "memdb=true"},
)

cc_test(
    name = "test",
    srcs = glob(["**/*_test.cc"]),
    deps = [
        ":as_server_lib",
        "@com_google_googletest//:gtest_main",
    ],
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
