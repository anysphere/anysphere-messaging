//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "daemon_rpc.hpp"

#include "google/protobuf/util/time_util.h"

using namespace asphrdaemon;

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

Status DaemonRpc::RegisterUser(ServerContext* context,
                               const RegisterUserRequest* registerUserRequest,
                               RegisterUserResponse* registerUserResponse) {
  cout << "RegisterUser() called" << endl;

  if (config->has_registered()) {
    cout << "already registered" << endl;
    return Status(grpc::StatusCode::ALREADY_EXISTS, "already registered");
  }

  const auto name = registerUserRequest->name();
  const auto [public_key, secret_key] = crypto.generate_keypair();

  auto beta_key = registerUserRequest->beta_key();

  // call register rpc to send the register request
  asphrserver::RegisterInfo request;
  request.set_public_key(public_key);
  request.set_beta_key(beta_key);

  asphrserver::RegisterResponse reply;
  grpc::ClientContext client_context;

  Status status = stub->Register(&client_context, request, &reply);

  if (status.ok()) {
    cout << "register success" << endl;

    const auto authentication_token = reply.authentication_token();
    auto alloc_repeated = reply.allocation();
    const auto allocation =
        vector<int>(alloc_repeated.begin(), alloc_repeated.end());

    if (reply.authentication_token() == "") {
      cout << "authentication token is empty" << endl;
      return Status(grpc::StatusCode::UNAUTHENTICATED,
                    "authentication token is empty");
    }
    if (reply.allocation().empty()) {
      cout << "allocation is empty" << endl;
      return Status(grpc::StatusCode::UNKNOWN, "allocation is empty");
    }

    config->do_register(name, public_key, secret_key, authentication_token,
                        allocation);

  } else {
    cout << status.error_code() << ": " << status.error_message() << endl;
    return Status(grpc::StatusCode::UNAVAILABLE, status.error_message());
  }

  return Status::OK;
}

