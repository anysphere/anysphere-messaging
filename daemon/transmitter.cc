//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "transmitter.hpp"

#include "schema/server.grpc.pb.h"

// TODO: make an actual directory structure here! do not just do giant .ndjson
// files
Transmitter::Transmitter(const Crypto crypto, shared_ptr<Config> config,
                         shared_ptr<asphrserver::Server::Stub> stub,
                         shared_ptr<Msgstore> msgstore)
    : crypto(crypto),
      config(config),
      stub(stub),
      msgstore(msgstore),
      inbox(config->data_dir_address() / "inbox.json"),
      outbox(config->data_dir_address() / "outbox.json", msgstore),
      next_async_friend_request_retrieve_index(0) {
  check_rep();
}

auto Transmitter::retrieve_messages() -> void {
  check_rep();
  if (!config->has_registered()) {
    cout << "hasn't registered yet, so cannot retrieve messages" << endl;
    return;
  }

  auto& client = config->pir_client();

  // choose RECEIVE_FRIENDS_PER_ROUND friends to receive from!!
  // priority 1: the friend that we just sent a message to
  vector<Friend> receive_friends;
  std::unordered_set<string> receive_friend_names;
  if (auto friend_info_status = config->get_friend(just_sent_friend);
      friend_info_status.ok()) {
    receive_friends.push_back(friend_info_status.value());
    receive_friend_names.insert(friend_info_status.value().name);
  }
  // priority 2: the friend that we successfully received a new message from the
  // previous round
  if (auto friend_info_status =
          config->get_friend(previous_success_receive_friend);
      friend_info_status.ok() &&
      previous_success_receive_friend != just_sent_friend) {
    receive_friends.push_back(friend_info_status.value());
    receive_friend_names.insert(friend_info_status.value().name);
  }
  // priority 3: random!
  auto receive_friends_old_size = receive_friends.size();
  for (size_t i = 0; i < RECEIVE_FRIENDS_PER_ROUND - receive_friends_old_size;
       i++) {
    // note: we do not need cryptographic randomness here. randomness is only
    // for liveness
    auto friend_info_status =
        config->random_enabled_friend(receive_friend_names);
    if (friend_info_status.ok()) {
      receive_friends.push_back(friend_info_status.value());
      assert(!receive_friend_names.contains(friend_info_status.value().name));
      receive_friend_names.insert(friend_info_status.value().name);
    } else {
      // dummy if no friends left :')
      receive_friends.push_back(config->dummy_me());
      receive_friend_names.insert(config->dummy_me().name);
    }
  }
  assert(receive_friends.size() == RECEIVE_FRIENDS_PER_ROUND);

  auto receive_friend_indices = vector<pir_index_t>(RECEIVE_FRIENDS_PER_ROUND);
  for (auto i = 0; i < RECEIVE_FRIENDS_PER_ROUND; i++) {
    receive_friend_indices.at(i) = receive_friends.at(i).read_index;
  }
  auto pir_replies = batch_retrieve_pir(client, receive_friend_indices);

  assert(pir_replies.size() == RECEIVE_FRIENDS_PER_ROUND);

  previous_success_receive_friend = "";

  check_rep();

  for (auto i = 0; i < RECEIVE_FRIENDS_PER_ROUND; i++) {
    auto& friend_info = receive_friends.at(i);
    if (friend_info.dummy) {
      continue;
    }
    if (pir_replies.at(i).ok()) {
      auto& reply = pir_replies.at(i).value();
      // TODO: inbox.receive_message and msgstore->add_incoming_message need to
      // be atomic!!!
      auto message_opt =
          inbox.receive_message(client, *config, reply, friend_info, crypto,
                                &previous_success_receive_friend);
      if (message_opt.has_value()) {
        auto message = message_opt.value();
        msgstore->add_incoming_message(message.id, message.friend_name,
                                       message.message);
      } else {
        cout << "no message received from " << friend_info.name << endl;
      }
    } else {
      cout << "could not retrieve message from " << friend_info.name << endl;
      cout << pir_replies.at(i).status().code() << ": "
           << pir_replies.at(i).status().message() << endl;
    }
  }

  // Crawl the async friend request database
  // TODO: currently, batch_size = 1,000
  // client_db_rows = 360,000
  // so it takes 360 rounds to scan the entire database
  // Is there anyway to make this more efficient?
  // We can compute that
  // Latency = CLIENT_DB_ROWS * SIZE / DOWNLOAD_RATE
  int start_index = next_async_friend_request_retrieve_index;
  int end_index = std::min(next_async_friend_request_retrieve_index +
                               ASYNC_FRIEND_REQUEST_BATCH_SIZE,
                           CLIENT_DB_ROWS);
  // call the server to retrieve the async friend requests
  auto async_friend_request_replies_status =
      retrieve_async_friend_request(start_index, end_index);
  if (!async_friend_request_replies_status.ok()) {
    // throw an error
    cerr << "could not retrieve async friend requests" << endl;
    cerr << async_friend_request_replies_status.status().code() << ": "
         << async_friend_request_replies_status.status().message() << endl;
    return;
  }
  auto async_friend_request_replies =
      async_friend_request_replies_status.value();
  // add them to the config
  config->add_incoming_async_friend_requests(async_friend_request_replies);
  // update the next_async_friend_request_retrieve_index
  if (end_index == CLIENT_DB_ROWS) {
    next_async_friend_request_retrieve_index = 0;
  } else {
    next_async_friend_request_retrieve_index = end_index;
  }
  check_rep();
}

