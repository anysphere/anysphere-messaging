//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "as_cli.hpp"
#include "asphr/asphr.hpp"

class Inbox {
 public:
  void add(const string& msg, const string& to, const string& from,
           absl::Time time);
  void update(vector<pair<absl::Time, Message>>& new_messages);
  void update(unique_ptr<asphrdaemon::Daemon::Stub>& stub, const string& me);

  // Note: run update always to get the updated messages.
  auto get_messages() const { return messages_; }

 private:
  std::map<absl::Time, Message> messages_;
};