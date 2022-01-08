
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

    for (auto& s : FriendTable.keys()) {
      getFriendListResponse->add_friend_list(s);
    }

    return Status::OK;
  }

  Status GenerateFriendKey(
      ServerContext* context,
      const Daemon::GenerateFriendKeyRequest* generateFriendKeyRequest,
      Daemon::GenerateFriendKeyResponse* generateFriendKeyResponse) override {
    cout << "GenerateFriendKey() called" << endl;

    if (RegisterationInfo.allocation.size() <= FriendTable.size()) {
      cout << "no more allocation" << endl;
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "no more allocation");
    }
  }

 private:
  Crypto& crypto;
};