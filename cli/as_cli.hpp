#pragma once

#include <grpcpp/grpcpp.h>

#include "as_scheduler.hpp"
#include "asphr/asphr.hpp"
#include "cli/cli.h"
#include "cli/clifilesession.h"
#include "cli/clilocalsession.h"
#include "client/client_lib/client_lib.hpp"
#include "schema/daemon.grpc.pb.h"

using MainScheduler = as_cli::AnysphereScheduler;

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
 public:
  Friend() = default;
  Friend(const string& name) : name_(name) {}
  Name name_;
  absl::Time time_;

  auto generate_key(unique_ptr<asphrdaemon::Daemon::Stub>& stub)
      -> asphr::StatusOr<string>;
  void add(unique_ptr<asphrdaemon::Daemon::Stub>& stub, const string& key);

  void clear() { name_.clear(); }
  bool complete() const;

 private:
  bool name_is_empty() const { return name_.empty(); }
};

struct Profile {
 public:
  Profile(const string& name, const string& public_key,
          const string& private_key)
      : name_(name), public_key_(public_key), private_key_(private_key) {}
  Name name_;
  PublicKey public_key_;
  PrivateKey private_key_;
  absl::Time time_;

  void add(unique_ptr<asphrdaemon::Daemon::Stub>& stub);

  void set_time() { time_ = absl::Now(); }
  void clear() {
    name_.clear();
    public_key_.clear();
    private_key_.clear();
  }
  bool complete() const {
    return !name_is_empty() && !public_key_is_empty() &&
           !private_key_is_empty();
  }

 private:
  bool name_is_empty() const { return name_.empty(); }
  bool public_key_is_empty() const { return public_key_.empty(); }
  bool private_key_is_empty() const { return private_key_.empty(); }
};
