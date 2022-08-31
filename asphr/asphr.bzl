#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

"""Setup asphr."""

load("@rules_proto//proto:repositories.bzl", "rules_proto_toolchains")
load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_toolchains")
load("@build_bazel_rules_nodejs//:index.bzl", "node_repositories", "yarn_install")
load("@llvm_toolchain//:toolchains.bzl", "llvm_register_toolchains")
load("@crate_index//:defs.bzl", "crate_repositories")

def setup_asphr(asphr_path):
    """Setup the asphr repository

    Args:
        asphr_path: The path to the asphr repository.
    """
