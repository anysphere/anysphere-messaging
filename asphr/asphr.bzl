#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

"""Setup asphr."""

load("@bazel_tools//tools/cpp:cc_configure.bzl", "cc_configure")
load("@rules_proto//proto:repositories.bzl", "rules_proto_toolchains")
load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_toolchains")
load("@build_bazel_rules_nodejs//:index.bzl", "yarn_install")

def setup_asphr(asphr_path):
    """Setup the asphr repository

    Args:
        asphr_path: The path to the asphr repository.
    """

    cc_configure()

    grpc_extra_deps()

    rules_proto_toolchains()

    rules_proto_grpc_toolchains()

    yarn_install(
        name = "npm",
        package_json = asphr_path + "//schema:package.json",
        yarn_lock = asphr_path + "//schema:yarn.lock",
    )
