//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "daemon_rpc.hpp"

#include "google/protobuf/util/time_util.h"

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

Status DaemonRpc::RegisterUser(
    ServerContext* context,
    const asphrdaemon::RegisterUserRequest* registerUserRequest,
    asphrdaemon::RegisterUserResponse* registerUserResponse) {
  ASPHR_LOG_INFO("RegisterUser() called.", rpc_call, "RegisterUser");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Already registered.", rpc_call, "RegisterUser");
    return Status(grpc::StatusCode::ALREADY_EXISTS, "already registered");
  }

  const auto name = registerUserRequest->name();
  const auto [public_key, secret_key] = crypto::generate_keypair();
  const auto [pir_secret_key, pir_galois_keys] = generate_keys();

  auto beta_key = registerUserRequest->beta_key();

  // call register rpc to send the register request
  asphrserver::RegisterInfo request;
  request.set_public_key(public_key);
  request.set_beta_key(beta_key);

  asphrserver::RegisterResponse reply;
  grpc::ClientContext client_context;

  Status status = stub->Register(&client_context, request, &reply);

  if (status.ok()) {
    ASPHR_LOG_INFO("Register success.", rpc_call, "RegisterUser");

    const auto authentication_token = reply.authentication_token();
    auto alloc_repeated = reply.allocation();
    const auto allocation =
        vector<int>(alloc_repeated.begin(), alloc_repeated.end());

    if (reply.authentication_token() == "") {
      ASPHR_LOG_ERR("Register failed: authentication token is empty.", rpc_call,
                    "RegisterUser");
      return Status(grpc::StatusCode::UNKNOWN, "authentication token is empty");
    }
    if (reply.allocation().empty()) {
      ASPHR_LOG_ERR("Register failed: allocation is empty.", rpc_call,
                    "RegisterUser");
      return Status(grpc::StatusCode::UNKNOWN, "allocation is empty");
    }
    if (reply.allocation().size() != 1) {
      ASPHR_LOG_ERR(
          "Register failed: allocation is not 1. We currently only support 1 "
          "allocation.",
          rpc_call, "RegisterUser");
      return Status(grpc::StatusCode::UNKNOWN, "allocation is empty");
    }

    try {
      G.db->do_register(db::RegistrationFragment{
          .public_key = string_to_rust_u8Vec(public_key),
          .private_key = string_to_rust_u8Vec(secret_key),
          .allocation = allocation.at(0),
          .pir_secret_key = string_to_rust_u8Vec(pir_secret_key),
          .pir_galois_key = string_to_rust_u8Vec(pir_galois_keys),
          .authentication_token = authentication_token,
      });
    } catch (const rust::Error& e) {
      ASPHR_LOG_ERR("Register failed in database.", error, e.what(), rpc_call,
                    "RegisterUser");
      return Status(grpc::StatusCode::UNKNOWN, e.what());
    }

  } else {
    ASPHR_LOG_ERR("Register failed.", error_code, status.error_code(),
                  error_message, status.error_message(), rpc_call,
                  "RegisterUser");
    return Status(grpc::StatusCode::UNAVAILABLE, status.error_message());
  }

  return Status::OK;
}

Status DaemonRpc::GetFriendList(
    ServerContext* context,
    const asphrdaemon::GetFriendListRequest* getFriendListRequest,
    asphrdaemon::GetFriendListResponse* getFriendListResponse) {
  ASPHR_LOG_INFO("GetFriendList() called.");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "GetFriendList");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    for (auto& s : G.db->get_friends()) {
      auto new_friend = getFriendListResponse->add_friend_infos();
      new_friend->set_unique_name(s.unique_name);
      new_friend->set_display_name(s.display_name);
      new_friend->set_enabled(s.enabled);
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Database failed.", error, e.what(), rpc_call,
                  "GetFriendList");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::GenerateFriendKey(
    ServerContext* context,
    const asphrdaemon::GenerateFriendKeyRequest* generateFriendKeyRequest,
    asphrdaemon::GenerateFriendKeyResponse* generateFriendKeyResponse) {
  ASPHR_LOG_INFO("GenerateFriendKey() called.", rpc_call, "GenerateFriendKey");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "GenerateFriendKey");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  // note: for now, we only support the first index ever!
  auto reg = G.db->get_small_registration();
  auto index = reg.allocation;
  auto friend_key = crypto::generate_friend_key(reg.public_key, index);

  try {
    const auto f = G.db->create_friend(generateFriendKeyRequest->unique_name(),
                                       generateFriendKeyRequest->display_name(),
                                       MAX_FRIENDS);
    generateFriendKeyResponse->set_key(friend_key);
    return Status::OK;
  } catch (const rust::Error& e) {
    // If the friend doesn't exist, great!
    ASPHR_LOG_INFO("Friend already exists, or you have too many friends.",
                   error, e.what(), rpc_call, "GenerateFriendKey");
    return Status(grpc::StatusCode::ALREADY_EXISTS, "friend already exists");
  }

  return Status::OK;
}

