#include "as_cli.hpp"

auto Friend::generate_key(unique_ptr<asphrdaemon::Daemon::Stub>& stub)
    -> asphr::StatusOr<string> {
  grpc::ClientContext context;
  asphrdaemon::GenerateFriendKeyRequest request;
  asphrdaemon::GenerateFriendKeyResponse response;

  request.set_name(name_);

  grpc::Status status = stub->GenerateFriendKey(&context, request, &response);

  if (!status.ok() || !response.success()) {
    cout << "generate friend key failed: " << status.error_message() << endl;
    return absl::UnknownError("generate friend key failed");
  } else {
    cout << "friend key generated" << endl;
    return response.key();
  }
}

auto Friend::add(unique_ptr<asphrdaemon::Daemon::Stub>& stub, const string& key)
    -> asphr::Status {
  grpc::ClientContext context;
  asphrdaemon::AddFriendRequest request;
  asphrdaemon::AddFriendResponse response;

  request.set_name(name_);
  request.set_key(key);

  grpc::Status status = stub->AddFriend(&context, request, &response);

  if (!status.ok() || !response.success()) {
    cout << "add friend failed: " << status.error_message() << endl;
    return absl::UnknownError("add friend failed");
  } else {
    cout << "friend added" << endl;
    return absl::OkStatus();
  }
}

bool Friend::complete() const { return !name_is_empty(); }