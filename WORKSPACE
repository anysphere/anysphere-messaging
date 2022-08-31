#
# Copyright 2022 Anysphere, Inc.
#

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

RUST_VERSION = "1.61.0"
BAZEL_TOOLCHAIN_TAG = "0.6.3"
BAZEL_TOOLCHAIN_SHA = "da607faed78c4cb5a5637ef74a36fdd2286f85ca5192222c4664efec2d529bb8"
asphr_path = "//asphr"

http_archive(
    name = "com_grail_bazel_toolchain",
    canonical_id = BAZEL_TOOLCHAIN_TAG,
    sha256 = BAZEL_TOOLCHAIN_SHA,
    strip_prefix = "bazel-toolchain-{tag}".format(tag = BAZEL_TOOLCHAIN_TAG),
    url = "https://github.com/grailbio/bazel-toolchain/archive/{tag}.tar.gz".format(tag = BAZEL_TOOLCHAIN_TAG),
)

# TODO(sualeh): this should be upgraded to something stable.
# Bug, main issue: cyclic dependency for rules_nodejs??
# http_archive(
#     name = "rules_proto_grpc",
#     sha256 = "a0519dccb89582a403a585fbdcd927d36894d43b5d9e71f445221d221d7106fd",
#     strip_prefix = "rules_proto_grpc-7ceb4ffaeaa9b3a4734bc7669b2a1c5aa65d9d73",
#     urls = ["https://github.com/rules-proto-grpc/rules_proto_grpc/archive/7ceb4ffaeaa9b3a4734bc7669b2a1c5aa65d9d73.tar.gz"],
# )

http_archive(
    name = "rules_proto_grpc",
    sha256 = "507e38c8d95c7efa4f3b1c0595a8e8f139c885cb41a76cab7e20e4e67ae87731",
    strip_prefix = "rules_proto_grpc-4.1.1",
    urls = ["https://github.com/rules-proto-grpc/rules_proto_grpc/archive/4.1.1.tar.gz"],
)

http_archive(
    name = "rules_proto",
    sha256 = "9850fcf6ad40fa348e6f13b2cfef4bb4639762f804794f2bf61d988f4ba0dae9",
    strip_prefix = "rules_proto-4.0.0-3.19.2-2",
    urls = [
        "https://github.com/bazelbuild/rules_proto/archive/refs/tags/4.0.0-3.19.2-2.tar.gz",
    ],
)

http_archive(
    name = "com_github_grpc_grpc",
    sha256 = "271bdc890bf329a8de5b65819f0f9590a5381402429bca37625b63546ed19e54",
    strip_prefix = "grpc-1.47.0",
    urls = ["https://github.com/grpc/grpc/archive/refs/tags/v1.47.0.tar.gz"],
)

http_archive(
    name = "com_google_absl",
    sha256 = "507ab77efa6b4c527c877707b7b184867ea809f769be2bb0ee4d548af477869d",
    strip_prefix = "abseil-cpp-b35ae3281ac7be49b42dc574403ff5fbcf1788fb",
    urls = ["https://github.com/abseil/abseil-cpp/archive/b35ae3281ac7be49b42dc574403ff5fbcf1788fb.zip"],  # master as of 2022-06-29
)

http_archive(
    name = "com_google_googletest",
    sha256 = "872e31f550e261936fc3aff1d0b5b454924466d4933aa4af1444af189fdd8598",
    strip_prefix = "googletest-d9335595b4cd73c8450c18e4dbf1ee0801c23c2e",
    urls = ["https://github.com/google/googletest/archive/d9335595b4cd73c8450c18e4dbf1ee0801c23c2e.zip"],  # master as of 2022-06-29
)

