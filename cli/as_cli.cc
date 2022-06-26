//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "as_cli.hpp"

#include "inbox.hpp"

using asphrdaemon::Daemon;

int main(int argc, char** argv) {
  // setup cli

  Message message_to_send;
  // Friend friend_to_add;
  static Beta_Profile kProfile_;
  static Inbox kInbox_;

  static Friend::FriendMap kFriends_map_;

  auto* const binary_name = argv[0];

  auto help = StrCat(
      "Usage: \n",
      // register
      binary_name, " register: {name}\n",
      // init-friend
      binary_name, " init-friend: {name}\n",
      // TODO(unknown): add-friend must have an init-friend before
      binary_name, " add-friend: {name} {key}\n",
      // TODO(unknown): explain the options better.
      binary_name, " (s | m | send | msg | message) {name}\n", binary_name,
      " (get-friends | friends)\n",
      // TODO(unknown): explain that -a is optional.
      binary_name, " (inbox | i) [-a | -all]\n", binary_name,
      " socket {address}\n", binary_name, " kill\n", binary_name, " status\n");

  CommandLine cmd_line{argc, argv, help};

  auto command_status = cmd_line.getArgument(1);

  if (!command_status.ok()) {
    cout << "Anysphere CLI." << endl << endl;
    cout << help << endl;
    return 0;
  }

  auto command = command_status.value();

  // set up the unix socket
  auto socket_address = StrCat("unix://", get_socket_path().string());

  if (cmd_line.getOption("socket")) {
    socket_address = cmd_line.getOptionValue("socket", socket_address).value();
  }

  // connect to the anysphere daemon
  auto channel =
      grpc::CreateChannel(socket_address, grpc::InsecureChannelCredentials());
  auto stub = Daemon::NewStub(channel);

  // parse the commands
  if (command == "register") {
    auto name_status = cmd_line.getArgument(2);
    auto beta_key_status = cmd_line.getArgument(3);

    if (!name_status.ok()) {
      cout << name_status.status() << endl;
      cout << StrCat("Usage: ", binary_name, " register {name} {beta_key}")
           << endl;
      cout << StrCat("Usage: ", binary_name, " register Elon MuskLovesTesla")
           << endl;
      cout << help << endl;
      return 0;
    }

    if (!beta_key_status.ok()) {
      cout << beta_key_status.status() << endl;
      cout << StrCat("Usage: ", binary_name, " register {name} {beta_key}")
           << endl;
      cout << StrCat("Usage: ", binary_name, " register Elon MuskLovesTesla")
           << endl;
      cout << help << endl;
      return 0;
    }

    auto name = name_status.value();
    auto beta_key = beta_key_status.value();

    kProfile_.set_name(name);
    kProfile_.set_beta_key(beta_key);

    kProfile_.add(stub);
  } else if (command == "init-friend") {
    auto status = cmd_line.getArgument(2);
    if (!status.ok()) {
      cout << status.status() << endl;
      cout << "Usage: " << binary_name << " init-friend {name}" << endl;
      cout << "Example: " << binary_name << " init-friend Elon\n\n" << endl;
      cout << help << endl;
      return 0;
    }
    auto name = status.value();
    Friend friend_to_add(name);
    kFriends_map_[name] = friend_to_add;

    auto friend_status = friend_to_add.generate_key(stub);

    if (!friend_status.ok()) {
      return 0;
    }
    auto key = friend_status.value();
    cout << "Friend " << name << " key generated!" << endl;
    cout << "Please give your friend the following key: " << key << endl;
    cout << "When they give you back their shared key, you can then add them "
            "with the command add-friend {their key}"
         << endl;
  } else if (command == "add-friend") {
    auto status = cmd_line.getArgument(2);
    auto key_status = cmd_line.getArgument(3);
    if (!status.ok() || !key_status.ok()) {
      cout << "Usage: " << binary_name << " add-friend {name} {key}" << endl;
      cout << "Example: " << binary_name << " add-friend Elon 123456789"
           << endl;
      cout << "You can find the key of your friend by asking them to add you "
              "as a friend with the command init-friend {name}\n\n"
           << endl;
      cout << help << endl;
      return 0;
    }

    auto name = status.value();
    auto key = key_status.value();
    Friend friend_to_add(name);

    kFriends_map_[name] = friend_to_add;
    auto friend_status = friend_to_add.add(stub, key);

    if (!friend_status.ok()) {
      cout << friend_status.message() << endl;
      cout << "We couldnt add your friend. Please try again." << endl;
      cout << "You can report this error to us at help@anysphere.co!" << endl;
      return 0;
    }

    cout << "Friend " << name << " added!" << endl;
    cout << "Yipppeeee!" << endl;
    cout << "You can now talk to them with the command 'message' {their name}"
         << endl;
  } else if (command == "s" || command == "m" || command == "send" ||
             command == "msg" || command == "message") {
    auto status = cmd_line.getArgument(2);
    auto message_status = cmd_line.getArgument(3);
    if (!status.ok() || !message_status.ok()) {
      cout << "Usage: " << binary_name
           << " (s | m | send | msg | message) {name} {message}" << endl;
      cout << "Example: " << binary_name
           << " message Elon Hello, how are you?\n\n"
           << endl;
      cout << help << endl;
      return 0;
    }
    auto name = status.value();
    auto msg = cmd_line.getConcatArguments(3).value();

    Message m{msg, name, kProfile_.name()};
    m.send(stub);

  } else if (command == "inbox" || command == "i") {
    cout << "Inbox:" << endl;
    kInbox_.update(stub, kProfile_.name());
    for (auto& [time, message] : kInbox_.get_messages()) {
      cout << absl::FormatTime(time, absl::UTCTimeZone()) << ": "
           << message.msg_ << endl;
    }
  } else if (command == "get-friends" || command == "friends") {
    cout << "Friends:" << endl;

    auto status = kProfile_.get_friends(stub);

    if (!status.ok()) {
      return 0;
    }
    kFriends_map_ = status.value();
    for (auto& [name, friend_] : kFriends_map_) {
      cout << name << endl;
    }
  } else if (command == "status") {
    grpc::ClientContext context;
    asphrdaemon::GetStatusRequest request;
    asphrdaemon::GetStatusResponse response;

    grpc::Status status = stub->GetStatus(&context, request, &response);

    if (!status.ok()) {
      cout << "get status failed: " << status.error_message() << endl;
      return 0;
    }

    cout << "Registered: " << (response.registered() ? "true" : "false")
         << endl;
    cout << "Release commit hash: " << response.release_hash() << endl;
    cout << "Latency: " << response.latency_seconds() << endl;

  } else if (command == "kill") {
    grpc::ClientContext context;
    asphrdaemon::KillRequest request;
    asphrdaemon::KillResponse response;

    grpc::Status status = stub->Kill(&context, request, &response);

    if (!status.ok() && status.error_message() != "Socket closed") {
      cout << "kill failed: " << status.error_message() << endl;
      return 0;
    }
    cout << "Successfully killed the daemon!" << endl;

  } else if (command == "change-latency" || command == "cltcy") {
    auto latency_seconds = cmd_line.getArgument(2);
    if (!latency_seconds.ok()) {
      cout << "Usage: " << binary_name << " (change-latency | cltcy) {latency}"
           << endl;
      cout << "Example: " << binary_name << " cltcy 10 (in seconds)" << endl;
      cout << "Latecy is always in seconds." << endl;
      cout << help << endl;
      return 0;
    }

    auto latency = std::stoi(latency_seconds.value());

    grpc::ClientContext context;
    asphrdaemon::ChangeLatencyRequest request;
    asphrdaemon::ChangeLatencyResponse response;

    // safely turn the latency into an int
    request.set_latency_seconds(latency);

    grpc::Status status = stub->ChangeLatency(&context, request, &response);

    if (!status.ok()) {
      cout << "change latency failed: " << status.error_message() << endl;
      return 0;
    }
    cout << "Successfully changed latency to " << latency << " seconds!"
         << endl;

  } else {
    cout << "Unknown command: " << command << endl;
    cout << help << endl;
    return 0;
  }

  return 0;
}
