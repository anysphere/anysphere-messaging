#pragma once

#include "as_cli.hpp"
#include "asphr/asphr.hpp"

class Inbox {
 public:
  Inbox() = default;

  void add(const string& msg, const string& to, const string& from,
           absl::Time time);
  void update(vector<pair<absl::Time, Message>>& new_messages);
  void update(unique_ptr<asphrdaemon::Daemon::Stub>& stub, const string me);

 private:
  std::unordered_map<absl::Time, Message> messages_;
};