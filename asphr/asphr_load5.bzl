#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

"""Load asphr repos (part 5)."""


load(":asphr_load.bzl", "RUST_VERSION")

def load_asphr_repos5(asphr_path):
    """Loads the remaining repositories for the asphr project (those that depend on load_asphr_repos4())."""

