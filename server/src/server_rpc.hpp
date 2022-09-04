//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#pragma once

#include <grpcpp/grpcpp.h>

#include <ctime>
#include <iostream>
#include <memory>
#include <string>

#include "account_manager.hpp"
#include "asphr/asphr.hpp"
#include "async_invitation_database.hpp"
#include "schema/server.grpc.pb.h"

template <typename PIR, typename AccountManager>
class ServerRpc final : public asphrserver::Server::Service {
  using pir_query_t = typename PIR::pir_query_t;
  using pir_answer_t = typename PIR::pir_answer_t;
  // TODO(sualeh): add a thread safety argument (because the methods may be
  // called from different threads)
  // TODO(sualeh): add representation invariant
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

  grpc::Status AddAsyncInvitation(
      grpc::ServerContext* context,
      const asphrserver::AddAsyncInvitationInfo* addAsyncInvitationInfo,
      asphrserver::AddAsyncInvitationResponse* addFriendAsyncResponse) override;

  grpc::Status GetAsyncInvitations(
      grpc::ServerContext* context,
      const asphrserver::GetAsyncInvitationsInfo* getAsyncInvitationsInfo,
      asphrserver::GetAsyncInvitationsResponse* getAsyncInvitationsResponse)
      override;

  // trunk-ignore(clang-tidy/bugprone-easily-swappable-parameters)
  ServerRpc(PIR&& pir_arg, PIR&& pir_acks_arg,
            AccountManager&& account_manager_arg)
      : pir(std::move(pir_arg)),
        pir_acks(std::move(pir_acks_arg)),
        account_manager(std::move(account_manager_arg)) {
    auto pir_indices = account_manager.get_all_pir_indices();
    if (pir_indices.size() > 0) {
      auto max_pir = *std::max_element(pir_indices.begin(), pir_indices.end());
      pir.allocate_to_max(max_pir);
      pir_acks.allocate_to_max(max_pir);
    }
  }

  auto get_seal_slot_count() const { return pir.get_seal_slot_count(); }

  auto account_manager_FOR_TESTING_ONLY() { return account_manager; }

 private:
  PIR pir;  // stores actual messages for every user
  // TODO(arvid): have a different size for the acks PIR, because the
  // requirements are slightly different!
  PIR pir_acks;  // stores ACKs for every user
  AccountManager account_manager;
  AsyncInvitationDatabase async_invitation_database;
};

#include "server_rpc.cc"