
#include "config.hpp"
#include "schema/daemon.grpc.pb.h"

class DaemonImpl final : public Daemon::Service {
 public:
  Status RegisterUser(
      ServerContext* context,
      const Daemon::RegisterUserRequest* registerUserRequest,
      Daemon::RegisterUserResponse* registerUserResponse) override {
    cout << "RegisterUser() called" << endl;

    if (RegistrationInfo.has_registered) {
      cout << "already registered" << endl;
      continue;
    }

    auto [public_key, secret_key] = crypto.generate_keypair();

    // call register rpc to send the register request
    RegisterInfo request;
    request.set_public_key(public_key);

    RegistrationInfo.name = entry["name"].get<string>();
    RegistrationInfo.public_key = public_key;
    RegistrationInfo.private_key = private_key;

    RegisterResponse reply;
    ClientContext context;

    Status status = stub->Register(&context, request, &reply);

    if (status.ok()) {
      cout << "register success" << endl;
      registerUserResponse->set_success(true);

      RegistrationInfo.has_registered = true;
      RegistrationInfo.authentication_token = reply.authentication_token();
      auto alloc_repeated = reply.allocation();
      RegistrationInfo.allocation =
          vector<int>(alloc_repeated.begin(), alloc_repeated.end());

      if (reply.authentication_token() == "") {
        cout << "authentication token is empty" << endl;
        continue;
      }
      if (reply.allocation().empty()) {
        cout << "allocation is empty" << endl;
        continue;
      }

      auto [secret_key, galois_keys] = generate_keys();
      RegistrationInfo.pir_secret_key = secret_key;
      RegistrationInfo.pir_galois_keys = galois_keys;

      RegistrationInfo.store(config_file_address);
    } else {
      cout << status.error_code() << ": " << status.error_message() << endl;
      registerUserResponse->set_success(false);
      return Status(grpc::StatusCode::UNAVAILABLE, e.what());
    }

    return Status::OK;
  }

  Status GetFriendList(
      ServerContext* context,
      const Daemon::GetFriendListRequest* getFriendListRequest,
      Daemon::GetFriendListResponse* getFriendListResponse) override {
    cout << "GetFriendList() called" << endl;

    for (auto& s : config.friendTable.keys()) {
      getFriendListResponse->add_friend_list(s);
    }

    return Status::OK;
  }

  Status GenerateFriendKey(
      ServerContext* context,
      const Daemon::GenerateFriendKeyRequest* generateFriendKeyRequest,
      Daemon::GenerateFriendKeyResponse* generateFriendKeyResponse) override {
    cout << "GenerateFriendKey() called" << endl;

    if (!config.has_space_for_friends()) {
      cout << "no more allocation" << endl;
      generateFriendKeyResponse->set_success(false);
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "no more allocation");
    }

    auto index = config.registrationInfo.allocation[config.friendTable.size()];

    auto friend_key =
        crypto.generate_friend_key(config.registrationInfo.public_key, index);

    auto friend_info =
        Friend{generateFriendKeyRequest->name(), index, -1, "", "", false};
    config.friendTable[friend_info.name] = friend_info;

    generateFriendKeyResponse->set_friend_key(friend_key);
    generateFriendKeyResponse->set_success(true);
  }

  Status AddFriend(ServerContext* context,
                   const Daemon::AddFriendRequest* addFriendRequest,
                   Daemon::AddFriendResponse* addFriendResponse) override {
    cout << "AddFriend() called" << endl;

    if (!config.has_space_for_friends()) {
      cout << "no more allocation" << endl;
      addFriendResponse->set_success(false);
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "no more allocation");
    }

    if (!config.friendTable.contains(addFriendRequest->name())) {
      cout << "friend not found" << endl;
      addFriendResponse->set_success(false);
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "friend not found");
    }

    auto decoded_friend_key = crypto.decode_friend_key(addFriendRequest->key());
    if (!decoded_friend_key.ok()) {
      cout << "invalid friend key" << endl;
      addFriendResponse->set_success(false);
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid friend key");
    }

    auto& [read_index, friend_public_key] = decoded_friend_key.value();

    auto& friend_info = config.friendTable[addFriendRequest->name()];
    friend_info.public_key = friend_public_key;
    friend_info.read_index = read_index;
    friend_info.disabled = true;

    addFriendResponse->set_success(true);
    return Status::OK;
  }

  Status RemoveFriend(
      ServerContext* context,
      const Daemon::RemoveFriendRequest* removeFriendRequest,
      Daemon::RemoveFriendResponse* removeFriendResponse) override {
    cout << "RemoveFriend() called" << endl;

    if (!config.friendTable.contains(removeFriendRequest->name())) {
      cout << "friend not found" << endl;
      removeFriendResponse->set_success(false);
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "friend not found");
    }

    // remove friend from friend table
    config.friendTable.erase(removeFriendRequest->name());

    removeFriendResponse->set_success(true);
    return Status::OK;
  }

  Status SendMessage(
      ServerContext* context,
      const Daemon::SendMessageRequest* sendMessageRequest,
      Daemon::SendMessageResponse* sendMessageResponse) override {
    cout << "SendMessage() called" << endl;

    if (!config.friendTable.contains(sendMessageRequest->name())) {
      cout << "friend not found" << endl;
      sendMessageResponse->set_success(false);
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "friend not found");
    }

    auto& friend_info = config.friendTable[sendMessageRequest->name()];

    if (friend_info.disabled) {
      cout << "friend disabled" << endl;
      sendMessageResponse->set_success(false);
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "friend disabled");
    }

    auto message = sendMessageRequest->message();
    auto encrypted_message = crypto.encrypt_send(message, friend_info);

    auto& [read_index, ciphertext] = encrypted_message;

    auto& friend_info = config.friendTable[sendMessageRequest->name()];
    friend_info.read_index = read_index;

    sendMessageResponse->set_success(true);
    return Status::OK;
  }

 private:
  Crypto& crypto;
  Config& config;
};