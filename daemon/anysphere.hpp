#pragma once
#include <grpcpp/grpcpp.h>

#include "asphr/asphr.hpp"
#include "client/client_lib/client_lib.hpp"
#include "crypto.hpp"
#include "daemon_rpc.hpp"
#include "schema/server.grpc.pb.h"

auto get_config_file_address() noexcept(false) -> std::filesystem::path {
  return get_daemon_config_dir() / "config.json";
}

// TODO(arvid): update server address to use a domain name so that this can be
// constant. maybe like server.anysphere.co or anything.
inline const string SERVER_ADDRESS = "0.0.0.0:50051";
