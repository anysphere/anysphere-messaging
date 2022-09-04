//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "asphr/asphr.hpp"
#include "daemon/rpc/daemon_rpc.hpp"
#include "daemon/transmitter/transmitter.hpp"
#include "google/protobuf/util/time_util.h"
#include "server/pir/fast_pir/fastpir.hpp"
#include "server/src/server_rpc.hpp"

inline auto gen_server_rpc() {
  FastPIR pir;
  FastPIR pir_acks;
  AccountManagerInMemory account_manager;
  return ServerRpc(std::move(pir), std::move(pir_acks),
                   std::move(account_manager));
}