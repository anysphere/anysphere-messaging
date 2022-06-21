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

  // temporary method for testing purposes
  auto set_async_friend_request_scan_index(int index) -> void {
    next_async_friend_request_retrieve_index = index;
  }

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

  int next_async_friend_request_retrieve_index;

  // for each index, get the PIR response for that index
  auto batch_retrieve_pir(FastPIRClient& client, vector<pir_index_t> indices)
      -> vector<asphr::StatusOr<asphrserver::ReceiveMessageResponse>>;

  // transmit async friend request to the server
  // we must reencrypt each round, to avoid
  auto transmit_async_friend_request() -> void;

  // retrieve and process async friend request from the server
  // and push them to an "inbox" in the daemon
  auto retrieve_async_friend_request(int start_index, int end_index)
      -> asphr::StatusOr<std::map<string, Friend>>;

  auto check_rep() const noexcept -> void;
};