auto Transmitter::send_messages() -> void {
  check_rep();
  if (!config->has_registered()) {
    cout << "hasn't registered yet, so don't send a message" << endl;
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

  transmit_async_friend_request();
  // check rep and finish
  check_rep();
}

auto Transmitter::batch_retrieve_pir(FastPIRClient& client,
                                     vector<pir_index_t> indices)
    -> vector<asphr::StatusOr<asphrserver::ReceiveMessageResponse>> {
  // TODO: use batch PIR here!!! we don't want to do this many PIR requests
  vector<asphr::StatusOr<asphrserver::ReceiveMessageResponse>> pir_replies;

  for (auto& index : indices) {
    asphrserver::ReceiveMessageInfo request;

    auto query = client.query(index, config->db_rows());

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

//----------------------------------------------------------------
//----------------------------------------------------------------
//|||      BELOW ARE METHODS FOR ASYNC FRIEND REQUESTS         |||
//----------------------------------------------------------------
//----------------------------------------------------------------

auto Transmitter::transmit_async_friend_request() -> void {
  // retrieve the friend request from config
  auto async_friend_request_ = config->get_outgoing_async_friend_request();
  if (!async_friend_request_.ok()) {
    std::cerr << "Error retrieving async friend request: "
              << async_friend_request_.status() << std::endl;
    return;
  }
  auto [friend_public_key, friend_info] = async_friend_request_.value();
  // encrypt the friend request
  auto encrypted_friend_request_status_ = crypto.encrypt_async_friend_request(
      config->registration_info(), friend_public_key);
  if (!encrypted_friend_request_status_.ok()) {
    std::cerr << "Error encrypting async friend request: "
              << encrypted_friend_request_status_.status() << std::endl;
    return;
  }
  auto encrypted_friend_request = encrypted_friend_request_status_.value();
  // Send to server
  asphrserver::AddFriendAsyncInfo request;
  request.set_index(config->registration_info().allocation.at(0));
  request.set_authentication_token(
      config->registration_info().authentication_token);
  request.set_request(encrypted_friend_request);
  asphrserver::AddFriendAsyncResponse reply;
  grpc::ClientContext context;
  grpc::Status status = stub->AddFriendAsync(&context, request, &reply);
  if (status.ok()) {
    std::cout << "Async Friend Request sent to server!" << std::endl;
  } else {
    std::cerr << status.error_code() << ": " << status.error_message()
              << " details:" << status.error_details() << std::endl;
  }
  return;
}

// Retrieves async friend requests from the server
// Returns a map of friend public key to friend info, aimed at the client
auto Transmitter::retrieve_async_friend_request(int start_index, int end_index)
    -> asphr::StatusOr<std::map<string, Friend>> {
  // check input
  if (start_index < 0 || end_index < 0 || start_index > end_index ||
      end_index - start_index > ASYNC_FRIEND_REQUEST_BATCH_SIZE) {
    return absl::InvalidArgumentError("Invalid indices");
  }

  asphrserver::GetAsyncFriendRequestsInfo request;
  request.set_start_index(start_index);
  request.set_end_index(end_index);
  asphrserver::GetAsyncFriendRequestsResponse reply;
  // transmit
  grpc::ClientContext context;
  grpc::Status status = stub->GetAsyncFriendRequests(&context, request, &reply);
  if (!status.ok()) {
    std::cerr << status.error_code() << ": " << status.error_message()
              << " details:" << status.error_details() << std::endl;
    return {
        absl::UnknownError("GetAsyncFriendRequests RPC failed with error: " +
                           status.error_message())};
  }

  assert(reply.friend_request_public_key_size() == reply.requests_size());
  // iterate over the returned friend requests
  std::map<string, Friend> friends = {};
  for (int i = 0; i < reply.requests_size(); i++) {
    // TODO: we need to get the friend's public_friend_key via a PKI
    // decrypt the friend request
    string friend_request = reply.requests(i);
    string friend_public_key = reply.friend_request_public_key(i);
    auto decrypted_friend_request_status = crypto.decrypt_async_friend_request(
        config->registration_info(), friend_public_key, friend_request);
    if (!decrypted_friend_request_status.ok()) {
      // not meant for us!
      continue;
    }
    // the friend request is meant for us
    // unpack the return value
    auto decrypted_friend_request = decrypted_friend_request_status.value();
    // create a friend object
    string name = std::get<0>(decrypted_friend_request);
    int allocation = std::get<1>(decrypted_friend_request);
    string friend_kx_public_key = std::get<2>(decrypted_friend_request);
    // derive read and write keys. TODO: do we do this here?
    auto [read_key, write_key] = crypto.derive_read_write_keys(
        config->registration_info().public_key,
        config->registration_info().private_key, friend_kx_public_key);

    // TODO: there's gotta be a better way than passing 11 arguments RIGHT?
    // Also, I'm not setting the ack index here.
    Friend friend_instance(
        name, allocation,
        crypto.generate_friend_key(friend_kx_public_key, allocation), read_key,
        write_key, 0, true, 0, 0, 0, false);

    // add the friend to the map
    friends.insert({friend_public_key, friend_instance});
  }
  return friends;
}
auto Transmitter::check_rep() const noexcept -> void {
  assert(config->has_registered() || !config->has_registered());
}