Status DaemonRpc::GetFriendList(
    ServerContext* context, const GetFriendListRequest* getFriendListRequest,
    GetFriendListResponse* getFriendListResponse) {
  cout << "GetFriendList() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
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
    const GenerateFriendKeyRequest* generateFriendKeyRequest,
    GenerateFriendKeyResponse* generateFriendKeyResponse) {
  cout << "GenerateFriendKey() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
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

Status DaemonRpc::AddFriend(ServerContext* context,
                            const AddFriendRequest* addFriendRequest,
                            AddFriendResponse* addFriendResponse) {
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

  auto friend_info = friend_info_status.value();

  auto decoded_friend_key = crypto.decode_friend_key(addFriendRequest->key());
  if (!decoded_friend_key.ok()) {
    cout << "invalid friend key" << endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid friend key");
  }

  auto& [read_index, friend_public_key] = decoded_friend_key.value();

  auto [read_key, write_key] = crypto.derive_read_write_keys(
      config->registration_info().public_key,
      config->registration_info().private_key, friend_public_key);
  friend_info.read_key = read_key;
  friend_info.write_key = write_key;
  friend_info.read_index = read_index;
  friend_info.enabled = true;

  config->update_friend(friend_info);

  return Status::OK;
}

Status DaemonRpc::RemoveFriend(ServerContext* context,
                               const RemoveFriendRequest* removeFriendRequest,
                               RemoveFriendResponse* removeFriendResponse) {
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

Status DaemonRpc::SendMessage(ServerContext* context,
                              const SendMessageRequest* sendMessageRequest,
                              SendMessageResponse* sendMessageResponse) {
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

Status DaemonRpc::GetAllMessages(
    ServerContext* context, const GetAllMessagesRequest* getAllMessagesRequest,
    GetAllMessagesResponse* getAllMessagesResponse) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  cout << "GetAllMessages() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto messages = msgstore->get_all_incoming_messages_sorted();

  for (auto& m : messages) {
    auto message_info = getAllMessagesResponse->add_messages();

    auto baseMessage = message_info->mutable_m();
    baseMessage->set_id(m.id);
    baseMessage->set_message(m.message);
    message_info->set_from(m.from);
    message_info->set_seen(m.seen);

    // TODO: do this conversion not through strings....
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
Status DaemonRpc::GetAllMessagesStreamed(
    ServerContext* context, const GetAllMessagesRequest* request,
    ServerWriter<GetAllMessagesResponse>* writer) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  cout << "GetAllMessagesStreamed() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  // the messages may not be added in the right order. so what we do is
  // we wait until the last_mono_index has changed, and then get all messages
  // from that point on.
  // this needs to happen before we get all the messages below.
  //
  // the caller will always get *each message AT LEAST once*, in the order of the
  // mono index. *this may or may not correspond to the timestamp order.*
  // the at least is because the mono index may be increasing while we're getting
  // messages.
  int last_mono_index;
  {
    std::lock_guard<std::mutex> l(msgstore->add_cv_mtx);
    last_mono_index = msgstore->last_mono_index;
  }

  {
    auto messages = msgstore->get_all_incoming_messages_sorted();

    GetAllMessagesResponse response;

    for (auto& m : messages) {
      auto message_info = response.add_messages();

      auto baseMessage = message_info->mutable_m();
      baseMessage->set_id(m.id);
      baseMessage->set_message(m.message);
      message_info->set_from(m.from);
      message_info->set_seen(m.seen);

      // TODO: do this conversion not through strings....
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
      last_mono_index_here = msgstore->last_mono_index;
    }

    auto messages = msgstore->get_incoming_messages_sorted_after(last_mono_index);

    GetAllMessagesResponse response;

    for (auto& m : messages) {
      auto message_info = response.add_messages();

      auto baseMessage = message_info->mutable_m();
      baseMessage->set_id(m.id);
      baseMessage->set_message(m.message);
      message_info->set_from(m.from);
      message_info->set_seen(m.seen);

      // TODO: do this conversion not through strings....
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

Status DaemonRpc::GetNewMessages(
    ServerContext* context, const GetNewMessagesRequest* getNewMessagesRequest,
    GetNewMessagesResponse* getNewMessagesResponse) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  cout << "GetNewMessages() called" << endl;

  if (!config->has_registered()) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto messages = msgstore->get_new_incoming_messages_sorted();

  for (auto& m : messages) {
    auto message_info = getNewMessagesResponse->add_messages();

    auto baseMessage = message_info->mutable_m();
    baseMessage->set_id(m.id);
    baseMessage->set_message(m.message);
    message_info->set_from(m.from);
    message_info->set_seen(m.seen);

    // TODO: do this conversion not through strings....
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

Status DaemonRpc::GetOutboxMessages(
    ServerContext* context,
    const GetOutboxMessagesRequest* getOutboxMessagesRequest,
    GetOutboxMessagesResponse* getOutboxMessagesResponse) {
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
    const GetSentMessagesRequest* getSentMessagesRequest,
    GetSentMessagesResponse* getSentMessagesResponse) {
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

Status DaemonRpc::MessageSeen(ServerContext* context,
                              const MessageSeenRequest* messageSeenRequest,
                              MessageSeenResponse* messageSeenResponse) {
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
                          const GetStatusRequest* getStatusRequest,
                          GetStatusResponse* getStatusResponse) -> Status {
  cout << "GetStatus() called" << endl;

  getStatusResponse->set_registered(config->has_registered());
  getStatusResponse->set_release_hash(RELEASE_COMMIT_HASH);

  return Status::OK;
}

auto DaemonRpc::GetLatency(ServerContext* context,
                           const GetLatencyRequest* getLatencyRequest,
                           GetLatencyResponse* getLatencyResponse) -> Status {
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

auto DaemonRpc::Kill(ServerContext* context, const KillRequest* killRequest,
                     KillResponse* killResponse) -> Status {
  cout << "Kill() called" << endl;
  config->kill();
  cout << "Will kill daemon ASAP" << endl;
  return Status::OK;
}