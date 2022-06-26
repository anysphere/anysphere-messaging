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

  auto transmitter_ping() -> void;
  // returns true if the transmitter did ping, false otherwise
  auto wait_for_transmitter_ping_with_timeout(int seconds) -> bool;

  auto grpc_ping() -> void;
  // returns true if the grpc did ping, false otherwise
  auto wait_for_grpc_ping_with_timeout(int seconds) -> bool;

  const string db_address;
  const rust::Box<db::DB> db;

  // the following are for notifying someone when an incoming message has
  // been added
  std::mutex message_notification_cv_mutex;
  std::condition_variable message_notification_cv;

 private:
  std::mutex transmitter_ping_mtx;
  std::condition_variable transmitter_ping_cv;
  int transmitter_ping_counter = 0;

  std::mutex grpc_ping_mtx;
  std::condition_variable grpc_ping_cv;
  int grpc_ping_counter = 0;
};