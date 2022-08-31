#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

"""Load asphr repos (part 3)."""

load("@rules_rust//crate_universe:repositories.bzl", "crate_universe_dependencies")

def load_asphr_repos3():
    """Loads the remaining repositories for the asphr project (those that depend on load_asphr_repos2())."""

