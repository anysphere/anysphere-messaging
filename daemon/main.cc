//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "main.hpp"

#include <grpcpp/grpcpp.h>

#include "asphr/asphr.hpp"
#include "client_lib/client_lib.hpp"
#include "daemon/db/db.hpp"
#include "global.hpp"
#include "rpc/daemon_rpc.hpp"
#include "schema/server.grpc.pb.h"
#include "transmitter/transmitter.hpp"

auto get_db_address() noexcept(false) -> std::filesystem::path {
  return get_daemon_config_dir() / "asphr.db";
}

int main_cc_impl(rust::Vec<rust::String> args) {
  vector<string> args_vec;
  for (auto i = args.begin() + 1; i != args.end(); ++i) {
    args_vec.push_back(std::string(*i));
  }

  auto server_address = string("");
  auto socket_address = string("");
  auto db_address = string("");
  auto tls = true;

  string infname;
  string outfname;

  auto override_default_round_delay = -1;

  // Loop over command-line args
  for (auto i = args_vec.begin(); i != args_vec.end(); ++i) {
    if (*i == "-h" || *i == "--help") {
      cout << "Syntax: daemon -s <server_address> -d <socket_address>"
              " -c <db_address>"
           << endl;
      cout << "  -s <server_address>  Address to listen on (default: "
           << server_address << ")" << endl;
      cout << "  -d <socket_address>  Address of socket (default: "
           << socket_address << ")" << endl;
      cout << "  -c <db_address>  Absolute path of db file (default: "
           << db_address << ")" << endl;
      cout << "  -r <round_delay>  Round delay in seconds (default: 60 seconds)"
           << endl;
      cout << "  --no-tls  Don't use TLS (default: use tls)" << endl;
      return 0;
    } else if (*i == "-s") {
      server_address = *++i;
    } else if (*i == "-d") {
      socket_address = *++i;
    } else if (*i == "-c") {
      db_address = *++i;
    } else if (*i == "-r") {
      override_default_round_delay = std::stoi(*++i);
    } else if (*i == "--no-tls") {
      tls = false;
    } else {
      ASPHR_LOG_ERR("Unknown argument.", argument, *i);
      return 1;
    }
  }

  if (socket_address.empty()) {
    socket_address = get_socket_path().string();
  }
  if (db_address.empty()) {
    db_address = get_db_address().string();
  }

  ASPHR_LOG_INFO("Parsed args.", db_address, db_address, socket_address,
                 socket_address)

  // Promise: G is NEVER destructed. That is, we promise that the main
  // thread will never die before any other threads die.
  auto G = Global(db_address);

  if (server_address.empty()) {
    server_address = std::string(G.db->get_server_address());
  }

  ASPHR_LOG_INFO("Querying server.", server_address, server_address)

  auto channel_creds = grpc::SslCredentials(
      grpc::SslCredentialsOptions{.pem_root_certs = AMAZON_ROOT_CERTS});

  if (!tls) {
    channel_creds = grpc::InsecureChannelCredentials();
  }
  shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(server_address, channel_creds);
  shared_ptr<asphrserver::Server::Stub> stub =
      asphrserver::Server::NewStub(channel);

  Transmitter transmitter(G, stub);

  // set up the daemon rpc server
  auto daemon = DaemonRpc(G, stub);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(socket_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&daemon);

  // start the daemon rpc server
  auto daemon_server = unique_ptr<grpc::Server>(builder.BuildAndStart());

  auto initial_latency = G.db->get_latency();

  while (true) {
    try {
      auto latency = G.db->get_latency();
      if (latency == initial_latency && override_default_round_delay != -1) {
        latency = override_default_round_delay;
      }
      auto killed = G.wait_until_killed_or_seconds(latency);
      if (killed) {
        daemon_server->Shutdown();
        ASPHR_LOG_INFO("Daemon killed.");
        break;
      }

      // do a round
      ASPHR_LOG_INFO("Client round.");

      // receive and then send! it is important! 2x speedup
      transmitter.retrieve();
      transmitter.send();
    } catch (const rust::Error& e) {
      ASPHR_LOG_ERR("Error in database.", error_msg, e.what());
      ASPHR_LOG_INFO("Retrying in 30 seconds...");
      G.wait_until_killed_or_seconds(30);
    }
  }

  return 0;
}

int main_cc(rust::Vec<rust::String> args) {
  try {
    return main_cc_impl(args);
  } catch (const std::exception& e) {
    ASPHR_LOG_ERR("Main failing fatally :(, probably with a database error.",
                  exception, e.what());
    throw;
  }
}
