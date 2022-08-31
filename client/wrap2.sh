#!/usr/bin/env bash

#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

# Prepend this script to any command to have it use the .anysphere2 directory as its source of truth.
# Useful to test two people on the same computer.
# E.g.: ./wrap2.sh ./bazel-bin/daemon/daemon

XDG_CONFIG_HOME=${HOME}/.anysphere2/data XDG_RUNTIME_DIR=${HOME}/.anysphere2/run XDG_CACHE_HOME=${HOME}/.anysphere2/cache "$@"
