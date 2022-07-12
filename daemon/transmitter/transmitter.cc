//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "transmitter.hpp"

#include <algorithm>

#include "daemon/identifier/identifier.hpp"
#include "schema/server.grpc.pb.h"

auto generate_dummy_address(const db::Registration& reg) -> db::Address {
  auto dummy_friend_keypair = crypto::generate_kx_keypair();

  // convert reg.kx_public_key, kx_private_key to a string

  auto kx_public_key_str = rust_u8Vec_to_string(reg.kx_public_key);
  auto kx_private_key_str = rust_u8Vec_to_string(reg.kx_private_key);

  auto dummy_read_write_keys = crypto::derive_read_write_keys(
      kx_public_key_str, kx_private_key_str, dummy_friend_keypair.first);

  // convert dummy_read_write_keys to a rust::Vec<uint8_t>
  auto read_key_vec_rust = string_to_rust_u8Vec(dummy_read_write_keys.first);
  auto write_key_vec_rust = string_to_rust_u8Vec(dummy_read_write_keys.second);

  return (db::Address){-1, 0, read_key_vec_rust, write_key_vec_rust, 0};
}

auto generate_dummy_async_invitation() -> db::OutgoingAsyncInvitation {
  auto dummy_kx_keypair = crypto::generate_kx_keypair();
  auto kx_public_key = dummy_kx_keypair.first;
  auto kx_public_key_rust = string_to_rust_u8Vec(kx_public_key);

  auto dummy_invitation_keypair = crypto::generate_invitation_keypair();
  auto invitation_public_key = dummy_invitation_keypair.first;
  auto invitation_public_key_rust = string_to_rust_u8Vec(invitation_public_key);

  auto public_id = PublicIdentifier(0, kx_public_key, invitation_public_key);
  auto public_id_str = public_id.to_public_id();

  return db::OutgoingAsyncInvitation{
      .friend_uid = -1,
      .unique_name = "dummy",
      .display_name = "Dummy",
      .invitation_progress = db::InvitationProgress::OutgoingAsync,
      .public_id = public_id_str,
      .invitation_public_key = invitation_public_key_rust,
      .kx_public_key = kx_public_key_rust,
      .message = "Hello dummy",
      .sent_at = 0,
  };
}

Transmitter::Transmitter(Global& G, shared_ptr<asphrserver::Server::Stub> stub)
    : G(G), stub(stub), next_async_invitation_retrieve_index(0) {
  check_rep();
}

auto Transmitter::setup_registration_caching() -> void {
  check_rep();

  // MAY throw if we are not registered.
  auto pir_secret_key_str = rust_u8Vec_to_string(G.db->get_pir_secret_key());

  if (!cached_pir_client.has_value() ||
      cached_pir_client_secret_key.value() != pir_secret_key_str) {
    auto reg = G.db->get_registration();
    cached_pir_client = std::optional(std::make_unique<FastPIRClient>(
        rust_u8Vec_to_string(reg.pir_secret_key),
        rust_u8Vec_to_string(reg.pir_galois_key)));

    cached_pir_client_secret_key = rust_u8Vec_to_string(reg.pir_secret_key);
    dummy_address = generate_dummy_address(reg);
    dummy_outgoing_invitation = generate_dummy_async_invitation();
  }

  check_rep();
}

auto decrypt_ack_row(pir_value_t& acks_row, const string& read_key)
    -> asphr::StatusOr<int> {
  vector<string> encrypted_acks(MAX_FRIENDS);
  for (size_t i = 0; i < MAX_FRIENDS; i++) {
    encrypted_acks.at(i).resize(ENCRYPTED_ACKING_BYTES);
    std::copy(acks_row.begin() + i * ENCRYPTED_ACKING_BYTES,
              acks_row.begin() + (i + 1) * ENCRYPTED_ACKING_BYTES,
              encrypted_acks.at(i).begin());
  }

  // try decrypting each!
  for (size_t i = 0; i < MAX_FRIENDS; i++) {
    auto ack = crypto::decrypt_ack(encrypted_acks.at(i), read_key);
    if (ack.ok()) {
      return ack.value();
    }
  }
  return absl::NotFoundError("Could not decrypt any ack.");
}

