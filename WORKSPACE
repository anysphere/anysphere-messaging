#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

local_repository(
    name = "asphr",
    path = "../asphr",
)

load("@asphr//:asphr_load.bzl", "load_asphr_repos")

load_asphr_repos()

load("@asphr//:asphr_load2.bzl", "load_asphr_repos2")

load_asphr_repos2()

load("@asphr//:asphr.bzl", "setup_asphr")

setup_asphr("@asphr")
