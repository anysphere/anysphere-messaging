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
      outbox(config->data_dir_address() / "outbox.json", msgstore) {
  check_rep();
}

auto Transmitter::retrieve_messages() -> void {
  check_rep();
  if (!config->has_registered()) {
    cout << "hasn't registered yet, so cannot retrieve messages" << endl;
    return;
  }

  auto& client = config->pir_client();
  asphrserver::ReceiveMessageInfo request;

  // choose a friend to receive from!!
  // priority 1: the friend that we just sent a message to
  // priority 2: the friend that we successfully received a message from the
  // previous round priority 3: random!
  Friend friend_info;
  bool dummy = false;
  if (auto friend_info_status = config->get_friend(just_sent_friend);
      friend_info_status.ok()) {
    friend_info = friend_info_status.value();
  } else if (auto friend_info_status =
                 config->get_friend(previous_success_receive_friend);
             friend_info_status.ok()) {
    friend_info = friend_info_status.value();
  } else if (auto friend_info_status = config->random_enabled_friend();
             friend_info_status.ok()) {
    // note: we do not need cryptographic randomness here. randomness is only
    // for liveness
    friend_info = friend_info_status.value();
  } else {
    friend_info = config->dummy_me();
    dummy = true;
  }

  auto query = client.query(friend_info.read_index, config->db_rows());

  auto serialized_query = query.serialize_to_string();

  request.set_pir_query(serialized_query);

  asphrserver::ReceiveMessageResponse reply;
  grpc::ClientContext context;

  grpc::Status status = stub->ReceiveMessage(&context, request, &reply);

  previous_success_receive_friend = "";

  check_rep();

  // if dummy, we do not actually care about the answer.
  // we still do the rpc to not leak information.
  if (dummy) {
    return;
  }

  if (status.ok()) {
    cout << "received message!!!" << endl;

    // TODO: inbox.receive_message and msgstore->add_incoming_message need to be
    // atomic!!!
    // TODO: msgstore should mark a message as delivered here possibly,
    // depending on the ACKs. EDIT: this is currently done in outbox which seems
    // fine
    auto message_opt =
        inbox.receive_message(client, *config, reply, friend_info, crypto,
                              previous_success_receive_friend);
    if (message_opt.has_value()) {
      auto message = message_opt.value();
      msgstore->add_incoming_message(message.id, message.friend_name,
                                     message.message);
    } else {
      cout << "no message received" << endl;
    }
  } else {
    cout << status.error_code() << ": " << status.error_message() << endl;
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

auto Transmitter::check_rep() const noexcept -> void {
  assert(config->has_registered() || !config->has_registered());
}