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
  } else if (!reply.success()) {
    cout << "register user failed from the daemon." << endl;
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
  } else if (!reply.success()) {
    cout << "get friends failed from the daemon." << endl;
    return asphr::Status(absl::StatusCode::kUnknown,
                         "get friends failed from the daemon.");
  }

  Friend::FriendMap friends_map;

  for (const auto& friend_name_ : reply.friend_list()) {
    Friend friend_struct(friend_name_);
    friends_map.insert(std::make_pair(friend_name_, friend_struct));
  }

  return friends_map;
}
