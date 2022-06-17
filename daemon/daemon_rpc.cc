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
  ASPHR_LOG_INFO("RegisterUser() called.");

  if (config->has_registered()) {
    ASPHR_LOG_INFO("already registered");
    return Status(grpc::StatusCode::ALREADY_EXISTS, "already registered");
  }

  const auto name = registerUserRequest->name();
  const auto [public_key, secret_key] = crypto.generate_keypair();
  const auto [friend_request_public_key, friend_request_secret_key] =
      crypto.generate_friend_request_keypair();

  auto beta_key = registerUserRequest->beta_key();

  // call register rpc to send the register request
  asphrserver::RegisterInfo request;
  request.set_public_key(public_key);
  request.set_beta_key(beta_key);

  asphrserver::RegisterResponse reply;
  grpc::ClientContext client_context;

  Status status = stub->Register(&client_context, request, &reply);

  // TODO: we need to send the friend_request public key to the PKI.
  // Whatever that is.

  if (status.ok()) {
    ASPHR_LOG_INFO("register success");

    const auto authentication_token = reply.authentication_token();
    auto alloc_repeated = reply.allocation();
    const auto allocation =
        vector<int>(alloc_repeated.begin(), alloc_repeated.end());

    if (reply.authentication_token() == "") {
      ASPHR_LOG_ERR("authentication token is empty");
      return Status(grpc::StatusCode::UNAUTHENTICATED,
                    "authentication token is empty");
    }
    if (reply.allocation().empty()) {
      ASPHR_LOG_ERR("allocation is empty");
      return Status(grpc::StatusCode::UNKNOWN, "allocation is empty");
    }

    config->do_register(name, public_key, secret_key, friend_request_public_key,
                        friend_request_secret_key, authentication_token,
                        allocation);

  } else {
    ASPHR_LOG_ERR("register failed", error_code, status.error_code(),
                  error_message, status.error_message());
    return Status(grpc::StatusCode::UNAVAILABLE, status.error_message());
  }

  return Status::OK;
}

Status DaemonRpc::GetFriendList(
    ServerContext* context,
    const asphrdaemon::GetFriendListRequest* getFriendListRequest,
    asphrdaemon::GetFriendListResponse* getFriendListResponse) {
  ASPHR_LOG_INFO("GetFriendList() called.");

  if (!config->has_registered()) {
    ASPHR_LOG_INFO("need to register first");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  for (auto& s : config->friends()) {
    auto new_friend = getFriendListResponse->add_friend_infos();
    new_friend->set_name(s.name);
    new_friend->set_enabled(s.enabled);
  }

  return Status::OK;
}

Status DaemonRpc::GenerateFriendKey(
    ServerContext* context,
    const asphrdaemon::GenerateFriendKeyRequest* generateFriendKeyRequest,
    asphrdaemon::GenerateFriendKeyResponse* generateFriendKeyResponse) {
  ASPHR_LOG_INFO("GenerateFriendKey() called.");

  if (!config->has_registered()) {
    ASPHR_LOG_INFO("need to register first");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  const auto friend_info_status =
      config->get_friend(generateFriendKeyRequest->name());
  if (friend_info_status.ok()) {
    const auto friend_info = friend_info_status.value();
    if (friend_info.enabled) {
      cout << "friend already exists" << endl;
      return Status(grpc::StatusCode::ALREADY_EXISTS, "friend already exists");
    } else {
      generateFriendKeyResponse->set_key(friend_info.add_key);
      return Status::OK;
    }
  }

  if (!config->has_space_for_friends()) {
    cout << "no more allocation" << endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "no more allocation");
  }

  // note: for now, we only support the first index ever!
  auto registration_info = config->registration_info();
  auto index = registration_info.allocation.at(0);

  auto friend_key =
      crypto.generate_friend_key(registration_info.public_key, index);

  auto friend_info =
      Friend(generateFriendKeyRequest->name(), config->friends(), friend_key);

  config->add_friend(friend_info);

  generateFriendKeyResponse->set_key(friend_key);
  return Status::OK;
}

Status DaemonRpc::AddFriend(
    ServerContext* context,
    const asphrdaemon::AddFriendRequest* addFriendRequest,
    asphrdaemon::AddFriendResponse* addFriendResponse) {
  cout << "AddFriend() called" << endl;
  cout << "name: " << addFriendRequest->name() << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto friend_info_status = config->get_friend(addFriendRequest->name());

  if (!friend_info_status.ok()) {
    cout << "friend not found; call generatefriendkey first!" << endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT,
                  "friend not found; call generatefriendkey first!");
  }

  auto decoded_friend_key = crypto.decode_friend_key(addFriendRequest->key());
  if (!decoded_friend_key.ok()) {
    cout << "invalid friend key" << endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid friend key");
  }

  auto& [read_index, friend_public_key] = decoded_friend_key.value();

  auto [read_key, write_key] = crypto.derive_read_write_keys(
      config->registration_info().public_key,
      config->registration_info().private_key, friend_public_key);

  config->update_friend(addFriendRequest->name(), {.read_index = read_index,
                                                   .read_key = read_key,
                                                   .write_key = write_key,
                                                   .enabled = true});

  return Status::OK;
}

