#!/usr/bin/env bash

#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: GPL-3.0-only
#

XDG_CONFIG_HOME=${HOME}/.anysphere2/data XDG_RUNTIME_DIR=${HOME}/.anysphere2/run XDG_CACHE_HOME=${HOME}/.anysphere2/cache $@
