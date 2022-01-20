#pragma once

#include "asphr/asphr.hpp"
#include "schema/message.pb.h"

// Outbox is ONLY concerned with outgoing messages.
class Outbox {
  Outbox(const string& file_address);
  Outbox(const asphr::json& serialized_json, const string& file_address);

  auto save() noexcept(false) -> void;

  // the message here can be any size! Outbox takes care of splitting it into
  // chunks.
  auto add(const string& message, const string& friend_name) noexcept -> void;

  auto ack(const string& friend_name, const int ack_id) noexcept -> void;

  // returns a pair of (message, to_friend_name)
  auto message_to_send() -> pair<asphrclient::Message, string>;
}