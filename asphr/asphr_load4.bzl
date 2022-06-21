#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

"""Load asphr repos (part 4)."""

load("@rules_rust//crate_universe:crates_deps.bzl", "crate_repositories")

def load_asphr_repos4():
    """Loads the remaining repositories for the asphr project (those that depend on load_asphr_repos3())."""

    crate_repositories()
