#include "daemon_rpc.hpp"

#include "google/protobuf/util/time_util.h"
#include "utils.hpp"

using namespace asphrdaemon;

using grpc::ServerContext;
using grpc::Status;

Status DaemonRpc::RegisterUser(ServerContext* context,
                               const RegisterUserRequest* registerUserRequest,
                               RegisterUserResponse* registerUserResponse) {
  cout << "RegisterUser() called" << endl;

  if (config.has_registered) {
    cout << "already registered" << endl;
    return Status(grpc::StatusCode::ALREADY_EXISTS, "already registered");
  }

  auto [public_key, secret_key] = crypto.generate_keypair();

  config.registrationInfo = RegistrationInfo{
      .name = registerUserRequest->name(),
      .public_key = public_key,
      .private_key = secret_key,
  };

  // call register rpc to send the register request
  asphrserver::RegisterInfo request;
  request.set_public_key(public_key);

  asphrserver::RegisterResponse reply;
  grpc::ClientContext client_context;

  Status status = stub->Register(&client_context, request, &reply);

  if (status.ok()) {
    cout << "register success" << endl;
    registerUserResponse->set_success(true);

    config.registrationInfo.authentication_token = reply.authentication_token();
    auto alloc_repeated = reply.allocation();
    config.registrationInfo.allocation =
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

    auto [secret_key, galois_keys] = generate_keys();
    config.pir_secret_key = secret_key;
    config.pir_galois_keys = galois_keys;

    assert(config.pir_client == nullptr);

    config.pir_client = std::make_unique<FastPIRClient>(config.pir_secret_key,
                                                        config.pir_galois_keys);
    config.initialize_dummy_me();
    // THIS has to happen last :)
    config.has_registered = true;

    config.save();
  } else {
    cout << status.error_code() << ": " << status.error_message() << endl;
    registerUserResponse->set_success(false);
    return Status(grpc::StatusCode::UNAVAILABLE, status.error_message());
  }

  return Status::OK;
}

Status DaemonRpc::GetFriendList(
    ServerContext* context, const GetFriendListRequest* getFriendListRequest,
    GetFriendListResponse* getFriendListResponse) {
  cout << "GetFriendList() called" << endl;

  if (!config.has_registered) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  for (auto& [s, _] : config.friendTable) {
    getFriendListResponse->add_friend_list(s);
  }

  getFriendListResponse->set_success(true);

  return Status::OK;
}

Status DaemonRpc::GenerateFriendKey(
    ServerContext* context,
    const GenerateFriendKeyRequest* generateFriendKeyRequest,
    GenerateFriendKeyResponse* generateFriendKeyResponse) {
  cout << "GenerateFriendKey() called" << endl;

  if (!config.has_registered) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  if (!config.has_space_for_friends()) {
    cout << "no more allocation" << endl;
    generateFriendKeyResponse->set_success(false);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "no more allocation");
  }

  // note: for now, we only support the first index ever!
  auto index = config.registrationInfo.allocation.at(0);

  auto friend_key =
      crypto.generate_friend_key(config.registrationInfo.public_key, index);

  auto friend_info =
      Friend(generateFriendKeyRequest->name(), config.friendTable);

  config.add_friend(friend_info);

  generateFriendKeyResponse->set_key(friend_key);
  generateFriendKeyResponse->set_success(true);
  return Status::OK;
}

Status DaemonRpc::AddFriend(ServerContext* context,
                            const AddFriendRequest* addFriendRequest,
                            AddFriendResponse* addFriendResponse) {
  cout << "AddFriend() called" << endl;
  cout << "name: " << addFriendRequest->name() << endl;

  if (!config.has_registered) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  for (auto& [s, _] : config.friendTable) {
    cout << "friend: " << s << endl;
  }

  if (!config.friendTable.contains(addFriendRequest->name())) {
    cout << "friend not found; call generatefriendkey first!" << endl;
    addFriendResponse->set_success(false);
    return Status(grpc::StatusCode::INVALID_ARGUMENT,
                  "friend not found; call generatefriendkey first!");
  }

  auto decoded_friend_key = crypto.decode_friend_key(addFriendRequest->key());
  if (!decoded_friend_key.ok()) {
    cout << "invalid friend key" << endl;
    addFriendResponse->set_success(false);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid friend key");
  }

  auto& [read_index, friend_public_key] = decoded_friend_key.value();

  auto& friend_info = config.friendTable.at(addFriendRequest->name());
  auto [read_key, write_key] = crypto.derive_read_write_keys(
      config.registrationInfo.public_key, config.registrationInfo.private_key,
      friend_public_key);
  friend_info.read_key = read_key;
  friend_info.write_key = write_key;
  friend_info.read_index = read_index;
  friend_info.enabled = true;
  config.save();

  addFriendResponse->set_success(true);
  return Status::OK;
}

Status DaemonRpc::RemoveFriend(ServerContext* context,
                               const RemoveFriendRequest* removeFriendRequest,
                               RemoveFriendResponse* removeFriendResponse) {
  cout << "RemoveFriend() called" << endl;

  if (!config.has_registered) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  if (!config.friendTable.contains(removeFriendRequest->name())) {
    cout << "friend not found" << endl;
    removeFriendResponse->set_success(false);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "friend not found");
  }

  // remove friend from friend table
  auto status = config.remove_friend(removeFriendRequest->name());

  if (!status.ok()) {
    cout << "remove friend failed" << endl;
    removeFriendResponse->set_success(false);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "remove friend failed");
  }

  removeFriendResponse->set_success(true);
  return Status::OK;
}