// send an async friend request
// currently, we need to supply several pieces of information:
// 1. the allocation index
// 2. the public keys for kx exchange and for the friend request
// 3. the local name for the friend
Status DaemonRpc::SendFriendRequestAsync(
    ServerContext* context,
    const asphrdaemon::SendFriendRequestAsyncRequest* request,
    asphrdaemon::SendFriendRequestAsyncResponse* response) {
  ASPHR_LOG_INFO("SendFriendRequestAsync() called.");
  // TODO: for simplicity, we only support adding one friend asynchronously at a
  // time This can be easily addressed later, but I need to finish testing
  // before 4pm

  // TODO: the architecture here will be refactored with the new inbox
  // so we need to change this later

  // make sure we have space
  if (!config->has_space_for_friends()) {
    return Status(grpc::StatusCode::INVALID_ARGUMENT,
                  "too many simultaneous async requests");
  }
  // use the kx key to initiate a friend instance
  string friend_kx_public_key = request->kx_public_key();
  string my_kx_public_key = config->registration_info().public_key;
  string my_kx_private_key = config->registration_info().private_key;
  auto friend_instance =
      Friend(request->name(), config->friends(), friend_kx_public_key);

  // we can generate the read write key now
  auto [read_key, write_key] = crypto.derive_read_write_keys(
      my_kx_public_key, my_kx_private_key, friend_kx_public_key);

  friend_instance.read_key = read_key;
  friend_instance.write_key = write_key;

  // push the request to the config
  config->add_async_friend_request(request->friend_request_public_key(),
                                   friend_instance);

  return Status::OK;
}

Status DaemonRpc::RemoveFriend(
    ServerContext* context,
    const asphrdaemon::RemoveFriendRequest* removeFriendRequest,
    asphrdaemon::RemoveFriendResponse* removeFriendResponse) {
  cout << "RemoveFriend() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto friend_info_status = config->get_friend(removeFriendRequest->name());

  if (!friend_info_status.ok()) {
    cout << "friend not found" << endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "friend not found");
  }

  // remove friend from friend table
  auto status = config->remove_friend(removeFriendRequest->name());

  if (!status.ok()) {
    cout << "remove friend failed" << endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "remove friend failed");
  }

  return Status::OK;
}

Status DaemonRpc::SendMessage(
    ServerContext* context,
    const asphrdaemon::SendMessageRequest* sendMessageRequest,
    asphrdaemon::SendMessageResponse* sendMessageResponse) {
  cout << "SendMessage() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto friend_info_status = config->get_friend(sendMessageRequest->name());

  if (!friend_info_status.ok()) {
    cout << "friend not found" << endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "friend not found");
  }

  auto friend_info = friend_info_status.value();

  if (!friend_info.enabled) {
    cout << "friend disabled" << endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "friend disabled");
  }

  auto message = sendMessageRequest->message();

  auto status = msgstore->add_outgoing_message(friend_info.name, message);
  if (!status.ok()) {
    cout << "write message to file failed" << endl;
    return Status(grpc::StatusCode::UNKNOWN, "write message failed");
  }

  return Status::OK;
}

Status DaemonRpc::GetMessages(
    ServerContext* context,
    const asphrdaemon::GetMessagesRequest* getMessagesRequest,
    asphrdaemon::GetMessagesResponse* getMessagesResponse) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  cout << "GetMessages() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto filter = getMessagesRequest->filter();

  vector<IncomingMessage> messages;

  if (filter == asphrdaemon::GetMessagesRequest::ALL) {
    messages = msgstore->get_all_incoming_messages_sorted();
  } else if (filter == asphrdaemon::GetMessagesRequest::NEW) {
    messages = msgstore->get_new_incoming_messages_sorted();
  } else {
    cout << "filter: INVALID" << endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid filter");
  }

  for (auto& m : messages) {
    auto message_info = getMessagesResponse->add_messages();

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