auto Transmitter::encrypt_ack_row(const vector<db::OutgoingAck>& acks,
                                  const string& dummy_write_key)
    -> asphr::StatusOr<pir_value_t> {
  assert(acks.size() <= MAX_FRIENDS);
  check_rep();

  vector<string> encrypted_acks(MAX_FRIENDS);
  for (auto& wrapped_ack : acks) {
    auto ack = crypto::encrypt_ack(wrapped_ack.ack,
                                   rust_u8Vec_to_string(wrapped_ack.write_key));
    if (!ack.ok()) {
      ASPHR_LOG_ERR("Could not encrypt ack.", error_code, ack.status().code(),
                    error_message, ack.status().message());
      return absl::UnknownError("encryption failed");
    }
    encrypted_acks.at(wrapped_ack.ack_index) = ack.value();
  }
  for (size_t i = 0; i < MAX_FRIENDS; i++) {
    if (encrypted_acks.at(i).empty()) {
      auto ack = crypto::encrypt_ack(0, dummy_write_key);
      if (!ack.ok()) {
        ASPHR_LOG_ERR("Could not encrypt dummy ack.", error_code,
                      ack.status().code(), error_message,
                      ack.status().message());
        return absl::UnknownError("encryption failed");
      }
      encrypted_acks.at(i) = ack.value();
    }
  }
  pir_value_t pir_acks;
  for (size_t i = 0; i < MAX_FRIENDS; i++) {
    std::copy(encrypted_acks.at(i).begin(), encrypted_acks.at(i).end(),
              pir_acks.begin() + i * ENCRYPTED_ACKING_BYTES);
  }

  check_rep();
  return pir_acks;
}

