#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make_variant")

package(
    default_visibility = ["//visibility:public"],
)

filegroup(
    name = "all_srcs",
    srcs = glob(["libsodium/**"]),
    visibility = ["//:__pkg__"],
)

configure_make_variant(
    name = "libsodium",
    args = ["-j 16"],
    # libevent script uses it's own libtool for linking;
    # so do not specify linker tool for it
    # (otherwise, if the libtool from bazel's toolchain is supplied,
    # the build script has problems with passing output file to libtool)
    # this fixes builds on mac
    env = {
        "AR": "",
    },
    lib_source = ":all_srcs",
    out_static_libs = [
        "libsodium.a",
    ],
    toolchain = "@rules_foreign_cc//toolchains:preinstalled_ninja_toolchain",
)
