//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "as_cli.hpp"

// TODO: Due to API change, this file needs to be rewritten

/**auto Friend::generate_key(unique_ptr<asphrdaemon::Daemon::Stub>& stub)
    -> asphr::StatusOr<string> {
  grpc::ClientContext context;
  asphrdaemon::GenerateFriendKeyRequest request;
  asphrdaemon::GenerateFriendKeyResponse response;

  request.set_unique_name(name_);

  grpc::Status status = stub->GenerateFriendKey(&context, request, &response);

  if (!status.ok()) {
    cout << "generate friend key failed: " << status.error_message() << endl;
    return absl::UnknownError("generate friend key failed");
  }
  return response.key();
}

auto Friend::add(unique_ptr<asphrdaemon::Daemon::Stub>& stub, const string& key)
    -> asphr::Status {
  grpc::ClientContext context;
  asphrdaemon::AddFriendRequest request;
  asphrdaemon::AddFriendResponse response;

  request.set_unique_name(name_);
  // REMOVE
  cout << "name: " << name_ << endl;
  request.set_key(key);

  grpc::Status status = stub->AddFriend(&context, request, &response);

  if (!status.ok()) {
    cout << "add friend failed: " << status.error_message() << endl;
    return absl::UnknownError("add friend failed");
  }
  return absl::OkStatus();
}

bool Friend::complete() const { return !name_is_empty(); }**/
