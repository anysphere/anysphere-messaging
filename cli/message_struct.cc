//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "as_cli.hpp"

void Message::send(unique_ptr<asphrdaemon::Daemon::Stub>& stub) {
  set_time();

  grpc::ClientContext context;
  asphrdaemon::SendMessageRequest request;
  asphrdaemon::SendMessageResponse reply;

  request.set_message(msg_);
  request.set_unique_name(to_);

  grpc::Status status = stub->SendMessage(&context, request, &reply);

  if (!status.ok()) {
    cout << "send message failed: " << status.error_message() << endl;
  } else {
    cout << "message sent" << endl;
  }

  clear();
}
