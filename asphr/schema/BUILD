#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@rules_proto_grpc//js:defs.bzl", "js_grpc_node_library")

package(
    default_visibility = ["//visibility:public"],
)

# The following three rules demonstrate the usage of the cc_grpc_library rule in
# in a mode compatible with the native proto_library and cc_proto_library rules.
proto_library(
    name = "server_proto",
    srcs = ["server.proto"],
)

cc_proto_library(
    name = "server_proto_cc",
    deps = [":server_proto"],
)

cc_grpc_library(
    name = "server_cc_grpc",
    srcs = [":server_proto"],
    grpc_only = True,
    deps = [":server_proto_cc"],
)

proto_library(
    name = "message_proto",
    srcs = ["message.proto"],
)

cc_proto_library(
    name = "message_proto_cc",
    deps = [":message_proto"],
)

proto_library(
    name = "daemon_proto",
    srcs = ["daemon.proto"],
    deps = ["@com_google_protobuf//:timestamp_proto"],
)

cc_proto_library(
    name = "daemon_proto_cc",
    deps = [":daemon_proto"],
)

cc_grpc_library(
    name = "daemon_cc_grpc",
    srcs = [":daemon_proto"],
    grpc_only = True,
    deps = [":daemon_proto_cc"],
)

js_grpc_node_library(
    name = "daemon_js_grpc",
    protos = [":daemon_proto"],
)