http_archive(
    name = "com_github_gperftools_gperftools",
    build_file = asphr_path + ":gperftools.BUILD",
    patch_args = ["-p1"],
    patches = [asphr_path + ":gperftools.patch"],  # random_shuffle not supported by c++20
    sha256 = "83e3bfdd28b8bcf53222c3798d4d395d52dadbbae59e8730c4a6d31a9c3732d8",
    strip_prefix = "gperftools-2.10",
    urls = ["https://github.com/gperftools/gperftools/releases/download/gperftools-2.10/gperftools-2.10.tar.gz"],
)

http_archive(
    name = "rules_cc",
    sha256 = "4dccbfd22c0def164c8f47458bd50e0c7148f3d92002cdb459c2a96a68498241",
    urls = ["https://github.com/bazelbuild/rules_cc/releases/download/0.0.1/rules_cc-0.0.1.tar.gz"],
)

http_archive(
    name = "rules_foreign_cc",
    sha256 = "62e364a05370059f07313ec46ae4205af23deb00e41f786f3233a98098c7e636",
    strip_prefix = "rules_foreign_cc-ae4ff42901354e2da8285dac4be8329eea2ea96a",
    url = "https://github.com/bazelbuild/rules_foreign_cc/archive/ae4ff42901354e2da8285dac4be8329eea2ea96a.tar.gz",  # v 0.7.1
    patch_args = ["-p1"],
    patches = [asphr_path + ":rules_foreign_cc.0.7.1.patch"],  # from https://github.com/bazelbuild/rules_foreign_cc/issues/859#issuecomment-1058361769
)

_RULES_BOOST_COMMIT = "ef58870fe00ecb8047cd34324b8c21221387d5fc"

http_archive(
    name = "com_github_nelhage_rules_boost",
    sha256 = "1557e4e1f2d009f14919dbf49b167f6616136d0cef1ca1cfada6ce0d4e3d6146",
    strip_prefix = "rules_boost-%s" % _RULES_BOOST_COMMIT,
    urls = [
        "https://github.com/nelhage/rules_boost/archive/%s.tar.gz" % _RULES_BOOST_COMMIT,
    ],
    patch_args = ["-p1"],
    patches = [asphr_path + ":rules_boost.patch"],  # issue: https://github.com/nelhage/rules_boost/issues/160 (on certain linux distros, clang cannot find backtrace.h. see https://www.boost.org/doc/libs/1_71_0/doc/html/stacktrace/configuration_and_build.html#stacktrace.configuration_and_build.f3)
)

# To find additional information on this release or newer ones visit:
# https://github.com/bazelbuild/rules_rust/releases
http_archive(
    name = "rules_rust",
    sha256 = "872b04538ca20dad94791c348623f079ba93daf274c1d57ae6bfe0930ec77f0d",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_rust/releases/download/0.6.0/rules_rust-v0.6.0.tar.gz",
        "https://github.com/bazelbuild/rules_rust/releases/download/0.6.0/rules_rust-v0.6.0.tar.gz",
    ],
)

http_archive(
    name = "cxx.rs",
    sha256 = "6fed9ef1c64a37c343727368b38c27fa4e15b27ca9924c6672a6a5496080c832",
    strip_prefix = "cxx-1.0.68",
    urls = ["https://github.com/dtolnay/cxx/archive/refs/tags/1.0.68.tar.gz"],
    patch_args = ["-p1"],
    patches = [asphr_path + ":cxx.patch"],  # issue: we want to derive custom traits in Rust that C++ doesn't necessarily need to know about!
)


load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")
load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")
load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies")
load("@rules_proto_grpc//js:repositories.bzl", rules_proto_grpc_js_repos = "js_repos")
load("@rules_rust//rust:repositories.bzl", "rules_rust_dependencies", "rust_register_toolchains")
load("@rules_rust//tools/rust_analyzer:deps.bzl", "rust_analyzer_deps")

bazel_toolchain_dependencies()

llvm_toolchain(
    name = "llvm_toolchain",
    llvm_version = "13.0.0",
)

rules_proto_grpc_js_repos()

rules_foreign_cc_dependencies(
    register_built_tools = True,
)

