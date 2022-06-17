//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "asphr/asphr.hpp"
#include "config.hpp"
#include "crypto.hpp"
#include "inbox.hpp"
#include "msgstore.hpp"
#include "outbox.hpp"

/**
 * @brief Transmitter manages sending and receiving messages.
 *
 * It owns an Inbox and an Outbox, and gets a shared_ptr to the Config and the
 * Msgstore.
 *
 * It is NOT threadsafe.
 */
class Transmitter {
 public:
  Transmitter(const Crypto crypto, shared_ptr<Config> config,
              shared_ptr<asphrserver::Server::Stub> stub,
              shared_ptr<Msgstore> msgstore);

  auto retrieve_messages() -> void;

  auto send_messages() -> void;

 private:
  const Crypto crypto;
  shared_ptr<Config> config;
  shared_ptr<asphrserver::Server::Stub> stub;

  shared_ptr<Msgstore> msgstore;

  Inbox inbox;
  Outbox outbox;

  string just_sent_friend;
  string previous_success_receive_friend;
  Time last_ui_timestamp;

  // for each index, get the PIR response for that index
  auto batch_retrieve_pir(FastPIRClient& client, vector<pir_index_t> indices)
      -> vector<asphr::StatusOr<asphrserver::ReceiveMessageResponse>>;

  // transmit async friend request to the server
  // we must reencrypt each round, to avoid
  auto transmit_async_friend_request() -> void;

  auto check_rep() const noexcept -> void;
};