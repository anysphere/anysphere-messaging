#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

"""Load asphr repos."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

RUST_VERSION = "1.61.0"

def load_asphr_repos(asphr_path):
    """Loads the repositories for the asphr project.

    Args:
        asphr_path: The path to the asphr repository.
    """

    BAZEL_TOOLCHAIN_TAG = "0.6.3"

    BAZEL_TOOLCHAIN_SHA = "da607faed78c4cb5a5637ef74a36fdd2286f85ca5192222c4664efec2d529bb8"

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
        build_file = asphr_path + "//:gperftools.BUILD",
        patch_args = ["-p1"],
        patches = [asphr_path + "//:gperftools.patch"],  # random_shuffle not supported by c++20
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
        patches = [asphr_path + "//:rules_foreign_cc.0.7.1.patch"],  # from https://github.com/bazelbuild/rules_foreign_cc/issues/859#issuecomment-1058361769
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
        patches = [asphr_path + "//:rules_boost.patch"],  # issue: https://github.com/nelhage/rules_boost/issues/160 (on certain linux distros, clang cannot find backtrace.h. see https://www.boost.org/doc/libs/1_71_0/doc/html/stacktrace/configuration_and_build.html#stacktrace.configuration_and_build.f3)
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
        patches = [asphr_path + "//:cxx.patch"],  # issue: we want to derive custom traits in Rust that C++ doesn't necessarily need to know about!
    )
