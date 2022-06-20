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

  if (G.db->has_registered()) {
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
      new_friend->set_unique_name(std::string(s.unique_name));
      new_friend->set_display_name(std::string(s.display_name));
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
  auto friend_key =
      crypto::generate_friend_key(rust_u8Vec_to_string(reg.public_key), index);

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
      rust_u8Vec_to_string(reg.public_key),
      rust_u8Vec_to_string(reg.private_key), friend_public_key);

  try {
    G.db->add_friend_address(
        (db::AddAddress){
            .unique_name = addFriendRequest->unique_name(),
            .read_index = read_index,
            .read_key = string_to_rust_u8Vec(read_key),
            .write_key = string_to_rust_u8Vec(write_key),
        },
        MAX_FRIENDS);
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

  auto message = sendMessageRequest->message();

  // chunk up the message and add it!
  // TODO: we probably want to use a protobuf for the entire message
  // and then chunk the resulting protobuf bytestring.
  // this is necessary for when we start supporting images, for example
  // for now, this is fine.
  rust::Vec<rust::String> chunked_message;
  for (size_t i = 0; i < message.size(); i += GUARANTEED_SINGLE_MESSAGE_SIZE) {
    chunked_message.push_back(
        message.substr(i, GUARANTEED_SINGLE_MESSAGE_SIZE));
  }

  try {
    G.db->queue_message_to_send(sendMessageRequest->unique_name(), message,
                                chunked_message);
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
        .limit = -1,  // TODO: make this configurable
        .filter = filter == asphrdaemon::GetMessagesRequest::ALL
                      ? db::MessageFilter::All
                      : db::MessageFilter::New,
        .delivery_status = db::DeliveryStatus::Delivered,
        .sort_by = db::SortBy::DeliveredAt,
        .after = 0});

    for (auto& m : messages) {
      auto message_info = getMessagesResponse->add_messages();

      auto baseMessage = message_info->mutable_m();
      baseMessage->set_id(m.uid);
      baseMessage->set_message(std::string(m.content));
      baseMessage->set_unique_name(std::string(m.from_unique_name));
      baseMessage->set_display_name(std::string(m.from_display_name));
      message_info->set_seen(m.seen);
      message_info->set_delivered(m.delivered);

      auto delivered_at = absl::FromUnixMicros(m.delivered_at);
      auto timestamp_str = absl::FormatTime(delivered_at);
      auto timestamp = message_info->mutable_delivered_at();
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
  ASPHR_LOG_INFO("GetStreamedMessages() called.", rpc_call,
                 "GetStreamedMessages");

  if (!G.db->has_registered()) {
    ASPHR_LOG_ERR("Need to register first.", rpc_call, "GetStreamedMessages");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto filter = getMessagesRequest->filter();

  // guarantee: the caller will get each message in the order of the
  // delivered_at timestamp. each message will be delivered exactly once.
  absl::Time last_delivered_at = absl::InfinitePast();

  const auto base_query = db::MessageQuery{
      .limit = -1,  // TODO: make this configurable
      .filter = filter == asphrdaemon::GetMessagesRequest::ALL
                    ? db::MessageFilter::All
                    : db::MessageFilter::New,
      .delivery_status = db::DeliveryStatus::Delivered,
      .sort_by = db::SortBy::DeliveredAt,
      .after = 0,
  };

  try {
    auto messages = G.db->get_received_messages(base_query);

    asphrdaemon::GetMessagesResponse response;

    for (auto& m : messages) {
      auto message_info = response.add_messages();

      auto baseMessage = message_info->mutable_m();
      baseMessage->set_id(m.uid);
      baseMessage->set_message(std::string(m.content));
      baseMessage->set_unique_name(std::string(m.from_unique_name));
      baseMessage->set_display_name(std::string(m.from_display_name));
      message_info->set_seen(m.seen);
      message_info->set_delivered(m.delivered);

      auto delivered_at = absl::FromUnixMicros(m.delivered_at);
      if (delivered_at > last_delivered_at) {
        last_delivered_at = delivered_at;
      }
      auto timestamp_str = absl::FormatTime(delivered_at);
      auto timestamp = message_info->mutable_delivered_at();
      auto success = TimeUtil::FromString(timestamp_str, timestamp);
      if (!success) {
        ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                      rpc_call, "GetMessagesStreamed");
        return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
      }
    }

    writer->Write(response);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get messages.", error, e.what(), rpc_call,
                  "GetMessagesStreamed");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  // keep the connection open forever
  while (!context->IsCancelled()) {
    {
      std::unique_lock<std::mutex> l(G.message_notification_cv_mutex);
      G.message_notification_cv.wait_for(l, std::chrono::seconds(60 * 60), [&] {
        return G.db->get_most_recent_delivered_at(base_query) !=
               absl::ToUnixMicros(last_delivered_at);
      });
      if (G.db->get_most_recent_delivered_at(base_query) ==
          absl::ToUnixMicros(last_delivered_at)) {
        continue;  // continue the loop to check if we are cancelled, because it
                   // has been an hour
      }
    }

    // same code as above. only difference: the .after = last_delivered_at
    try {
      auto messages = G.db->get_received_messages(db::MessageQuery{
          .limit = -1,  // TODO: make this configurable
          .filter = filter == asphrdaemon::GetMessagesRequest::ALL
                        ? db::MessageFilter::All
                        : db::MessageFilter::New,
          .delivery_status = db::DeliveryStatus::Delivered,
          .sort_by = db::SortBy::DeliveredAt,
          .after = absl::ToUnixMicros(last_delivered_at),
      });

      asphrdaemon::GetMessagesResponse response;

      for (auto& m : messages) {
        auto message_info = response.add_messages();

        auto baseMessage = message_info->mutable_m();
        baseMessage->set_id(m.uid);
        baseMessage->set_message(std::string(m.content));
        baseMessage->set_unique_name(std::string(m.from_unique_name));
        baseMessage->set_display_name(std::string(m.from_display_name));
        message_info->set_seen(m.seen);
        message_info->set_delivered(m.delivered);

        auto delivered_at = absl::FromUnixMicros(m.delivered_at);
        if (delivered_at > last_delivered_at) {
          last_delivered_at = delivered_at;
        }
        auto timestamp_str = absl::FormatTime(delivered_at);
        auto timestamp = message_info->mutable_delivered_at();
        auto success = TimeUtil::FromString(timestamp_str, timestamp);
        if (!success) {
          ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                        rpc_call, "GetMessagesStreamed");
          return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
        }
      }

      writer->Write(response);
    } catch (const rust::Error& e) {
      ASPHR_LOG_ERR("Failed to get messages.", error, e.what(), rpc_call,
                    "GetMessagesStreamed");
      return Status(grpc::StatusCode::UNKNOWN, e.what());
    }
  }

  return Status::OK;
}

