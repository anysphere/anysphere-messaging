//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "../crypto/crypto.hpp"
#include "../global.hpp"
#include "asphr/asphr.hpp"
#include "inbox.hpp"
#include "outbox.hpp"
#include "pir/fast_pir/fast_pir_client.hpp"

/**
 * @brief Transmitter manages sending and receiving messages.
 *
 * It owns an Inbox and an Outbox.
 *
 * It is NOT threadsafe.
 */
class Transmitter {
 public:
  Transmitter(const Global& G, shared_ptr<asphrserver::Server::Stub> stub);

  auto retrieve_messages() -> void;

  auto send_messages() -> void;

 private:
  const Global& G;
  shared_ptr<asphrserver::Server::Stub> stub;

  // We cache the pir_client here, because it takes some time to create it.
  std::unique_ptr<FastPIRClient> cached_pir_client = nullptr;
  string cached_pir_client_secret_key = "";

  Inbox inbox;
  Outbox outbox;

  string just_sent_friend;
  string previous_success_receive_friend;
  Time last_ui_timestamp;

  // for each index, get the PIR response for that index
  auto batch_retrieve_pir(FastPIRClient& client, vector<pir_index_t> indices)
      -> vector<asphr::StatusOr<asphrserver::ReceiveMessageResponse>>;

  auto check_rep() const noexcept -> void;
};