Status DaemonRpc::AddFriend(
    ServerContext* context,
    const asphrdaemon::AddFriendRequest* addFriendRequest,
    asphrdaemon::AddFriendResponse* addFriendResponse) {
  ASPHR_LOG_INFO("AddFriend() called.", rpc_call, "AddFriend",
                 friend_unique_name, addFriendRequest->unique_name());

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "AddFriend");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto decoded_friend_key = crypto::decode_friend_key(addFriendRequest->key());
  if (!decoded_friend_key.ok()) {
    ASPHR_LOG_ERR("Invalid friend key.", rpc_call, "AddFriend");
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid friend key");
  }
  auto [read_index, friend_public_key] = decoded_friend_key.value();

  auto reg = G.db->get_small_registration();
  auto [read_key, write_key] = crypto::derive_read_write_keys(
      reg.public_key, reg.private_key, friend_public_key);

  try {
    G.db->add_friend_address((db::AddressFragment){
        .unique_name = addFriendRequest->unique_name(),
        .read_index = read_index,
        .read_key = read_key,
        .write_key = write_key,
    });
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to add friend.", error, e.what(), rpc_call,
                  "AddFriend");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::RemoveFriend(
    ServerContext* context,
    const asphrdaemon::RemoveFriendRequest* removeFriendRequest,
    asphrdaemon::RemoveFriendResponse* removeFriendResponse) {
  ASPHR_LOG_INFO("RemoveFriend() called.", rpc_call, "RemoveFriend",
                 friend_unique_name, removeFriendRequest->unique_name());

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "RemoveFriend");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    G.db->delete_friend(removeFriendRequest->unique_name());
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to remove friend.", error, e.what(), rpc_call,
                  "RemoveFriend");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::SendMessage(
    ServerContext* context,
    const asphrdaemon::SendMessageRequest* sendMessageRequest,
    asphrdaemon::SendMessageResponse* sendMessageResponse) {
  ASPHR_LOG_INFO("SendMessage() called.", rpc_call, "SendMessage",
                 friend_unique_name, sendMessageRequest->unique_name(),
                 message_length, sendMessageRequest->message().size(), message,
                 sendMessageRequest->message());

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "SendMessage");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto friend_info_status = config->get_friend(sendMessageRequest->name());

  try {
    G.db->send_message(sendMessageRequest->unique_name(),
                       sendMessageRequest->message());
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to send message.", error, e.what(), rpc_call,
                  "SendMessage");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::GetMessages(
    ServerContext* context,
    const asphrdaemon::GetMessagesRequest* getMessagesRequest,
    asphrdaemon::GetMessagesResponse* getMessagesResponse) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  ASPHR_LOG_INFO("GetMessages() called.", rpc_call, "GetMessages");

  if (!G.db->has_registered()) {
    ASPHR_LOG_ERR("Need to register first.", rpc_call, "GetMessages");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto filter = getMessagesRequest->filter();

  if (filter != asphrdaemon::GetMessagesRequest::ALL &&
      filter != asphrdaemon::GetMessagesRequest::NEW) {
    ASPHR_LOG_ERR("Invalid filter.", rpc_call, "GetMessages", filter, filter);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid filter");
  }

  try {
    auto messages = G.db->get_received_messages(db::MessageQuery{
        .limit = 100,  // TODO: make this configurable
        .filter = filter == asphrdaemon::GetMessagesRequest::ALL
                      ? db::MessageFilter::All
                      : db::MessageFilter::New,
    });

    for (auto& m : messages) {
      auto message_info = getMessagesResponse->add_messages();

      auto baseMessage = message_info->mutable_m();
      baseMessage->set_id(m.uid);
      baseMessage->set_message(m.content);
      baseMessage->set_unique_name(m.from_unique_name);
      baseMessage->set_display_name(m.from_display_name);
      message_info->set_seen(m.seen);
      message_info->set_delivered(m.delivered);
      auto timestamp = message_info->mutable_received_timestamp();
      auto success = TimeUtil::FromString(timestamp_str, timestamp);
      if (!success) {
        ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                      rpc_call, "GetMessages");
        return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
      }
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get messages.", error, e.what(), rpc_call,
                  "GetMessages");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

// WARNING: this method is subtle. please take a moment to understand
// what's going on before modifying it.
Status DaemonRpc::GetMessagesStreamed(
    ServerContext* context,
    const asphrdaemon::GetMessagesRequest* getMessagesRequest,
    ServerWriter<asphrdaemon::GetMessagesResponse>* writer) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  cout << "GetMessagesStreamed() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto filter = getMessagesRequest->filter();

  // the messages may not be added in the right order. so what we do is
  // we wait until the last_mono_index has changed, and then get all messages
  // from that point on.
  // this needs to happen before we get all the messages below.
  //
  // the caller will always get *each message EXACTLY ONCE*, in the order of
  // the mono index. *this may or may not correspond to the timestamp order.*
  int last_mono_index;
  {
    std::lock_guard<std::mutex> l(msgstore->add_cv_mtx);
    last_mono_index = msgstore->last_mono_index;
  }

  {
    vector<IncomingMessage> messages;

    if (filter == asphrdaemon::GetMessagesRequest::ALL) {
      messages = msgstore->get_all_incoming_messages_sorted();
    } else if (filter == asphrdaemon::GetMessagesRequest::NEW) {
      messages = msgstore->get_new_incoming_messages_sorted();
    } else {
      cout << "filter: INVALID" << endl;
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid filter");
    }

    asphrdaemon::GetMessagesResponse response;

    for (auto& m : messages) {
      // let last_mono_index be the maximum
      last_mono_index =
          m.mono_index > last_mono_index ? m.mono_index : last_mono_index;

      auto message_info = response.add_messages();

      auto baseMessage = message_info->mutable_m();
      baseMessage->set_id(m.id);
      baseMessage->set_message(m.message);
      message_info->set_from(m.from);
      message_info->set_seen(m.seen);

      // TODO(arvid): do this conversion not through strings....
      auto timestamp_str = absl::FormatTime(m.received_timestamp);
      auto timestamp = message_info->mutable_received_timestamp();
      auto success = TimeUtil::FromString(timestamp_str, timestamp);
      if (!success) {
        cout << "invalid timestamp" << endl;
        return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
      }
    }

    writer->Write(response);
  }

  // keep the connection open forever
  while (!context->IsCancelled()) {
    int last_mono_index_here;
    {
      std::unique_lock<std::mutex> l(msgstore->add_cv_mtx);
      msgstore->add_cv.wait_for(l, std::chrono::seconds(60 * 60), [&] {
        return msgstore->last_mono_index != last_mono_index;
      });
      if (msgstore->last_mono_index == last_mono_index) {
        continue;  // continue the loop to check if we are cancelled
      }
      last_mono_index_here = msgstore->last_mono_index;
    }

    auto messages =
        msgstore->get_incoming_messages_sorted_after(last_mono_index);

    asphrdaemon::GetMessagesResponse response;

    for (auto& m : messages) {
      // let last_mono_index_here be the maximum
      last_mono_index_here = m.mono_index > last_mono_index_here
                                 ? m.mono_index
                                 : last_mono_index_here;

      auto message_info = response.add_messages();

      auto baseMessage = message_info->mutable_m();
      baseMessage->set_id(m.id);
      baseMessage->set_message(m.message);
      message_info->set_from(m.from);
      message_info->set_seen(m.seen);

      // TODO(arvid): do this conversion not through strings....
      auto timestamp_str = absl::FormatTime(m.received_timestamp);
      auto timestamp = message_info->mutable_received_timestamp();
      auto success = TimeUtil::FromString(timestamp_str, timestamp);
      if (!success) {
        cout << "invalid timestamp" << endl;
        return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
      }
    }

    writer->Write(response);

    last_mono_index = last_mono_index_here;
  }

  return Status::OK;
}

