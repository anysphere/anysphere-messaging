#pragma once

#include "asphr/asphr.hpp"
#include "config.hpp"
#include "crypto.hpp"
#include "inbox.hpp"
#include "outbox.hpp"

class Transmitter {
 public:
  Transmitter(const Crypto& crypto, Config& config,
              unique_ptr<asphrserver::Server::Stub>& stub);

  auto retrieve_messages() -> void;

  auto send_messages() -> void;

 private:
  const Crypto& crypto;
  Config& config;
  unique_ptr<asphrserver::Server::Stub>& stub;

  Inbox inbox;
  Outbox outbox;

  string just_sent_friend;
  string previous_success_receive_friend;
  Time last_ui_timestamp;

  auto check_rep() const noexcept -> void;
};