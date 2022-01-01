load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")


cc_binary(
    name = "as_server",
    srcs = [
        "messenger_impl.h",
        "nonprivate_pir.h",
        "nonprivate_pir.cc",
        "pir_common.h",
        "account_manager.h",
        "as_server.cc"
    ],
    linkopts = ["-lpq"],
    deps = [
        "//schema:messenger_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        ":libpqxx",
    ],
)

configure_make(
    name = "libpqxx",
    args = [
        "-j4",  # Make it faster by using more processes.
    ],
    configure_options = [
        "--disable-documentation",
    ],
    lib_source = "@libpqxx//:all_srcs",
    out_static_libs = [
        "libpqxx.a",
    ],
)