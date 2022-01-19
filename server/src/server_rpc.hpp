#pragma once

#include <grpcpp/grpcpp.h>

#include <ctime>
#include <iostream>
#include <memory>
#include <string>

#include "account_manager.hpp"
#include "asphr/asphr.hpp"
#include "schema/server.grpc.pb.h"

template <typename PIR, typename AccountManager>
class ServerRpc final : public asphrserver::Server::Service {
  using pir_query_t = typename PIR::pir_query_t;
  using pir_answer_t = typename PIR::pir_answer_t;
  // TODO: add a thread safety argument (because the methods may be called from
  // different threads)
  // TODO: add representation invariant
 public:
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

  ServerRpc(PIR&& pir, PIR&& pir_acks, AccountManager&& account_manager)
      : pir(std::move(pir)),
        pir_acks(std::move(pir_acks)),
        account_manager(std::move(account_manager)) {}

  auto get_seal_slot_count() const { return pir.get_seal_slot_count(); }

 private:
  PIR pir;       // stores actual messages for every user
  PIR pir_acks;  // stores ACKs for every user
  AccountManager account_manager;
};

#include "server_rpc.cc"