auto Transmitter::retrieve() -> void {
  check_rep();

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Not registered, so not retrieving messages.");
    return;
  }

  setup_registration_caching();
  auto& client = **cached_pir_client;

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
       i < std::min(RECEIVE_FRIENDS_PER_ROUND,
                    static_cast<int>(priority_receive_friends.size()));
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

    // get a vector of recieve uids
    rust::Vec<int> receive_uids;
    for (auto& r : receive_addresses) {
      receive_uids.push_back(r.uid);
    }

    try {
      auto f = G.db->get_random_enabled_friend_address_excluding(receive_uids);

      receive_addresses.push_back(f);
      receive_addresses_is_dummy.push_back(false);
    } catch (const rust::Error& e) {
      // dummy if no friends left :')
      receive_addresses.push_back(dummy_address.value());
      receive_addresses_is_dummy.push_back(true);
    }
  }
  assert(receive_addresses.size() == RECEIVE_FRIENDS_PER_ROUND);

  auto friends_to_receive_from = string("");
  for (auto& r : receive_addresses) {
    friends_to_receive_from += std::to_string(r.uid) + ",";
  }
  ASPHR_LOG_INFO("Receiving messages from the following friends.", friend_uids,
                 friends_to_receive_from);

  // -----
  // Step 2: execute the PIR queries
  // -----

  // create a vector of read_indices
  vector<uint32_t> read_indices;
  for (auto& r : receive_addresses) {
    read_indices.push_back(r.read_index);
  }

  vector<absl::StatusOr<asphrserver::ReceiveMessageResponse>> pir_replies =
      batch_retrieve_pir(client, read_indices);

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
      auto decrypted_ack_status =
          decrypt_ack_row(ack_decoded, rust_u8Vec_to_string(f.read_key));
      if (decrypted_ack_status.ok()) {
        auto decrypted_ack = decrypted_ack_status.value();

        auto new_ack = G.db->receive_ack(f.uid, decrypted_ack);
        if (new_ack) {
          ASPHR_LOG_INFO("Received new ACK from friend.", friend_uid, f.uid);
          just_acked_friend = f.uid;
        }
      } else {
        ASPHR_LOG_INFO("Could not decrypt ACK from friend.", friend_uid, f.uid);
      }

      //
      // Step 3(b): Record the message in the database.
      //
      auto answer = reply.pir_answer();
      auto answer_obj = client.answer_from_string(answer);
      auto decoded = client.decode(answer_obj, f.read_index);

      auto decrypted =
          crypto::decrypt_receive(decoded, rust_u8Vec_to_string(f.read_key));
      if (decrypted.ok()) {
        auto& chunk = decrypted.value();

        if (chunk.sequence_number() == 0) {
          ASPHR_LOG_INFO(
              "Received empty garbage-message for security purposes.",
              friend_uid, f.uid);
          return;
        } else {
          ASPHR_LOG_INFO("Received real chunk from friend.", friend_uid, f.uid,
                         sequence_number, chunk.sequence_number(),
                         chunks_start_sequence_number,
                         chunk.chunks_start_sequence_number(), num_chunks,
                         chunk.num_chunks(), chunk_content, chunk.msg(),
                         is_system, chunk.system(), system_message,
                         chunk.system_message(), system_message_data,
                         chunk.system_message_data());
        }

        if (chunk.system()) {
          switch (chunk.system_message()) {
            case asphrclient::SystemMessage::OUTGOING_INVITATION: {
              ASPHR_LOG_INFO(
                  "Received outgoing invitation from someone who's already "
                  "someone we wanted to add. This means we want to make them a "
                  "complete friend!!",
                  friend_uid, f.uid);
              // we need to get the public_id here, and verify that it is
              // correct with what we have on file!
              auto public_id_status =
                  PublicIdentifier::from_public_id(chunk.system_message_data());
              if (!public_id_status.ok()) {
                ASPHR_LOG_ERR(
                    "Received outgoing invitation from someone who's already "
                    "someone we wanted to add, but the public id was invalid.",
                    friend_uid, f.uid);
              } else {
                auto public_id = public_id_status.value();
                try {
                  G.db->receive_invitation_system_message(
                      f.uid, chunk.sequence_number(),
                      chunk.system_message_data(),
                      string_to_rust_u8Vec(public_id.kx_public_key),
                      string_to_rust_u8Vec(public_id.invitation_public_key));
                } catch (const rust::Error& e) {
                  ASPHR_LOG_ERR("Unable to receive invitation system message",
                                error_message, e.what(), friend_uid, f.uid);
                }
              }
              break;
            }
            default: {
              ASPHR_LOG_ERR("Received unknown system message from friend.",
                            friend_uid, f.uid, system_message,
                            chunk.system_message(), system_message_data,
                            chunk.system_message_data());
              break;
            }
          }
        } else {
          // we don't set these fields if we only have one chunk
          auto num_chunks = chunk.num_chunks() > 1 ? chunk.num_chunks() : 1;
          auto chunks_start_sequence_number =
              chunk.num_chunks() > 1 ? chunk.chunks_start_sequence_number()
                                     : chunk.sequence_number();

          auto chunk_content = rust::Vec<uint8_t>();
          for (auto& c : chunk.msg()) {
            chunk_content.push_back(c);
          }

          // TODO: we probably don't want to cast to int32 here... let's use
          // int64s everywhere
          auto receive_chunk_status = G.db->receive_chunk(
              (db::IncomingChunkFragment){
                  .from_friend = f.uid,
                  .sequence_number = static_cast<int>(chunk.sequence_number()),
                  .chunks_start_sequence_number =
                      static_cast<int>(chunks_start_sequence_number),
                  .content = chunk_content,
              },
              static_cast<int>(num_chunks));
          if (receive_chunk_status ==
              db::ReceiveChunkStatus::NewChunkAndNewMessage) {
            std::lock_guard<std::mutex> l(G.message_notification_cv_mutex);
            G.message_notification_cv.notify_all();
          }

          if (receive_chunk_status == db::ReceiveChunkStatus::NewChunk ||
              receive_chunk_status ==
                  db::ReceiveChunkStatus::NewChunkAndNewMessage) {
            previous_success_receive_friend = std::optional<int>(f.uid);
          }
        }
      } else {
        ASPHR_LOG_INFO(
            "Failed to decrypt message (message was probably not for us, "
            "which "
            "is okay).",
            decrypted_error_code, decrypted.status().code(),
            decrypted_error_message, decrypted.status().message());
      }
    } else {
      ASPHR_LOG_ERR("Could not retrieve PIR reply from server.", friend_uid,
                    f.uid, server_status_code,
                    pir_replies.at(i).status().code(), server_status_message,
                    pir_replies.at(i).status().message());
    }
  }
  // Crawl the async friend request database
  // TODO: we could accelerate / deccelerate this as the client desires/
  // by changing ASYNC_FRIEND_REQUEST_BATCH_SIZE
  auto [start_index, end_index] = update_async_invitation_retrieve_index();

  // call the server to retrieve the async friend requests
  // this will also reset the index to 0 if end_index is at the end of the DB
  // ROWS
  retrieve_async_invitations(start_index, end_index);

  check_rep();
}