Status DaemonRpc::GetOutboxMessages(
    ServerContext* context,
    const asphrdaemon::GetOutboxMessagesRequest* getOutboxMessagesRequest,
    asphrdaemon::GetOutboxMessagesResponse* getOutboxMessagesResponse) {
  // TODO: somehow merge this with GetSentMessaegs because they are very similar
  using TimeUtil = google::protobuf::util::TimeUtil;
  ASPHR_LOG_INFO("GetOutboxMessages() called.", rpc_call, "GetOutboxMessages");

  if (!G.db->has_registered()) {
    ASPHR_LOG_ERR("Need to register first.", rpc_call, "GetOutboxMessages");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    auto messages = G.db->get_sent_messages(
        db::MessageQuery{.limit = -1,  // TODO: make this configurable
                         .filter = db::MessageFilter::New,
                         .delivery_status = db::DeliveryStatus::Undelivered,
                         .sort_by = db::SortBy::DeliveredAt,
                         .after = 0});

    for (auto& m : messages) {
      auto message_info = getOutboxMessagesResponse->add_messages();

      auto baseMessage = message_info->mutable_m();
      baseMessage->set_id(m.uid);
      baseMessage->set_message(std::string(m.content));
      baseMessage->set_unique_name(std::string(m.to_unique_name));
      baseMessage->set_display_name(std::string(m.to_display_name));
      message_info->set_delivered(m.delivered);

      {
        auto delivered_at = absl::FromUnixMicros(m.delivered_at);
        auto timestamp_str = absl::FormatTime(delivered_at);
        auto timestamp = message_info->mutable_delivered_at();
        auto success = TimeUtil::FromString(timestamp_str, timestamp);
        if (!success) {
          ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                        rpc_call, "GetSentMessages");
          return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
        }
      }
      {
        auto sent_at = absl::FromUnixMicros(m.sent_at);
        auto timestamp_str = absl::FormatTime(sent_at);
        auto timestamp = message_info->mutable_sent_at();
        auto success = TimeUtil::FromString(timestamp_str, timestamp);
        if (!success) {
          ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                        rpc_call, "GetSentMessages");
          return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
        }
      }
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get messages.", error, e.what(), rpc_call,
                  "GetMessages");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::GetSentMessages(
    ServerContext* context,
    const asphrdaemon::GetSentMessagesRequest* getSentMessagesRequest,
    asphrdaemon::GetSentMessagesResponse* getSentMessagesResponse) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  ASPHR_LOG_INFO("GetSentMessages() called.", rpc_call, "GetSentMessages");

  if (!G.db->has_registered()) {
    ASPHR_LOG_ERR("Need to register first.", rpc_call, "GetSentMessages");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    auto messages = G.db->get_sent_messages(
        db::MessageQuery{.limit = -1,  // TODO: make this configurable
                         .filter = db::MessageFilter::New,
                         .delivery_status = db::DeliveryStatus::Delivered,
                         .sort_by = db::SortBy::DeliveredAt,
                         .after = 0});

    for (auto& m : messages) {
      auto message_info = getSentMessagesResponse->add_messages();

      auto baseMessage = message_info->mutable_m();
      baseMessage->set_id(m.uid);
      baseMessage->set_message(std::string(m.content));
      baseMessage->set_unique_name(std::string(m.to_unique_name));
      baseMessage->set_display_name(std::string(m.to_display_name));
      message_info->set_delivered(m.delivered);

      {
        auto delivered_at = absl::FromUnixMicros(m.delivered_at);
        auto timestamp_str = absl::FormatTime(delivered_at);
        auto timestamp = message_info->mutable_delivered_at();
        auto success = TimeUtil::FromString(timestamp_str, timestamp);
        if (!success) {
          ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                        rpc_call, "GetSentMessages");
          return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
        }
      }
      {
        auto sent_at = absl::FromUnixMicros(m.sent_at);
        auto timestamp_str = absl::FormatTime(sent_at);
        auto timestamp = message_info->mutable_sent_at();
        auto success = TimeUtil::FromString(timestamp_str, timestamp);
        if (!success) {
          ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                        rpc_call, "GetSentMessages");
          return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
        }
      }
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get messages.", error, e.what(), rpc_call,
                  "GetMessages");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::MessageSeen(
    ServerContext* context,
    const asphrdaemon::MessageSeenRequest* messageSeenRequest,
    asphrdaemon::MessageSeenResponse* messageSeenResponse) {
  ASPHR_LOG_INFO("MessageSeen() called.", rpc_call, "MessageSeen");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "MessageSeen");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto message_id = messageSeenRequest->id();

  try {
    G.db->mark_message_as_seen(message_id);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to mark message as seen.", error, e.what(), rpc_call,
                  "MessageSeen");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

