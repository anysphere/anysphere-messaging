//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "asphr/asphr.hpp"
#include "config.hpp"
#include "crypto.hpp"
#include "inbox.hpp"
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
              shared_ptr<asphrserver::Server::Stub> stub);

  auto retrieve_messages() -> void;

  auto send_messages() -> void;

 private:
  const Crypto crypto;
  shared_ptr<Config> config;
  shared_ptr<asphrserver::Server::Stub> stub;

  Inbox inbox;
  Outbox outbox;

  string just_sent_friend;
  string previous_success_receive_friend;
  Time last_ui_timestamp;

  auto check_rep() const noexcept -> void;
};