#pragma once

#include "asphr/asphr.hpp"
#include "config.hpp"
#include "schema/message.pb.h"

struct MessageToSend {
  Friend to;
  uint32_t id;
  string msg;
  bool chunked;
  uint32_t num_chunks;
  uint32_t chunks_start_id;

  auto to_proto() -> asphrclient::Message {
    asphrclient::Message message;
    message.set_id(id);
    message.set_msg(msg);
    if (chunked) {
      message.set_num_chunks(num_chunks);
      message.set_chunks_start_id(chunks_start_id);
    }
    return message;
  }

  auto to_json() -> asphr::json;
  static auto from_json(const asphr::json& j) -> MessageToSend;
};

// Outbox is ONLY concerned with outgoing messages.
class Outbox {
 public:
  Outbox(const string& file_address);
  Outbox(const asphr::json& serialized_json, const string& file_address);

  auto save() noexcept(false) -> void;

  // the message here can be any size! Outbox takes care of splitting it into
  // chunks.
  // modifies the friend to update the last_send_id
  auto add(const string& message, Friend& friend_info) noexcept -> void;

  // returns a pair of (message, to_friend_name)
  // the prioritization of messages guarantees eventual delivery
  // only returns a message sent to a friend if the friend is enabled
  // if there are no real messages, returns a message sent to
  // registrationInfo.name
  auto message_to_send(const std::unordered_map<string, Friend>& friendTable,
                       const Friend& dummyMe) -> MessageToSend;

 private:
  const string saved_file_address;
  // stores a mapping from friend -> message to send. the vector is sorted by
  // ID, such that the first element has the lowest ID, i.e., is the first that
  // should be sent
  std::unordered_map<string, std::vector<MessageToSend>> outbox;

  auto check_rep() const noexcept -> void;
};