Status DaemonRpc::GetOutboxMessages(
    ServerContext* context,
    const asphrdaemon::GetOutboxMessagesRequest* getOutboxMessagesRequest,
    asphrdaemon::GetOutboxMessagesResponse* getOutboxMessagesResponse) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  cout << "GetOutboxMessages() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto messages = msgstore->get_undelivered_outgoing_messages_sorted();

  for (auto& m : messages) {
    auto message_info = getOutboxMessagesResponse->add_messages();

    auto baseMessage = message_info->mutable_m();
    baseMessage->set_id(m.id);
    baseMessage->set_message(m.message);
    message_info->set_to(m.to);
    message_info->set_delivered(m.delivered);

    auto timestamp_str = absl::FormatTime(m.written_timestamp);
    auto timestamp = message_info->mutable_written_timestamp();
    auto success = TimeUtil::FromString(timestamp_str, timestamp);
    if (!success) {
      cout << "invalid timestamp" << endl;
      return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
    }
  }

  return Status::OK;
}

Status DaemonRpc::GetSentMessages(
    ServerContext* context,
    const asphrdaemon::GetSentMessagesRequest* getSentMessagesRequest,
    asphrdaemon::GetSentMessagesResponse* getSentMessagesResponse) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  cout << "GetSentMessages() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto messages = msgstore->get_delivered_outgoing_messages_sorted();

  for (auto& m : messages) {
    auto message_info = getSentMessagesResponse->add_messages();

    auto baseMessage = message_info->mutable_m();
    baseMessage->set_id(m.id);
    baseMessage->set_message(m.message);
    message_info->set_to(m.to);
    message_info->set_delivered(m.delivered);

    auto timestamp_str = absl::FormatTime(m.written_timestamp);
    auto timestamp = message_info->mutable_written_timestamp();
    auto success = TimeUtil::FromString(timestamp_str, timestamp);
    if (!success) {
      cout << "invalid timestamp" << endl;
      return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
    }
  }

  return Status::OK;
}

