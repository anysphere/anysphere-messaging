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

  // transmit async friend request to the server
  // we must reencrypt each round, to avoid
  // replaying the same message to the server
  auto transmit_async_invitation() -> void;

  // retrieve and process async friend request from the server
  // and push them to the database
  // It is important to define the behavior of this function in the case of
  // duplicate requests. i.e. when a friend (request) with the same public key
  // is already in the database. Here's the definition for now.
  // 1. If the friend is marked as deleted, then we ignore the request.
  // 2. If the friend is marked as accepted, then we ignore the request.
  // 3. If the friend is marked as incoming, then we ignore the request.
  // 4. If the friend is marked as outgoing, then we approve this request
  // immediately.
  auto retrieve_async_invitations(int start_index, int end_index) -> void;

  // method for testing
  // because during microtests, we do not want to full db scan
  auto reset_async_scanner(int index) {
    next_async_invitation_retrieve_index = index;
  }
  auto update_async_invitation_retrieve_index() -> pair<int, int> {
    int start_index = next_async_invitation_retrieve_index;
    int end_index = std::min(
        next_async_invitation_retrieve_index + ASYNC_FRIEND_REQUEST_BATCH_SIZE,
        CLIENT_DB_ROWS);
    return make_pair(start_index, end_index);
  }

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
  optional<db::OutgoingAsyncInvitation> dummy_outgoing_invitation;

  // some caching work the first time we set up, setting up the things above
  auto setup_registration_caching() -> void;

  // Cached values that are not necessary, but might be useful for
  // optimizations. Heuristics.
  // TODO: do we want to get rid of this optimization, because it relies on the
  // trusted third friend assumption?
  optional<int> just_sent_friend;
  optional<int> previous_success_receive_friend;
  optional<int> just_acked_friend;

  // used to crawl the db for the async invitations
  int next_async_invitation_retrieve_index;

  // for each index, get the PIR response for that index
  auto batch_retrieve_pir(FastPIRClient& client, vector<pir_index_t> indices)
      -> vector<asphr::StatusOr<asphrserver::ReceiveMessageResponse>>;

  auto encrypt_ack_row(const vector<db::OutgoingAck>& acks,
                       const string& write_key) -> asphr::StatusOr<pir_value_t>;

  auto check_rep() const noexcept -> void;
};