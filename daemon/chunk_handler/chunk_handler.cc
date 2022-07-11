#include "chunk_handler.hpp"

namespace chunk_handler {

auto chunks_to_msg(rust::Vec<uint8_t> chunks) -> chunk_handler::MsgProto {
  auto empty_vec = rust::Vec<rust::String>();
  auto empty_msg = rust::String();
  return chunk_handler::MsgProto{.other_recipients = empty_vec,
                                 .msg = empty_msg};
}

}  // namespace chunk_handler