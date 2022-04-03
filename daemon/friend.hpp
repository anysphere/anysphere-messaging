//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "asphr/asphr.hpp"
#include "constants.hpp"

class Friend {
 public:
  Friend(const string& name, const vector<Friend>& friends,
         const string add_key)
      : name(name),
        read_index(0),
        add_key(add_key),
        read_key(""),
        write_key(""),
        ack_index(0),
        enabled(false),
        latest_ack_id(0),
        latest_send_id(0),
        last_receive_id(0),
        dummy(false) {
    auto rng = std::default_random_engine{};

    auto all_ack_indexes_not_used = asphr::unordered_set<int>{};
    for (size_t i = 0; i < MAX_FRIENDS; i++) {
      all_ack_indexes_not_used.insert(i);
    }

    // get all the ack_indexes from the FriendTable
    for (const auto& f : friends) {
      all_ack_indexes_not_used.erase(f.ack_index);
    }

    // get a random ack_index that is not used
    std::uniform_int_distribution<int> dist(
        0, all_ack_indexes_not_used.size() - 1);
    auto random_ack_index = dist(rng);
    // TODO: do this a better way
    ack_index = *std::next(all_ack_indexes_not_used.begin(), random_ack_index);

    check_rep();
  }

  // Be very careful when changing the order of these parameters... please
  // search for all usages.
  Friend(const string& name, const int read_index, const string& add_key,
         const string& read_key, const string& write_key, const int ack_index,
         const bool enabled, const uint32_t latest_ack_id,
         const uint32_t latest_send_id, const uint32_t last_receive_id,
         bool dummy)
      : name(name),
        read_index(read_index),
        add_key(add_key),
        read_key(read_key),
        write_key(write_key),
        ack_index(ack_index),
        enabled(enabled),
        latest_ack_id(latest_ack_id),
        latest_send_id(latest_send_id),
        last_receive_id(last_receive_id),
        dummy(dummy) {
    check_rep();
  }

  Friend(const Friend& f) = default;
  Friend& operator=(const Friend& f) = default;

  string name;
  pir_index_t read_index;
  string add_key;
  string read_key;
  string write_key;
  // ack_index is the index into the acking data for this friend
  // this NEEDS to be unique for every friend!!
  // This needs to be between 0 <= ack_index < MAX_FRIENDS
  int ack_index;
  bool enabled;
  // latest_ack is the latest ID that was ACKed by the friend
  // any message with ID > latest_ack_id MUST be retried. note that
  // this refers to ID in the sequence number space, not the message ID space.
  uint32_t latest_ack_id;
  // latest_send_id is the latest full message ID that was sent to the friend
  // note that this is not a chunked ID, but the full message ID
  // hence, it might be that latest_ack_id > latest_send_id, even if that seems
  // weird
  uint32_t latest_send_id;
  // last_receive_id is the value that should be ACKed. we acknowledge that we
  // have received all IDs up to and including this value. Note that this refers
  // to ID in the sequence_number space, not the message ID space.
  uint32_t last_receive_id;
  // dummy is true if the friend is a dummy friend!
  bool dummy;

  auto to_json() -> asphr::json;
  static auto from_json(const asphr::json& j) -> Friend;

 private:
  auto check_rep() const -> void;
};