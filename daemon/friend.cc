//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "friend.hpp"

#include "crypto.hpp"

auto Friend::to_json() -> asphr::json {
  check_rep();
  return asphr::json{{"name", name},
                     {"read_index", read_index},
                     {"add_key", add_key},
                     {"write_key", asphr::Base64Escape(write_key)},
                     {"read_key", asphr::Base64Escape(read_key)},
                     {"ack_index", ack_index},
                     {"enabled", enabled},
                     {"latest_ack_id", latest_ack_id},
                     {"latest_send_id", latest_send_id},
                     {"last_receive_id", last_receive_id}};
}

auto Friend::from_json(const asphr::json& j) -> Friend {
  string read_key;
  string write_key;
  asphr::Base64Unescape(j.at("read_key").get<string>(), &read_key);
  asphr::Base64Unescape(j.at("write_key").get<string>(), &write_key);
  Friend f(j.at("name").get<string>(), j.at("read_index").get<int>(),
           j.at("add_key").get<string>(), read_key, write_key,
           j.at("ack_index").get<int>(), j.at("enabled").get<bool>(),
           j.at("latest_ack_id").get<uint32_t>(),
           j.at("latest_send_id").get<uint32_t>(),
           j.at("last_receive_id").get<uint32_t>(), false);
  return f;
}

auto Friend::check_rep() const -> void {
  assert(!name.empty());
  assert(add_key.size() > 0);
  assert(ack_index >= 0);
  assert(static_cast<size_t>(ack_index) < MAX_FRIENDS);
  if (enabled) {
    assert(read_index >= 0);
    assert(read_key.size() == crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    assert(write_key.size() == crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
  } else {
    // read_index must be valid! and index 0 will ALWAYS be a valid index.
    assert(read_index == 0);
  }
}