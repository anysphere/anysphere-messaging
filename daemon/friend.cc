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
                     {"write_key", asphr::Base64Escape(write_key)},
                     {"read_key", asphr::Base64Escape(read_key)},
                     {"ack_index", ack_index},
                     {"enabled", enabled},
                     {"latest_ack_id", latest_ack_id},
                     {"last_receive_id", last_receive_id}};
}

auto Friend::from_json(const asphr::json& j) -> Friend {
  Friend f;
  f.name = j.at("name").get<string>();
  f.read_index = j.at("read_index").get<int>();
  asphr::Base64Unescape(j.at("write_key").get<string>(), &f.write_key);
  asphr::Base64Unescape(j.at("read_key").get<string>(), &f.read_key);
  f.ack_index = j.at("ack_index").get<int>();
  f.enabled = j.at("enabled").get<bool>();
  f.latest_ack_id = j.at("latest_ack_id").get<uint32_t>();
  f.last_receive_id = j.at("last_receive_id").get<uint32_t>();
  f.check_rep();
  return f;
}

auto Friend::check_rep() const -> void {
  assert(!name.empty());
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