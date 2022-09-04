#!/usr/bin/env bash

#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: MIT
#

# Prepend this script to any command to have it use the .anysphere3 directory as its source of truth.
# Useful to test two people on the same computer.
# E.g.: ./wrap3.sh ./bazel-bin/daemon/daemon

XDG_CONFIG_HOME=${HOME}/.anysphere3/data XDG_RUNTIME_DIR=${HOME}/.anysphere3/run XDG_CACHE_HOME=${HOME}/.anysphere3/cache "$@"
