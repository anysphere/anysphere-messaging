//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "transmitter.hpp"

#include <algorithm>

#include "schema/server.grpc.pb.h"

auto generate_dummy_address(const Global& G, const db::Registration& reg)
    -> db::Address {
  auto dummy_friend_keypair = crypto::generate_keypair();
  auto dummy_read_write_keys = crypto::derive_read_write_keys(
      std::string(reg.public_key), std::string(reg.private_key),
      dummy_friend_keypair.first);

  return (db::Address){-1, 0, 0, dummy_read_write_keys.first,
                       dummy_read_write_keys.second};
}

Transmitter::Transmitter(const Global& G,
                         shared_ptr<asphrserver::Server::Stub> stub)
    : G(G), stub(stub) {
  check_rep();
}

auto Transmitter::retrieve() -> void {
  check_rep();

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Not registered, so not retrieving messages.");
    return;
  }

  if (!cached_pir_client.has_value() ||
      cached_pir_client_secret_key.value() !=
          std::string(G.db->get_pir_secret_key())) {
    auto reg = G.db->get_registration();
    cached_pir_client = std::make_optional<FastPIRClient>(reg.pir_secret_key,
                                                          reg.pir_galois_key);
    cached_pir_client_secret_key = reg.pir_secret_key;
    dummy_address = generate_dummy_address(G, reg);
  }

  check_rep();

  auto& client = *cached_pir_client;

  // -----
  // Step 1: choose RECEIVE_FRIENDS_PER_ROUND friends to receive from!!
  // -----
  //    Priority 1: the friend that we just sent a message to
  std::vector<int> priority_receive_friends;
  if (just_sent_friend.has_value()) {
    priority_receive_friends.push_back(just_sent_friend.value());
  }
  //    Priority 2: the friend that we successfully received a new message
  //    from the previous round
  if (previous_success_receive_friend.has_value()) {
    if (std::find(priority_receive_friends.begin(),
                  priority_receive_friends.end(),
                  previous_success_receive_friend.value()) ==
        priority_receive_friends.end()) {
      priority_receive_friends.push_back(
          previous_success_receive_friend.value());
    }
  }

  // Get the proritized addresses if we can.
  vector<db::Address> receive_addresses;
  vector<bool> receive_addresses_is_dummy;
  for (auto i = 0;
       i < std::min(RECEIVE_FRIENDS_PER_ROUND, priority_receive_friends.size());
       i++) {
    try {
      auto f = G.db->get_friend_address(priority_receive_friends.at(i));
      receive_addresses.push_back(f);
      receive_addresses_is_dummy.push_back(false);
    } catch (const rust::Error& e) {
      continue;
    }
  }

  //    Last priority: random! let's fill in the rest of the indices
  auto receive_friends_old_size = receive_addresses.size();
  for (size_t i = 0; i < RECEIVE_FRIENDS_PER_ROUND - receive_friends_old_size;
       i++) {
    // note: we do not need cryptographic randomness here. randomness is only
    // for liveness
    try {
      auto f = G.db->get_random_enabled_friend_address_excluding(
          receive_addresses.map([](auto& a) { return a.uid; }));
      receive_addresses.push_back(f);
      receive_addresses_is_dummy.push_back(false);
    } catch (const rust::Error& e) {
      // dummy if no friends left :')
      receive_addresses.push_back(dummy_address);
      receive_addresses_is_dummy.push_back(true);
    }
  }
  assert(receive_addresses.size() == RECEIVE_FRIENDS_PER_ROUND);

  // -----
  // Step 2: execute the PIR queries
  // -----
  auto pir_replies = batch_retrieve_pir(
      client, receive_addresses.map([](auto& a) { return a.read_index; }));

  assert(pir_replies.size() == RECEIVE_FRIENDS_PER_ROUND);

  previous_success_receive_friend = std::nullopt;

  check_rep();

  // -----
  // Step 3: process the PIR replies, and store them in the database
  // -----
  for (auto i = 0; i < RECEIVE_FRIENDS_PER_ROUND; i++) {
    if (receive_addresses_is_dummy.at(i)) {
      continue;
    }
    auto& f = receive_addresses.at(i);
    if (pir_replies.at(i).ok()) {
      auto& reply = pir_replies.at(i).value();

      // Transactions and atomicity matter here.
      // Think carefully about what needs to be atomic when editing here.
      // Remember that atomicity can only happen in Rust.

      //
      // Step 3(a): Record the ACK in the database.
      //
      auto ack_answer = reply.pir_answer_acks();
      auto ack_answer_obj = client.answer_from_string(ack_answer);
      auto ack_decoded = client.decode(ack_answer_obj, f.read_index);

      G.db->receive_ack(f.uid, ack_decoded);

      //
      // Step 3(b): Record the message in the database.
      //
      auto answer = reply.pir_answer();
      auto answer_obj = client.answer_from_string(answer);
      auto decoded = client.decode(answer_obj, f.read_index);

      auto decrypted = crypto::decrypt_receive(decoded, f);
      if (decrypted.ok()) {
        auto& message = decrypted.value();

        G.db->receive_chunk(f.uid, message);
      } else {
        ASPHR_LOG_INFO(
            "Failed to decrypt message (message was probably not for us, which "
            "is okay).",
            decrypted_status, decrypted.status());
      }
    } else {
      ASPHR_LOG_ERR("Could not retrieve PIR reply from server.", friend_uid,
                    f.uid, server_status_code,
                    pir_replies.at(i).status().code(), server_status_message,
                    pir_replies.at(i).status().message());
    }
  }

  check_rep();
}