auto Transmitter::send() -> void {
  check_rep();

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Not registered, so not sending messages.");
    return;
  }

  setup_registration_caching();

  auto send_info = G.db->get_send_info();

  rust::Vec<int> prioritized_friends;
  if (just_acked_friend.has_value()) {
    prioritized_friends.push_back(just_acked_friend.value());
  }
  string write_key;
  asphrclient::Chunk chunk;
  try {
    auto chunk_to_send = G.db->chunk_to_send(prioritized_friends);
    just_sent_friend = chunk_to_send.to_friend;
    write_key = rust_u8Vec_to_string(chunk_to_send.write_key);

    chunk.set_sequence_number(chunk_to_send.sequence_number);

    if (chunk_to_send.system) {
      chunk.set_system(true);
      switch (chunk_to_send.system_message) {
        case db::SystemMessage::OutgoingInvitation:
          chunk.set_system_message(asphrclient::OUTGOING_INVITATION);
          break;
        default:
          ASPHR_ASSERT(false);
      }
      chunk.set_system_message_data(std::string(chunk_to_send.content.begin(),
                                                chunk_to_send.content.end()));
    } else {
      chunk.set_msg(std::string(chunk_to_send.content.begin(),
                                chunk_to_send.content.end()));
      if (chunk_to_send.num_chunks > 1) {
        chunk.set_num_chunks(chunk_to_send.num_chunks);
        chunk.set_chunks_start_sequence_number(
            chunk_to_send.chunks_start_sequence_number);
      }
    }

  } catch (const rust::Error& e) {
    ASPHR_LOG_INFO("No chunks to send (probably).", error_msg, e.what());
    just_sent_friend = std::nullopt;
    write_key = rust_u8Vec_to_string(dummy_address.value().write_key);
    chunk.set_sequence_number(0);
    chunk.set_msg("fake chunk");
  }

  auto encrypted_chunk_status = crypto::encrypt_send(chunk, write_key);
  if (!encrypted_chunk_status.ok()) {
    ASPHR_LOG_ERR("Could not encrypt message.", error_msg,
                  encrypted_chunk_status.status().message());
    return;
  }
  auto encrypted_chunk = encrypted_chunk_status.value();

  auto acks_to_send_rustvec = G.db->acks_to_send();
  auto acks_to_send =
      std::vector(acks_to_send_rustvec.begin(), acks_to_send_rustvec.end());
  auto encrypted_acks_status = encrypt_ack_row(
      acks_to_send, rust_u8Vec_to_string(dummy_address.value().write_key));
  if (!encrypted_acks_status.ok()) {
    ASPHR_LOG_ERR("Could not encrypt ACKs.", error_msg,
                  encrypted_acks_status.status().message());
    return;
  }
  auto encrypted_acks = encrypted_acks_status.value();

  ASPHR_LOG_INFO("Sending chunk.", friend_uid, just_sent_friend.value_or(-1),
                 index, send_info.allocation, auth_token,
                 std::string(send_info.authentication_token));

  check_rep();

  // call register rpc to send the register request
  asphrserver::SendMessageInfo request;

  request.set_index(send_info.allocation);
  request.set_authentication_token(std::string(send_info.authentication_token));

  string encrypted_chunk_string = "";
  for (auto& c : encrypted_chunk) {
    encrypted_chunk_string += c;
  }
  request.set_message(encrypted_chunk_string);

  string encrypted_acks_string = "";
  for (auto& c : encrypted_acks) {
    encrypted_acks_string += c;
  }
  request.set_acks(encrypted_acks_string);

  asphrserver::SendMessageResponse reply;

  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() +
                       std::chrono::seconds(60));

  grpc::Status status = stub->SendMessage(&context, request, &reply);

  if (status.ok()) {
    ASPHR_LOG_INFO("Sent chunk.", friend_uid, just_sent_friend.value_or(-1),
                   index, send_info.allocation, auth_token,
                   std::string(send_info.authentication_token));
  } else {
    ASPHR_LOG_ERR("Could not send chunk.", friend_uid,
                  just_sent_friend.value_or(-1), index, send_info.allocation,
                  auth_token, std::string(send_info.authentication_token),
                  server_status_code, status.error_code(),
                  server_status_message, status.error_message());
  }
  transmit_async_invitation();

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
    context.set_deadline(std::chrono::system_clock::now() +
                         std::chrono::seconds(60));

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

