//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "asphr/asphr.hpp"
#include "daemon-rs/crypto/crypto.hpp"
#include "daemon-rs/db/src/db.hpp"
#include "daemon-rs/global.hpp"
#include "pir/fast_pir/fast_pir_client.hpp"
#include "schema/daemon.grpc.pb.h"
#include "schema/server.grpc.pb.h"

/**
 * @brief Transmitter manages sending and receiving messages.
 *
 * It is NOT threadsafe.
 */
class Transmitter {
 public:
  Transmitter(Global& G, shared_ptr<asphrserver::Server::Stub> stub);

  // may throw rust::Error if there is a database problem
  auto retrieve() -> void;

  // may throw rust::Error if there is a database problem
  auto send() -> void;

 private:
  Global& G;
  shared_ptr<asphrserver::Server::Stub> stub;

  // We cache the pir_client here, because it takes some time to create it.
  optional<unique_ptr<FastPIRClient>> cached_pir_client;
  optional<string> cached_pir_client_secret_key;

  // We create a dummy-address which we send to whenever we don't have an
  // actual friend to send to. This is critically important in order to
  // not leak metadata!!
  optional<db::Address> dummy_address;

  // some caching work the first time we set up, setting up the things above
  auto setup_registration_caching() -> void;

  // Cached values that are not necessary, but might be useful for
  // optimizations. Heuristics.
  // TODO: do we want to get rid of this optimization, because it relies on the
  // trusted third friend assumption?
  optional<int> just_sent_friend;
  optional<int> previous_success_receive_friend;
  optional<int> just_acked_friend;

  // for each index, get the PIR response for that index
  auto batch_retrieve_pir(FastPIRClient& client, vector<pir_index_t> indices)
      -> vector<asphr::StatusOr<asphrserver::ReceiveMessageResponse>>;

  auto encrypt_ack_row(const vector<db::OutgoingAck>& acks,
                       const string& write_key) -> asphr::StatusOr<pir_value_t>;

  auto check_rep() const noexcept -> void;
};