//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once
#include <grpcpp/grpcpp.h>

#include "asphr/asphr.hpp"
#include "client_lib/client_lib.hpp"
#include "crypto.hpp"
#include "daemon_rpc.hpp"
#include "schema/server.grpc.pb.h"

auto get_config_file_address() noexcept(false) -> std::filesystem::path {
  return get_daemon_config_dir() / "config.json";
}