rules_proto_dependencies()

grpc_deps()

boost_deps()

rules_rust_dependencies()
rust_register_toolchains(version = RUST_VERSION, edition = "2021")
rust_analyzer_deps()

load("@rules_rust//crate_universe:repositories.bzl", "crate_universe_dependencies")

crate_universe_dependencies()

load("@rules_rust//crate_universe:crates_deps.bzl", "crate_repositories")

crate_repositories()

load("@cxx.rs//tools/bazel:vendor.bzl", rust_cxx_vendor = "vendor")
load("@rules_rust//crate_universe:defs.bzl", "crate", "crates_repository", "render_config")

crates_repository(
    name = "crate_index",
    lockfile = asphr_path + ":Cargo.Bazel.lock",
    packages = {
        "anyhow": crate.spec(
            version = "1.0",
        ),
        "diesel": crate.spec(
            version = "2.0.0-rc.0",
            features = ["sqlite", "returning_clauses_for_sqlite_3_35"],
        ),
        "diesel_migrations": crate.spec(
            version = "2.0.0-rc.0",
            features = ["sqlite"],
        ),
        "libsqlite3-sys": crate.spec(
            version = "0.24.2",
            features = ["bundled"],
        ),
        "rand": crate.spec(
            version = "0.8.5",
        ),
    },
    # Setting the default package name to `""` forces the use of the macros defined in this repository
    # to always use the root package when looking for dependencies or aliases. This should be considered
    # optional as the repository also exposes aliases for easy access to all dependencies.
    render_config = render_config(
        default_package_name = "",
    ),
)

rust_cxx_vendor(
    name = "third-party",
    cargo_version = RUST_VERSION,
    lockfile = "@cxx.rs//third-party:Cargo.lock",
)


load("@rules_proto//proto:repositories.bzl", "rules_proto_toolchains")
load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_toolchains")
load("@build_bazel_rules_nodejs//:index.bzl", "node_repositories", "yarn_install")
load("@llvm_toolchain//:toolchains.bzl", "llvm_register_toolchains")
load("@crate_index//:defs.bzl", "crate_repositories")

llvm_register_toolchains()

grpc_extra_deps()

rules_proto_toolchains()

rules_proto_grpc_toolchains()

# M1 Macs require Node 16+
# Note that anything that grpc tools currently fail on M1 macs. see this issue: https://github.com/grpc/grpc-node/issues/1405
# Hence, bazel build //... in asphr will fail on M1 macs. this is expected!
node_repositories(
    node_version = "16.6.2",
)
# it's fine if yarn_install fails on M1 macs; see above
yarn_install(
    name = "npm",
    package_json = asphr_path + "/schema:package.json",
    yarn_lock = asphr_path + "/schema:yarn.lock",
)

crate_repositories()

# docker stuff
http_archive(
    name = "io_bazel_rules_docker",
    sha256 = "59536e6ae64359b716ba9c46c39183403b01eabfbd57578e84398b4829ca499a",
    strip_prefix = "rules_docker-0.22.0",
    urls = ["https://github.com/bazelbuild/rules_docker/releases/download/v0.22.0/rules_docker-v0.22.0.tar.gz"],
)

load(
    "@io_bazel_rules_docker//repositories:repositories.bzl",
    container_repositories = "repositories",
)

container_repositories()

load(
    "@io_bazel_rules_docker//cc:image.bzl",
    _cc_image_repos = "repositories",
)

_cc_image_repos()

git_repository(
    name = "bazel_clang_tidy",
    commit = "69aa13e6d7cf102df70921c66be15d4592251e56",
    remote = "https://github.com/erenon/bazel_clang_tidy.git",
)

new_git_repository(
    name = "libpqxx",
    build_file = "//:libpqxx.BUILD",
    commit = "9100bef4b7488d05d414bd5f58a6811f4dae636e",
    remote = "https://github.com/jtv/libpqxx",
)
