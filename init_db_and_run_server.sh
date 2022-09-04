#!/usr/bin/env bash

#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: MIT
#

PGPASSWORD="$2" psql -U postgres -d postgres -h "$1" -f schema.sql
./as_server -d "$1" -p "$2"