auto Transmitter::send_messages() -> void {
  check_rep();

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Not registered, so not sending messages.");
    return;
  }

  auto undelivered_messages =
      msgstore->get_undelivered_outgoing_messages_sorted();
  // we want to send messages in chronological order, not reverse chronological
  // order hence, we need to reverse this array
  std::reverse(undelivered_messages.begin(), undelivered_messages.end());

  auto authentication_token = config->registration_info().authentication_token;

  for (auto& undelivered_msg : undelivered_messages) {
    const auto friend_info_status = config->get_friend(undelivered_msg.to);
    if (!friend_info_status.ok()) {
      std::cerr << "FriendHashTable does not contain " << undelivered_msg.to
                << "; ignoring message" << endl;
      continue;
    }
    const auto friend_info = friend_info_status.value();
    outbox.add(undelivered_msg.id, undelivered_msg.message, friend_info);
  }

  auto messageToSend = outbox.message_to_send(*config, config->dummy_me());
  // TODO: update status in msgstore for this message so it can be shown in the
  // UI that now we're trying to send this

  // always send to the 0th index. currently we only support one index per
  // person!! in the future, this may change, so please don't assert that the
  // length is 1.
  auto index = config->registration_info().allocation.at(0);

  just_sent_friend = messageToSend.to.name;

  auto pir_value_message_status =
      crypto.encrypt_send(messageToSend.to_proto(), messageToSend.to);
  if (!pir_value_message_status.ok()) {
    cout << "encryption failed; not doing anything with message"
         << pir_value_message_status.status() << endl;
    return;
  }
  auto pir_value_message = pir_value_message_status.value();

  auto pir_encrypted_acks_status =
      inbox.get_encrypted_acks(config->friends(), crypto, config->dummy_me());
  if (!pir_encrypted_acks_status.ok()) {
    cout << "encryption failed; not doing anything with message"
         << pir_encrypted_acks_status.status() << endl;
    return;
  }
  auto pir_encrypted_acks = pir_encrypted_acks_status.value();

  cout << "Sending message to server: " << endl;
  cout << "index: " << index << endl;
  cout << "authentication_token: " << authentication_token << endl;
  check_rep();

  // call register rpc to send the register request
  asphrserver::SendMessageInfo request;

  request.set_index(index);
  request.set_authentication_token(authentication_token);

  string padded_msg_str = "";
  for (auto& c : pir_value_message) {
    padded_msg_str += c;
  }
  request.set_message(padded_msg_str);

  string padded_acks_str = "";
  for (auto& c : pir_encrypted_acks) {
    padded_acks_str += c;
  }
  request.set_acks(padded_acks_str);

  asphrserver::SendMessageResponse reply;

  grpc::ClientContext context;

  grpc::Status status = stub->SendMessage(&context, request, &reply);

  if (status.ok()) {
    std::cout << "Message sent to server!" << std::endl;
  } else {
    std::cerr << status.error_code() << ": " << status.error_message()
              << " details:" << status.error_details() << std::endl;
  }
  check_rep();
}

auto Transmitter::batch_retrieve_pir(FastPIRClient& client,
                                     vector<pir_index_t> indices)
    -> vector<asphr::StatusOr<asphrserver::ReceiveMessageResponse>> {
  // TODO: use batch PIR here!!! we don't want to do this many PIR requests
  vector<asphr::StatusOr<asphrserver::ReceiveMessageResponse>> pir_replies;

  for (auto& index : indices) {
    asphrserver::ReceiveMessageInfo request;

    // Remember: for security, CLIENT_DB_ROWS *needs* to be hardcoded
    auto query = client.query(index, CLIENT_DB_ROWS);

    auto serialized_query = query.serialize_to_string();

    request.set_pir_query(serialized_query);

    asphrserver::ReceiveMessageResponse reply;
    grpc::ClientContext context;

    grpc::Status status = stub->ReceiveMessage(&context, request, &reply);

    if (status.ok()) {
      pir_replies.push_back(reply);
    } else {
      pir_replies.push_back(absl::UnknownError(
          "ReceiveMessage RPC failed with error: " + status.error_message()));
    }
  }

  return pir_replies;
}

auto Transmitter::check_rep() const noexcept -> void {
  ASPHR_ASSERT(G.alive());

  ASPHR_ASSERT_EQ_MSG(
      cached_pir_client.has_value(), cached_pir_client_secret_key.has_value(),
      "cached_pir_client and cached_pir_client_status are not in sync");
  ASPHR_ASSERT_EQ_MSG(cached_pir_client.has_value(), dummy_address.has_value(),
                      "cached_pir_client and dummy_address are not in sync");
}