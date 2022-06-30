#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

"""Load asphr repos (part 5)."""

load("@cxx.rs//tools/bazel:vendor.bzl", rust_cxx_vendor = "vendor")
load("@rules_rust//crate_universe:defs.bzl", "crate", "crates_repository", "render_config")
load(":asphr_load.bzl", "RUST_VERSION")

def load_asphr_repos5(asphr_path):
    """Loads the remaining repositories for the asphr project (those that depend on load_asphr_repos4())."""

    crates_repository(
        name = "crate_index",
        lockfile = asphr_path + "//:Cargo.Bazel.lock",
        packages = {
            "anyhow": crate.spec(
                version = "1.0",
            ),
            "diesel": crate.spec(
                version = "2.0.0-rc.0",
                features = ["sqlite", "returning_clauses_for_sqlite_3_35"],
            ),
            "diesel_migrations": crate.spec(
                version = "2.0.0-rc.0",
                features = ["sqlite"],
            ),
            "libsqlite3-sys": crate.spec(
                version = "0.24.2",
                features = ["bundled"],
            ),
            "rand": crate.spec(
                version = "0.8.5",
            ),
        },
        # Setting the default package name to `""` forces the use of the macros defined in this repository
        # to always use the root package when looking for dependencies or aliases. This should be considered
        # optional as the repository also exposes aliases for easy access to all dependencies.
        render_config = render_config(
            default_package_name = "",
        ),
    )

    rust_cxx_vendor(
        name = "third-party",
        cargo_version = RUST_VERSION,
        lockfile = "@cxx.rs//third-party:Cargo.lock",
    )
