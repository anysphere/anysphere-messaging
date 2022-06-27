#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "asphr",
<<<<<<< HEAD
    commit = "b56fda2adfb971289586e2dcd25911810f383fc9",  # autoupdate anysphere/asphr
=======
    commit = "b56fda2adfb971289586e2dcd25911810f383fc9",  # autoupdate anysphere/asphr
>>>>>>> origin/friends
    init_submodules = True,
    remote = "https://github.com/anysphere/asphr.git",
)
# for local builds, we add --override_repository=asphr=../asphr, which we do in setupgit.sh

load("@asphr//:asphr_load.bzl", "load_asphr_repos")

load_asphr_repos("@asphr")

load("@asphr//:asphr_load2.bzl", "load_asphr_repos2")

load_asphr_repos2()

load("@asphr//:asphr_load3.bzl", "load_asphr_repos3")

load_asphr_repos3()

load("@asphr//:asphr_load4.bzl", "load_asphr_repos4")

load_asphr_repos4()

load("@asphr//:asphr_load5.bzl", "load_asphr_repos5")

load_asphr_repos5("@asphr")

load("@asphr//:asphr.bzl", "setup_asphr")

setup_asphr("@asphr")
