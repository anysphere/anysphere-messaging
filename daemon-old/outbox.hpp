//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "asphr/asphr.hpp"
#include "config.hpp"
#include "msgstore.hpp"
#include "schema/message.pb.h"

struct MessageToSend {
  Friend to;
  uint32_t sequence_number;
  string msg;
  bool chunked;
  uint32_t num_chunks;
  uint32_t chunks_start_id;
  string full_message_id;

  auto to_proto() -> asphrclient::Message {
    asphrclient::Message message;
    message.set_id(sequence_number);
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

/**
 * @brief Outbox stores message chunks that have not been received by their
 * recipients yet.
 *
 * It is NOT threadsafe.
 */
class Outbox {
 public:
  Outbox(const string& file_address, shared_ptr<Msgstore> msgstore);
  Outbox(const asphr::json& serialized_json, const string& file_address,
         shared_ptr<Msgstore> msgstore);

  // the message here can be any size! Outbox takes care of splitting it into
  // chunks.
  // if a message with the same id is already in the outbox, nothing happens
  auto add(const string& id, const string& message,
           const Friend& friend_info) noexcept -> void;

  // returns a pair of (message, to_friend_name)
  // the prioritization of messages guarantees eventual delivery
  // only returns a message sent to a friend if the friend is enabled
  // if there are no real messages, returns a message sent to
  // registrationInfo.name
  auto message_to_send(const Config& config, const Friend& dummyMe)
      -> MessageToSend;

 private:
  shared_ptr<Msgstore> msgstore;

  const string saved_file_address;
  // stores a mapping from friend -> message to send. the vector is sorted by
  // ID, such that the first element has the lowest ID, i.e., is the first that
  // should be sent
  std::unordered_map<string, std::vector<MessageToSend>> outbox;
  // stores the IDs of the messages currently in the outbox
  std::unordered_set<string> outbox_ids;

  absl::BitGen rand_bitgen_;

  auto check_rep() const noexcept -> void;
  auto save() noexcept(false) -> void;
};