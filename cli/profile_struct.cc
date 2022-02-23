//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "as_cli.hpp"

void Profile::add(unique_ptr<asphrdaemon::Daemon::Stub>& stub) {
  set_time();

  grpc::ClientContext context;
  asphrdaemon::RegisterUserRequest request;
  asphrdaemon::RegisterUserResponse reply;

  request.set_name(name_);

  grpc::Status status = stub->RegisterUser(&context, request, &reply);

  if (!status.ok()) {
    cout << "register user failed: " << status.error_message() << endl;
  } else {
    cout << "Registered as " << name_ << endl;
  }
}

auto Profile::get_friends(unique_ptr<asphrdaemon::Daemon::Stub>& stub)
    -> asphr::StatusOr<Friend::FriendMap> {
  set_time();

  grpc::ClientContext context;
  asphrdaemon::GetFriendListRequest request;
  asphrdaemon::GetFriendListResponse reply;

  grpc::Status status = stub->GetFriendList(&context, request, &reply);

  if (!status.ok()) {
    cout << "get friends failed: " << status.error_message() << endl;
    return asphr::Status(absl::StatusCode::kUnknown,
                         "get friends failed: " + status.error_message());
  }

  Friend::FriendMap friends_map;

  for (const auto& friend_info : reply.friend_infos()) {
    Friend friend_struct(friend_info.name());
    friends_map.insert(std::make_pair(friend_info.name(), friend_struct));
  }

  return friends_map;
}

void Beta_Profile::add(unique_ptr<asphrdaemon::Daemon::Stub>& stub) {
  set_time();

  grpc::ClientContext context;
  asphrdaemon::RegisterUserRequest request;
  asphrdaemon::RegisterUserResponse reply;

  request.set_name(profile_.name());
  request.set_beta_key(beta_key_);

  grpc::Status status = stub->RegisterUser(&context, request, &reply);

  if (!status.ok()) {
    cout << "register user failed: " << status.error_message() << endl;
  } else {
    cout << "Registered as " << profile_.name() << endl;
  }
}