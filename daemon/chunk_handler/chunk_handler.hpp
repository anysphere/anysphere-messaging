//
// copyright 2022 anysphere, inc.
// spdx-license-identifier: gpl-3.0-only
//

#pragma once

#include "asphr/asphr.hpp"
#include "daemon/chunk_handler/chunk_handler.rs.h"
#include "rust/cxx.h"

namespace chunk_handler {
inline auto chunks_to_msg(rust::Vec<uint8_t> chunks)
    -> chunk_handler::MsgProto {
  auto empty_vec = rust::Vec<rust::String>();
  auto empty_msg = rust::String();
  return chunk_handler::MsgProto{.other_recipients = empty_vec,
                                 .msg = empty_msg};
}

}  // namespace chunk_handler