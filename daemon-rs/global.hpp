#pragma once

#include "constants.hpp"
#include "crypto/crypto.hpp"
#include "db/db.hpp"

// Global is a thread safe singleton that all threads should have
// access to. It exists to provide notifications as well as set up the database.
//
// Might throw irrecoverably during initialization.
class Global {
 public:
  Global(const string& db_address)
      : db_address(db_address), db(db::init(db_address)) {
    crypto::init();
  }

  ~Global() = default;
  Global(const Global& other) = delete;
  Global(Global&& other) noexcept = delete;
  Global& operator=(const Global& other) = delete;
  Global& operator=(Global&& other) noexcept = delete;

  auto alive() const noexcept -> bool { return true; }

  auto kill() -> void;
  // returns true iff killed
  auto wait_until_killed_or_seconds(int seconds) -> bool;

  const string db_address;
  const rust::Box<db::DB> db;

  // the following are for notifying someone when an incoming message has
  // been added
  mutable std::mutex message_notification_cv_mutex;
  mutable std::condition_variable message_notification_cv;

 private:
  mutable std::mutex kill_mtx;
  mutable std::condition_variable kill_cv;
  bool kill_ = false;
};