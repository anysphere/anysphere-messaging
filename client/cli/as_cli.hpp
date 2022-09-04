//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#pragma once

#include <grpcpp/grpcpp.h>

#include "asphr/asphr/asphr.hpp"
#include "asphr/schema/daemon.grpc.pb.h"
#include "client/client_lib/client_lib.hpp"
#include "parse_command_line.hpp"

using std::cin;
using std::make_unique;

// The message struct to store and send messages.
struct Message {
 public:
  Message() = default;
  Message(const string& msg, const string& to, const string& from)
      : msg_(msg), to_(to), from_(from) {}

  Msg msg_;
  Name to_;
  Name from_;

  absl::Time time_;

  bool complete() const {
    return !message_is_empty() && !to_is_empty() && !from_is_empty();
  }
  void set_time() { time_ = absl::Now(); }
  void send(unique_ptr<asphrdaemon::Daemon::Stub>& stub);

  void clear() {
    msg_.clear();
    to_.clear();
    from_.clear();
  }

 private:
  bool message_is_empty() const { return msg_.empty(); }
  bool to_is_empty() const { return to_.empty(); }
  bool from_is_empty() const { return from_.empty(); }
};

// The friend struct to store the name and public key of a friend.
// When a friend is complete we can add it to the config
struct Friend {
  using FriendMap = std::map<string, Friend>;

 public:
  Friend() = default;
  Friend(const string& name) : name_(name) {}
  Name name_;
  absl::Time time_;

  auto generate_key(unique_ptr<asphrdaemon::Daemon::Stub>& stub)
      -> asphr::StatusOr<string>;
  auto add(unique_ptr<asphrdaemon::Daemon::Stub>& stub, const string& key)
      -> asphr::Status;

  void clear() { name_.clear(); }
  bool complete() const;

 private:
  bool name_is_empty() const { return name_.empty(); }
};

struct Profile {
 public:
  Profile() = default;
  Profile(const string& name) : name_(name) {}
  Name name_;
  absl::Time time_;

  void add(unique_ptr<asphrdaemon::Daemon::Stub>& stub);
  auto get_friends(unique_ptr<asphrdaemon::Daemon::Stub>& stub)
      -> asphr::StatusOr<Friend::FriendMap>;

  void set_time() { time_ = absl::Now(); }
  void set_name(const string& name) { name_ = name; }
  auto name() -> string { return name_; }

  void clear() { name_.clear(); }
  bool complete() const { return !name_is_empty(); }

 private:
  bool name_is_empty() const { return name_.empty(); }
};

struct Beta_Profile {
 public:
  Beta_Profile() = default;
  Beta_Profile(const string& name, const string& beta_key)
      : profile_(Profile(name)), beta_key_(beta_key) {}

  Profile profile_;
  string beta_key_;

  void add(unique_ptr<asphrdaemon::Daemon::Stub>& stub);
  auto get_friends(unique_ptr<asphrdaemon::Daemon::Stub>& stub)
      -> asphr::StatusOr<Friend::FriendMap> {
    return profile_.get_friends(stub);
  };

  void set_time() { profile_.set_time(); }
  void set_name(const string& name) { profile_.set_name(name); }
  void set_beta_key(const string& beta_key) { beta_key_ = beta_key; }
  auto name() -> string { return profile_.name(); }

  void clear() { profile_.clear(); }
  bool complete() const { return profile_.complete() && !beta_key_.empty(); }
};
