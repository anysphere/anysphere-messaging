#include "asphr/asphr.hpp"
#include "daemon/daemon_rpc.hpp"
#include "daemon/transmitter.hpp"
#include "google/protobuf/util/time_util.h"
#include "server/pir/fast_pir/fastpir.hpp"
#include "server/src/server_rpc.hpp"

auto gen_crypto() -> Crypto {
  Crypto crypto;
  return crypto;
}

auto gen_config(string tmp_dir, string tmp_file) -> shared_ptr<Config> {
  json config_json = {{"has_registered", false},
                      {"friends", asphr::json::array()},
                      {"data_dir", tmp_dir},
                      {"server_address", "unused"}};
  auto config = make_shared<Config>(config_json, tmp_file);
  return config;
}

auto gen_msgstore(shared_ptr<Config> config) -> shared_ptr<Msgstore> {
  auto msgstore = make_shared<Msgstore>(config->msgstore_address(), config);
  return msgstore;
}

auto gen_server_rpc() {
  FastPIR pir;
  FastPIR pir_acks;
  AccountManagerInMemory account_manager;
  return ServerRpc(std::move(pir), std::move(pir_acks),
                   std::move(account_manager));
}