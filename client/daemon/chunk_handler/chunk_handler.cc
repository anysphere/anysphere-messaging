#include "chunk_handler.hpp"

#include "daemon/db/db.hpp"
#include "schema/message.pb.h"

namespace chunk_handler {

auto deserialize_message(rust::Vec<uint8_t> serialized)
    -> chunk_handler::WireMessage {
  asphrclient::Message message;
  auto status = message.ParseFromString(rust_u8Vec_to_string(serialized));
  if (!status) {
    throw std::invalid_argument("failed to parse message");
  }
  rust::Vec<rust::String> other_recipients;
  for (auto& recipient : message.other_recipients()) {
    other_recipients.push_back(recipient);
  }
  return chunk_handler::WireMessage{
      .other_recipients = other_recipients,
      .msg = message.msg(),
  };
}

auto serialize_message(chunk_handler::WireMessage message)
    -> rust::Vec<uint8_t> {
  asphrclient::Message msg;
  for (auto& recipient : message.other_recipients) {
    msg.add_other_recipients(string(recipient));
  }
  msg.set_msg(string(message.msg));
  return string_to_rust_u8Vec(msg.SerializeAsString());
}

}  // namespace chunk_handler