//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "asphr/asphr/asphr.hpp"
#include "client/daemon/db/db.rs.h"
#include "rust/cxx.h"

namespace chunk_handler {

struct WireMessage;

// both of these may throw
auto deserialize_message(rust::Vec<uint8_t> serialized)
    -> chunk_handler::WireMessage;
auto serialize_message(chunk_handler::WireMessage message)
    -> rust::Vec<uint8_t>;

}  // namespace chunk_handler