auto Transmitter::transmit_async_invitation() -> void {
  ASPHR_ASSERT(dummy_outgoing_invitation.has_value());

  // retrieve the friend request from DB
  std::vector<db::OutgoingAsyncInvitation> invitations = {};
  try {
    auto rust_invitations = G.db->get_outgoing_async_invitations();
    for (auto& rust_invitation : rust_invitations) {
      invitations.push_back(rust_invitation);
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get outgoing async invitations.", error_msg,
                  e.what());
    // TODO: alert the user saying that their invitation may be delayed
  }
  // TODO: allow us to send multiple friend requests at once
  // Right now, we only send one friend request at once
  // We can change this later if we want to.

  if (invitations.size() == 0) {
    invitations.push_back(dummy_outgoing_invitation.value());
  }

  ASPHR_ASSERT_EQ_MSG(
      std::ssize(invitations), 1,
      "We only support one outgoing invitation at once at the moment.");

  // send the friend request
  db::OutgoingAsyncInvitation invitation = invitations.at(0);

  // we need to compute the id for both parties
  // We declare these outside cause I don't want to everything wrapped in
  // try-catch
  string my_id;  // we could probably cache this in DB, but we don't need to
  string my_invitation_private_key;
  string friend_invitation_public_key;
  db::SmallRegistrationFragment reg_info;
  try {
    reg_info = G.db->get_small_registration();
    my_id = std::string(reg_info.public_id);
    my_invitation_private_key =
        rust_u8Vec_to_string(reg_info.invitation_private_key);
    friend_invitation_public_key =
        rust_u8Vec_to_string(invitation.invitation_public_key);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Could not get registration.", error_msg, e.what());
    return;
  }

  // encrypt the friend request
  auto encrypted_invitation_status_ = crypto::encrypt_async_invitation(
      my_id, my_invitation_private_key, friend_invitation_public_key,
      std::string(invitation.message));

  if (!encrypted_invitation_status_.ok()) {
    ASPHR_LOG_ERR("Error encrypting async friend request: ", error_msg,
                  encrypted_invitation_status_.status().ToString());
    return;
  }

  auto encrypted_invitation = encrypted_invitation_status_.value();

  // Send to server
  asphrserver::AddAsyncInvitationInfo request;
  request.set_index(reg_info.allocation);
  request.set_authentication_token(std::string(reg_info.authentication_token));
  request.set_invitation(encrypted_invitation);
  asphrserver::AddAsyncInvitationResponse reply;
  grpc::ClientContext context;
  grpc::Status status = stub->AddAsyncInvitation(&context, request, &reply);
  if (status.ok()) {
    ASPHR_LOG_INFO("Async invitation sent to server.");
  } else {
    ASPHR_LOG_ERR(
        "Communication with server failed when sending friend request",
        error_code, status.error_code(), details, status.error_details());
  }
}

