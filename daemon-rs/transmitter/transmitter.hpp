//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "../crypto/crypto.hpp"
#include "../db.hpp"
#include "../global.hpp"
#include "asphr/asphr.hpp"
#include "pir/fast_pir/fast_pir_client.hpp"

/**
 * @brief Transmitter manages sending and receiving messages.
 *
 * It is NOT threadsafe.
 */
class Transmitter {
 public:
  Transmitter(const Global& G, shared_ptr<asphrserver::Server::Stub> stub);

  auto retrieve() -> void;

  auto send() -> void;

 private:
  const Global& G;
  shared_ptr<asphrserver::Server::Stub> stub;

  // We cache the pir_client here, because it takes some time to create it.
  std::optional<FastPIRClient> cached_pir_client;
  std::optional<string> cached_pir_client_secret_key;

  // We create a dummy-address which we send to whenever we don't have an
  // actual friend to send to. This is critically important in order to
  // not leak metadata!!
  std::optional<db::Address> dummy_address;

  // Cached values that are not necessary, but might be useful for
  // optimizations.
  // TODO: do we want to get rid of this optimization, because it relies on the
  // trusted third friend assumption?
  std::optional<int> just_sent_friend;
  std::optional<int> previous_success_receive_friend;

  // for each index, get the PIR response for that index
  auto batch_retrieve_pir(FastPIRClient& client, vector<pir_index_t> indices)
      -> vector<asphr::StatusOr<asphrserver::ReceiveMessageResponse>>;

  auto check_rep() const noexcept -> void;
};