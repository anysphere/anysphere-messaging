//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//
// only for testing !!!

#pragma once

#include "asphr/asphr/asphr.hpp"

auto append_checksum(string data) -> string;
auto remove_checksum(string bytes) -> asphr::StatusOr<string>;

auto change_base(vector<int> values, std::function<int(int)> from_base,
                 std::function<int(int)> to_base) -> vector<int>;