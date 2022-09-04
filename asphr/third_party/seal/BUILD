#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: MIT
#

load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

package(
    default_visibility = ["//visibility:public"],
)

filegroup(
    name = "all_srcs",
    srcs = glob(["SEAL/**"]),
    visibility = ["//:__pkg__"],
)

cmake(
    name = "seal",
    build_args = [
        "--verbose",
    ],
    cache_entries = {
        "CMAKE_C_FLAGS": "-fPIC",
        "CMAKE_BUILD_TYPE": "Release",
        "BUILD_SHARED_LIBS": "OFF",
        # our security guarantees derive entirely from the client code. what the server is doing
        # does not matter at all — including if it multiplies by 0! the server gains no extra information,
        # and an external party gets no information it couldn't get otherwise (it knows that certain database
        # elements are 0, but it could have just gotten that by querying the server on its own). hence, as long
        # as we are careful in the client not to subtract a ciphertext from itself, or multiply by 0 (which we
        # never do; we never do any evaluation at all!) this is completely fine!
        #
        # in fact, we do not even need this long explanation at all. the security proof is trivial: the index
        # of retrieval is encrypted, and hence, the server, or anyone else, will never know the index of retrieval,
        # regardless of what else happens.
        #
        # tl;dr: this setting only applies when you trust the evaluator, which we don't. our security guarantees
        # come from other places. this is compromising nothing.
        "SEAL_THROW_ON_TRANSPARENT_CIPHERTEXT": "OFF",
        # We need the `requires-network` tag for this to work. I can't believe I spent 48 hours debugging something
        # that can be solved by a simple tag.
        "SEAL_BUILD_DEPS": "ON",
        # TODO(sualeh): turn this ON for performance, later. it is way too hard to get this right.
        "SEAL_USE_MSGSL": "ON",
        # for some reason it finds ZLIB..... idk why
        "SEAL_USE_ZLIB": "ON",
        # TODO(sualeh): turn this ON for performance, later. it is way too hard to get this right.
        "SEAL_USE_ZSTD": "ON",
    },
    # generate_args = [
    #     "-G Ninja",
    # ],
    lib_source = ":all_srcs",
    out_static_libs = [
        "libseal-3.7.a",
    ],
    tags = ["requires-network"],
    # toolchain = "@rules_foreign_cc//toolchains:preinstalled_ninja_toolchain",
)
