#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

"""Load asphr repos."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")



def load_asphr_repos(asphr_path):
    """Loads the repositories for the asphr project.

    Args:
        asphr_path: The path to the asphr repository.
    """

