#!/usr/bin/env bash

#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: MIT
#

if [[ $(diesel --version || true) != *"diesel 2.0.0-rc.0"* ]]; then
	echo "Installing Diesel CLI..."
	cargo install diesel_cli --version 2.0.0-rc.0 --no-default-features --features sqlite
fi
if [[ $(sd --version || true) != *"sd"* ]]; then
	echo "Installing sd..."
	cargo install sd
fi

DIESEL_CONFIG_FILE="${ANYSPHEREROOT}"/client/daemon/diesel.toml DATABASE_URL="${ANYSPHEREROOT}"/client/daemon/test.db diesel "$@"

if [[ $* == *"migration"* ]]; then
	echo "Replacing Timestamp with BigInt (UNIX microseconds stored as i64s)"
	sd Timestamp BigInt "${ANYSPHEREROOT}"/client/daemon/db/schema.rs
fi
