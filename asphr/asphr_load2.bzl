#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

"""Load asphr repos (part 2)."""

load("@rules_proto_grpc//js:repositories.bzl", rules_proto_grpc_js_repos = "js_repos")
load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies")
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

def load_asphr_repos2():
    """Loads the remaining repositories for the asphr project (those that depend on load_asphr_repos())."""

    rules_proto_grpc_js_repos()

    # we don't register built tools here because make_tool currently fails to build on Mac
    # see this issue: https://github.com/bazelbuild/rules_foreign_cc/issues/859
    rules_foreign_cc_dependencies(register_built_tools = False)

    rules_proto_dependencies()

    grpc_deps()
