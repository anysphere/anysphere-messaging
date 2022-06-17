//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "../crypto/crypto.hpp"
#include "asphr/asphr.hpp"
#include "schema/server.grpc.pb.h"

struct InboxMessage {
  std::string message;
  std::string friend_name;
  uint32_t id;
};

/**
 * @brief Inbox stores only messages for which all parts haven't been received
 * yet.
 *
 * It is NOT threadsafe.
 */
class Inbox {
 public:
  Inbox(const Global& G);

  // returns the ACKs that should be sent from the current user to their index
  auto get_encrypted_acks(const vector<Friend>& friends, const Friend& dummyMe)
      -> asphr::StatusOr<pir_value_t>;

  // receives the ack value for a particular friend, parses it, and
  // modifies friend_info to store the latest ack id!
  auto update_ack_from_friend(Config& config, pir_value_t& pir_acks,
                              const Friend& friend_info) -> bool;

  // receives a message! returns a message if all of its chunks have been
  // received, and it's a real message.
  auto receive_message(FastPIRClient& client,
                       const asphrserver::ReceiveMessageResponse& reply,
                       const Friend& friend_info_in,
                       string* previous_success_receive_friend)
      -> std::optional<InboxMessage>;

 private:
  const Global& G;

  auto check_rep() const noexcept -> void;
};