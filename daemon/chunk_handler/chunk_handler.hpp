//
// copyright 2022 anysphere, inc.
// spdx-license-identifier: gpl-3.0-only
//

#pragma once

#include "asphr/asphr.hpp"
#include "daemon/db/db.rs.h"
#include "rust/cxx.h"

namespace chunk_handler {

struct MsgProto;

auto chunks_to_msg(rust::Vec<uint8_t> chunks) -> chunk_handler::MsgProto;

}  // namespace chunk_handler