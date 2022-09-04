//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "asphr/asphr.hpp"

// Cap invitation length at 4KB to make sure we don't get things that are too
// big
constexpr int MAX_INVITATION_LENGTH = 4096;

constexpr int MAX_ASYNC_INVITATION_BATCH_SIZE = 2000;