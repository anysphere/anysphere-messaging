#pragma once

#include <grpcpp/grpcpp.h>

#include <ctime>
#include <iostream>
#include <memory>
#include <string>

#ifdef BAZEL_BUILD
#include "schema/server.grpc.pb.h"
#else
#include "schema/server.grpc.pb.h"
#endif

#include "account_manager.h"
#include "asphr/asphr.h"

template <typename PIR, typename AccountManager>
class ServerRpc final : public asphrserver::Server::Service {
  using pir_query_t = typename PIR::pir_query_t;
  using pir_answer_t = typename PIR::pir_answer_t;
  // TODO: add a thread safety argument (because the methods may be called from
  // different threads)
  // TODO: add representation invariant
  grpc::Status Register(
      grpc::ServerContext* context,
      const asphrserver::RegisterInfo* registerInfo,
      asphrserver::RegisterResponse* registerResponse) override;

  grpc::Status SendMessage(
      grpc::ServerContext* context,
      const asphrserver::SendMessageInfo* sendMessageInfo,
      asphrserver::SendMessageResponse* sendMessageResponse) override;

  grpc::Status ReceiveMessage(
      grpc::ServerContext* context,
      const asphrserver::ReceiveMessageInfo* receiveMessageInfo,
      asphrserver::ReceiveMessageResponse* receiveMessageResponse) override;

 public:
  ServerRpc(PIR& pir, AccountManager& account_manager)
      : pir(pir), account_manager(account_manager) {}

 private:
  PIR& pir;
  AccountManager& account_manager;
};

#include "server_rpc.cc"