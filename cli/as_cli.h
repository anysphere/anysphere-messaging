#pragma once

#include "asphr/asphr.h"
#include "cli/cli.h"
#include "cli/clifilesession.h"
#include "cli/clilocalsession.h"
#include "cli/loopscheduler.h"
#include "client/client_lib/client_lib.h"

using MainScheduler = cli::LoopScheduler;

using std::cin;
using std::make_unique;

// The message struct to store and send messages.
struct Message {
 public:
  Message() = default;
  Message(const string& msg, const string& friend_name)
      : msg_(msg), friend_(friend_name) {}
  Msg msg_;
  Name friend_;
  absl::Time time_;
  constexpr static auto file_address_ = UI_FILE;

  bool complete() const { return !message_is_empty() && !friend_is_empty(); }
  void set_time() { time_ = absl::Now(); }

  void send() {
    set_time();
    write_msg_to_file("MESSAGE", file_address_, msg_, friend_, time_);
    clear();
  }

  void clear() {
    msg_.clear();
    friend_.clear();
  }

 private:
  bool message_is_empty() const { return msg_.empty(); }
  bool friend_is_empty() const { return friend_.empty(); }
};

// The friend struct to store the name and public key of a friend.
// When a friend is complete we can add it to the config
struct Friend {
 public:
  Friend() = default;
  Friend(const string& name, const int& write_index, const int& read_index,
         const string& shared_key)
      : name_(name),
        write_index_(write_index),
        read_index_(read_index),
        shared_key_(shared_key) {}
  Name name_;
  absl::Time time_;
  Name name;
  int write_index_ = -1;
  int read_index_ = -1;
  string shared_key_;

  constexpr static auto file_address_ = UI_URGENT_FILE;

  bool complete() const {
    return !name_is_empty() && !shared_key_is_empty() && write_index_ != -1 &&
           read_index_ != -1;
  }
  void set_time() { time_ = absl::Now(); }

  void add() {
    set_time();
    write_friend_to_file(file_address_, name_, write_index_, read_index_,
                         shared_key_, time_);
    clear();
  }

  void clear() { name_.clear(); }

 private:
  bool name_is_empty() const { return name_.empty(); }
  bool write_index_is_empty() const { return write_index_ == -1; }
  bool read_index_is_empty() const { return read_index_ == -1; }
  bool shared_key_is_empty() const { return shared_key_.empty(); }
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

  bool complete() const {
    return !name_is_empty() && !public_key_is_empty() &&
           !private_key_is_empty();
  }
  void set_time() { time_ = absl::Now(); }

  void add() {
    set_time();
    register_profile_to_file(name_, public_key_, private_key_, time_);
    clear();
  }

  void clear() {
    name_.clear();
    public_key_.clear();
    private_key_.clear();
  }

 private:
  bool name_is_empty() const { return name_.empty(); }
  bool public_key_is_empty() const { return public_key_.empty(); }
  bool private_key_is_empty() const { return private_key_.empty(); }
};