// retrieve and process async friend request from the server
// and push them to the database
auto Transmitter::retrieve_async_invitations(int start_index, int end_index)
    -> void {
  // check input
  ASPHR_LOG_INFO("Retrieving async invitations.", start_index, start_index,
                 end_index, end_index);
  ASPHR_ASSERT_MSG(start_index >= 0, "start_index must be >= 0");
  ASPHR_ASSERT_MSG(end_index >= 0, "end_index must be >= 0");
  ASPHR_ASSERT_MSG(start_index <= end_index,
                   "start_index must be <= end_index");
  ASPHR_ASSERT_MSG(
      end_index - start_index <= ASYNC_FRIEND_REQUEST_BATCH_SIZE,
      "end_index - start_index must be <= ASYNC_FRIEND_REQUEST_BATCH_SIZE");

  // STEP 1: ask the server for all the friend database entries
  asphrserver::GetAsyncInvitationsInfo request;
  request.set_start_index(start_index);
  request.set_end_index(end_index);
  asphrserver::GetAsyncInvitationsResponse reply;

  grpc::ClientContext context;
  grpc::Status status = stub->GetAsyncInvitations(&context, request, &reply);
  if (!status.ok()) {
    ASPHR_LOG_ERR("Could not retrieve async friend requests.", error_code,
                  status.error_code(), error_message, status.error_message(),
                  error_details, status.error_details());
    return;
  }

  if (reply.invitation_public_key_size() != reply.invitations_size()) {
    ASPHR_LOG_ERR("Response is malformed!",
                  "size1:", reply.invitation_public_key_size(),
                  "size2:", reply.invitations_size());
    return;
  }

  // Step 2: iterate over the returned friend requests
  // we need to obtain our own id here
  string my_id;
  string my_invitation_private_key;
  db::SmallRegistrationFragment reg_info;
  try {
    reg_info = G.db->get_small_registration();
    my_id = std::string(reg_info.public_id);
    my_invitation_private_key =
        rust_u8Vec_to_string(reg_info.invitation_private_key);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to retrieve registration from db.", error_msg,
                  e.what());
    return;
  }

  ASPHR_LOG_INFO("Retrieved async friend requests from server.",
                 "invitations_size", reply.invitations_size());

  for (int i = 0; i < reply.invitations_size(); i++) {
    // Step 2.1: test if the friend request is meant for us
    // For now, we attach the friend_public_key along with every request.
    // decrypt the friend request
    string invitation = reply.invitations(i);
    string friend_public_key = reply.invitation_public_key(i);
    auto decrypted_invitation_status = crypto::decrypt_async_invitation(
        my_invitation_private_key, friend_public_key, invitation);
    if (!decrypted_invitation_status.ok()) {
      // not meant for us! this is ok. the expected outcome.
      continue;
    }
    // the friend request is meant for us
    // Step 2.2: unpack the friend request
    auto [friend_public_id_str, friend_message] =
        decrypted_invitation_status.value();
    ASPHR_LOG_INFO("Found async friend request!", public_id,
                   friend_public_id_str, message, friend_message);
    // unpack the friend id
    auto friend_public_id_status =
        PublicIdentifier::from_public_id(friend_public_id_str);
    if (!friend_public_id_status.ok()) {
      ASPHR_LOG_ERR("Read malformed friend public ID.", error_message,
                    friend_public_id_status.status().message());
      continue;
    }
    auto friend_public_id = friend_public_id_status.value();
    // verify that the friend_request_public_key is the same as the one we
    // authenticated the thing with
    // TODO: write the security checks needed here clearly in the whitepaper.
    if (friend_public_id.invitation_public_key != friend_public_key) {
      // TODO: maybe we can warn the user that an imposter is about.
      ASPHR_LOG_ERR("Friend public ID does not match friend public key.",
                    error_message, friend_public_id_status.status().message());
      continue;
    }

    try {
      G.db->add_incoming_async_invitation(friend_public_id_str, friend_message);
    } catch (const rust::Error& e) {
      ASPHR_LOG_ERR("Error when adding async invitation.", error_message,
                    e.what());
      continue;
    }
  }

  if (end_index == CLIENT_DB_ROWS) {
    next_async_invitation_retrieve_index = 0;
  } else {
    next_async_invitation_retrieve_index = end_index;
  }
}

auto Transmitter::check_rep() const noexcept -> void {
  ASPHR_ASSERT(G.alive());

  ASPHR_ASSERT_EQ_MSG(
      cached_pir_client.has_value(), cached_pir_client_secret_key.has_value(),
      "cached_pir_client and cached_pir_client_status are not in sync");
  ASPHR_ASSERT_EQ_MSG(cached_pir_client.has_value(), dummy_address.has_value(),
                      "cached_pir_client and dummy_address are not in sync");
  ASPHR_ASSERT_EQ_MSG(
      cached_pir_client.has_value(), dummy_outgoing_invitation.has_value(),
      "cached_pir_client and dummy_outgoing_invitation are not in sync");
}