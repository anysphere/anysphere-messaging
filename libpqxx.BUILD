filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
)

configure_make(
    name = "libpqxx",
    args = [
        "-j4",  # Make it faster by using more processes.
    ],
    configure_options = [
        "--disable-documentation",
    ],
    lib_source = ":all_srcs",
    out_static_libs = [
        "libpqxx.a",
    ],
    visibility = ["//visibility:public"],
)