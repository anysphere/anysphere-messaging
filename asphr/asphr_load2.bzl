#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

"""Load asphr repos (part 2)."""

load("@rules_proto_grpc//js:repositories.bzl", rules_proto_grpc_js_repos = "js_repos")
load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies")
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")
load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

def load_asphr_repos2():
    """Loads the remaining repositories for the asphr project (those that depend on load_asphr_repos())."""

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
