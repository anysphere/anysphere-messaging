//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "inbox.hpp"

void Inbox::add(const string& msg, const string& to, const string& from,
                absl::Time time) {
  messages_.emplace(time, Message{msg, to, from});
}
void Inbox::update(vector<pair<absl::Time, Message>>& new_messages) {
  for (auto& [time, message] : new_messages) {
    messages_.emplace(time, message);
  }
}

void Inbox::update(unique_ptr<asphrdaemon::Daemon::Stub>& stub,
                   const string& me) {
  vector<pair<absl::Time, Message>> new_messages;

  grpc::ClientContext context;
  asphrdaemon::GetMessagesRequest request;
  request.set_filter(asphrdaemon::GetMessagesRequest::ALL);
  asphrdaemon::GetMessagesResponse response;

  grpc::Status status = stub->GetMessages(&context, request, &response);

  if (!status.ok()) {
    cout << "get all messages failed: " << status.error_message() << endl;
  } else {
    for (const auto& message : response.messages()) {
      auto time_proto = message.delivered_at();
      absl::Time time = absl::FromUnixSeconds(time_proto.seconds());

      new_messages.emplace_back(
          time, Message{message.m().message(), me, message.m().unique_name()});
    }
  }

  update(new_messages);
}
