//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "global.hpp"

auto Global::transmitter_ping() -> void {
  {
    const std::lock_guard<std::mutex> l(transmitter_ping_mtx);
    transmitter_ping_counter++;
  }

  transmitter_ping_cv.notify_all();
}

auto Global::wait_for_transmitter_ping_with_timeout(int seconds) -> bool {
  std::unique_lock<std::mutex> l(transmitter_ping_mtx);

  const auto counter_start = transmitter_ping_counter;

  transmitter_ping_cv.wait_for(
      l, std::chrono::seconds(seconds), [this, counter_start] {
        return transmitter_ping_counter != counter_start;
      });

  auto did_get_pinged = transmitter_ping_counter != counter_start;

  return did_get_pinged;
}

auto Global::grpc_ping() -> void {
  {
    const std::lock_guard<std::mutex> l(grpc_ping_mtx);
    grpc_ping_counter++;
  }

  grpc_ping_cv.notify_all();
}

auto Global::wait_for_grpc_ping_with_timeout(int seconds) -> bool {
  std::unique_lock<std::mutex> l(grpc_ping_mtx);

  const auto counter_start = grpc_ping_counter;

  grpc_ping_cv.wait_for(
      l, std::chrono::seconds(seconds),
      [this, counter_start] { return grpc_ping_counter != counter_start; });

  auto did_get_pinged = grpc_ping_counter != counter_start;

  return did_get_pinged;
}