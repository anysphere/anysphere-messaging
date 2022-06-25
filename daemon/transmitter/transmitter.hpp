//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "asphr/asphr.hpp"
#include "daemon/crypto/crypto.hpp"
#include "daemon/db/db.hpp"
#include "daemon/global.hpp"
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

  // used to crawl the db
  int next_async_friend_request_retrieve_index;

  // for each index, get the PIR response for that index
  auto batch_retrieve_pir(FastPIRClient& client, vector<pir_index_t> indices)
      -> vector<asphr::StatusOr<asphrserver::ReceiveMessageResponse>>;

  auto encrypt_ack_row(const vector<db::OutgoingAck>& acks,
                       const string& write_key) -> asphr::StatusOr<pir_value_t>;

  // transmit async friend request to the server
  // we must reencrypt each round, to avoid
  // replaying the same message to the server
  auto transmit_async_friend_request() -> void;

  // retrieve and process async friend request from the server
  // and push them to the daemon
  // It is important to define the behavior of this function in the case of
  // duplicate requests. i.e. when a friend (request) with the same public key
  // is already in the database. Here's the definition for now.
  // 1. If the friend is marked as deleted, then we ignore the request.
  // 2. If the friend is marked as accepted, then we ignore the request.
  // 3. If the friend is marked as incoming, then we ignore the request.
  // 4. If the friend is marked as outgoing, then we approve this request
  // immediately.
  auto retrieve_async_friend_request(int start_index, int end_index) -> void;

  auto check_rep() const noexcept -> void;
};