auto DaemonRpc::GetStatus(ServerContext* context,
                          const asphrdaemon::GetStatusRequest* getStatusRequest,
                          asphrdaemon::GetStatusResponse* getStatusResponse)
    -> Status {
  ASPHR_LOG_INFO("GetStatus() called.", rpc_call, "GetStatus");

  try {
    getStatusResponse->set_registered(G.db->has_registered());
    getStatusResponse->set_release_hash(RELEASE_COMMIT_HASH);
    getStatusResponse->set_latency_seconds(G.db->get_latency());
    getStatusResponse->set_server_address(
        std::string(G.db->get_server_address()));
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get status.", error, e.what(), rpc_call,
                  "GetStatus");
    return Status(grpc::StatusCode::RESOURCE_EXHAUSTED, e.what());
  }

  return Status::OK;
}

auto DaemonRpc::GetLatency(
    ServerContext* context,
    const asphrdaemon::GetLatencyRequest* getLatencyRequest,
    asphrdaemon::GetLatencyResponse* getLatencyResponse) -> Status {
  ASPHR_LOG_INFO("GetLatency() called.", rpc_call, "GetLatency");

  try {
    getLatencyResponse->set_latency_seconds(G.db->get_latency());
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get latency.", error, e.what(), rpc_call,
                  "GetLatency");
    return Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "get latency failed");
  }

  return Status::OK;
}

auto DaemonRpc::ChangeLatency(
    grpc::ServerContext* context,
    const asphrdaemon::ChangeLatencyRequest* changeLatencyRequest,
    asphrdaemon::ChangeLatencyResponse* changeLatencyResponse) -> Status {
  ASPHR_LOG_INFO("ChangeLatency() called.", rpc_call, "ChangeLatency");

  auto new_latency = changeLatencyRequest->latency_seconds();

  if (new_latency <= 0) {
    ASPHR_LOG_ERR("Invalid latency <= 0.", new_latency, new_latency, rpc_call,
                  "ChangeLatency");
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid latency");
  }

  try {
    G.db->set_latency(new_latency);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to change latency.", error, e.what(), rpc_call,
                  "ChangeLatency");
    return Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "set latency failed");
  }

  return Status::OK;
}

auto DaemonRpc::Kill(ServerContext* context,
                     const asphrdaemon::KillRequest* killRequest,
                     asphrdaemon::KillResponse* killResponse) -> Status {
  ASPHR_LOG_INFO("Kill() called.", rpc_call, "Kill");
  G.kill();
  ASPHR_LOG_INFO("Daemon is shutting down asap.", rpc_call, "Kill");
  return Status::OK;
}