Status DaemonRpc::SendMessage(ServerContext* context,
                              const SendMessageRequest* sendMessageRequest,
                              SendMessageResponse* sendMessageResponse) {
  cout << "SendMessage() called" << endl;

  if (!config.has_registered) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  if (!config.friendTable.contains(sendMessageRequest->name())) {
    cout << "friend not found" << endl;
    sendMessageResponse->set_success(false);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "friend not found");
  }

  auto& friend_info = config.friendTable.at(sendMessageRequest->name());

  if (!friend_info.enabled) {
    cout << "friend disabled" << endl;
    sendMessageResponse->set_success(false);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "friend disabled");
  }

  auto message = sendMessageRequest->message();

  auto status = write_msg_to_file(config.send_file_address(), message,
                                  "MESSAGE", friend_info.name);
  if (!status.ok()) {
    cout << "write message to file failed" << endl;
    sendMessageResponse->set_success(false);
    return Status(grpc::StatusCode::UNKNOWN, "write message failed");
  }

  sendMessageResponse->set_success(true);
  return Status::OK;
}

Status DaemonRpc::GetAllMessages(
    ServerContext* context, const GetAllMessagesRequest* getAllMessagesRequest,
    GetAllMessagesResponse* getAllMessagesResponse) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  cout << "GetAllMessages() called" << endl;

  if (!config.has_registered) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto messages = get_entries(config.receive_file_address());

  // sort messages
  std::sort(messages.begin(), messages.end(), [](const json& a, const json& b) {
    auto timestamp_str = a.at("timestamp").get<string>();
    Time a_time;
    string err;
    absl::ParseTime(absl::RFC3339_full, timestamp_str, &a_time, &err);

    auto timestamp_str2 = b.at("timestamp").get<string>();
    Time b_time;
    absl::ParseTime(absl::RFC3339_full, timestamp_str2, &b_time, &err);

    // convert timestamp into time_t
    return a_time > b_time;
  });

  for (auto& message_json : messages) {
    auto id = message_id(message_json);

    auto message_info = getAllMessagesResponse->add_messages();

    message_info->set_id(id);
    message_info->set_sender(message_json.at("from").get<string>());
    message_info->set_message(message_json.at("message").get<string>());

    auto timestamp_str = message_json.at("timestamp").get<string>();

    // convert timestamp using TimeUtil::FromString
    auto timestamp = message_info->mutable_timestamp();
    auto success = TimeUtil::FromString(timestamp_str, timestamp);
    if (!success) {
      cout << "invalid timestamp" << endl;
      getAllMessagesResponse->set_success(false);
      return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
    }
  }

  getAllMessagesResponse->set_success(true);
  return Status::OK;
}

Status DaemonRpc::GetNewMessages(
    ServerContext* context, const GetNewMessagesRequest* getNewMessagesRequest,
    GetNewMessagesResponse* getNewMessagesResponse) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  cout << "GetNewMessages() called" << endl;

  if (!config.has_registered) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto messages = get_entries(config.receive_file_address());

  auto seen_messages = get_entries(config.seen_file_address());

  auto seen_set = std::unordered_set<string>();
  for (auto& seen_message : seen_messages) {
    seen_set.insert(seen_message.at("id").get<string>());
  }

  // sort messages
  std::sort(messages.begin(), messages.end(), [](const json& a, const json& b) {
    auto timestamp_str = a.at("timestamp").get<string>();
    Time a_time;
    string err;
    absl::ParseTime(absl::RFC3339_full, timestamp_str, &a_time, &err);

    auto timestamp_str2 = b.at("timestamp").get<string>();
    Time b_time;
    absl::ParseTime(absl::RFC3339_full, timestamp_str2, &b_time, &err);

    // convert timestamp into time_t
    return a_time > b_time;
  });

  for (auto& message_json : messages) {
    auto id = message_id(message_json);
    if (seen_set.contains(id)) {
      continue;
    }

    auto message_info = getNewMessagesResponse->add_messages();
    message_info->set_id(id);
    message_info->set_sender(message_json.at("from").get<string>());
    message_info->set_message(message_json.at("message").get<string>());

    auto timestamp_str = message_json.at("timestamp").get<string>();

    // convert timestamp using TimeUtil::FromString
    auto timestamp = message_info->mutable_timestamp();
    auto success = TimeUtil::FromString(timestamp_str, timestamp);
    if (!success) {
      cout << "invalid timestamp" << endl;
      getNewMessagesResponse->set_success(false);
      return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
    }
  }

  getNewMessagesResponse->set_success(true);
  return Status::OK;
}

Status DaemonRpc::MessageSeen(ServerContext* context,
                              const MessageSeenRequest* messageSeenRequest,
                              MessageSeenResponse* messageSeenResponse) {
  cout << "MessageSeen() called" << endl;

  if (!config.has_registered) {
    cout << "need to register first!" << endl;
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto message_id = messageSeenRequest->id();

  auto file = std::ofstream(config.seen_file_address(), std::ios_base::app);
  json jmsg = {{"id", message_id}};
  if (file.is_open()) {
    file << std::setw(4) << jmsg.dump() << std::endl;
    file.close();
  } else {
    return Status(grpc::StatusCode::UNKNOWN, "file is not open");
  }

  return Status::OK;
}

auto DaemonRpc::message_id(const asphr::json& message_json) -> string {
  return asphr::StrCat("from:", message_json.at("from").get<string>(), ":",
                       message_json.at("id").get<uint32_t>());
}