Status DaemonRpc::MessageSeen(
    ServerContext* context,
    const asphrdaemon::MessageSeenRequest* messageSeenRequest,
    asphrdaemon::MessageSeenResponse* messageSeenResponse) {
  cout << "MessageSeen() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto message_id = messageSeenRequest->id();

  auto status = msgstore->mark_message_as_seen(message_id);
  if (!status.ok()) {
    cout << "mark message as seen failed" << endl;
    return Status(grpc::StatusCode::UNKNOWN, "mark message as seen failed");
  }

  return Status::OK;
}

auto DaemonRpc::GetStatus(ServerContext* context,
                          const asphrdaemon::GetStatusRequest* getStatusRequest,
                          asphrdaemon::GetStatusResponse* getStatusResponse)
    -> Status {
  cout << "GetStatus() called" << endl;

  getStatusResponse->set_registered(config->has_registered());
  getStatusResponse->set_release_hash(RELEASE_COMMIT_HASH);
  getStatusResponse->set_latency_seconds(config->get_latency_seconds());

  return Status::OK;
}

auto DaemonRpc::GetLatency(
    ServerContext* context,
    const asphrdaemon::GetLatencyRequest* getLatencyRequest,
    asphrdaemon::GetLatencyResponse* getLatencyResponse) -> Status {
  cout << "GetLatency() called" << endl;

  getLatencyResponse->set_latency_seconds(config->get_latency_seconds());

  return Status::OK;
}

auto DaemonRpc::ChangeLatency(
    grpc::ServerContext* context,
    const asphrdaemon::ChangeLatencyRequest* changeLatencyRequest,
    asphrdaemon::ChangeLatencyResponse* changeLatencyResponse) -> Status {
  cout << "ChangeLatency() called" << endl;

  auto new_latency = changeLatencyRequest->latency_seconds();

  if (new_latency <= 0) {
    cout << "invalid latency" << endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid latency");
  }

  auto status = config->set_latency(new_latency);
  if (!status.ok()) {
    cout << "set latency failed" << endl;
    return Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "set latency failed");
  }

  return Status::OK;
}

auto DaemonRpc::Kill(ServerContext* context,
                     const asphrdaemon::KillRequest* killRequest,
                     asphrdaemon::KillResponse* killResponse) -> Status {
  cout << "Kill() called" << endl;
  config->kill();
  cout << "Will kill daemon ASAP" << endl;
  return Status::OK;
}