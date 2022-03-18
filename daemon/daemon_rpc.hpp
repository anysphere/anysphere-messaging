//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "config.hpp"
#include "crypto.hpp"
#include "msgstore.hpp"
#include "schema/daemon.grpc.pb.h"
#include "schema/server.grpc.pb.h"

class DaemonRpc final : public asphrdaemon::Daemon::Service {
 public:
  DaemonRpc(const Crypto crypto, shared_ptr<Config> config,
            shared_ptr<asphrserver::Server::Stub> stub,
            shared_ptr<Msgstore> msgstore)
      : crypto(crypto), config(config), stub(stub), msgstore(msgstore) {}

  grpc::Status RegisterUser(
      grpc::ServerContext* context,
      const asphrdaemon::RegisterUserRequest* registerUserRequest,
      asphrdaemon::RegisterUserResponse* registerUserResponse) override;

  grpc::Status GetFriendList(
      grpc::ServerContext* context,
      const asphrdaemon::GetFriendListRequest* getFriendListRequest,
      asphrdaemon::GetFriendListResponse* getFriendListResponse) override;

  grpc::Status GenerateFriendKey(
      grpc::ServerContext* context,
      const asphrdaemon::GenerateFriendKeyRequest* generateFriendKeyRequest,
      asphrdaemon::GenerateFriendKeyResponse* generateFriendKeyResponse)
      override;

  grpc::Status AddFriend(
      grpc::ServerContext* context,
      const asphrdaemon::AddFriendRequest* addFriendRequest,
      asphrdaemon::AddFriendResponse* addFriendResponse) override;

  grpc::Status RemoveFriend(
      grpc::ServerContext* context,
      const asphrdaemon::RemoveFriendRequest* removeFriendRequest,
      asphrdaemon::RemoveFriendResponse* removeFriendResponse) override;

  grpc::Status SendMessage(
      grpc::ServerContext* context,
      const asphrdaemon::SendMessageRequest* sendMessageRequest,
      asphrdaemon::SendMessageResponse* sendMessageResponse) override;

  grpc::Status GetAllMessages(
      grpc::ServerContext* context,
      const asphrdaemon::GetAllMessagesRequest* getAllMessagesRequest,
      asphrdaemon::GetAllMessagesResponse* getAllMessagesResponse) override;

  grpc::Status GetAllMessagesStreamed(
      grpc::ServerContext* context,
      const asphrdaemon::GetAllMessagesRequest* request,
      grpc::ServerWriter<asphrdaemon::GetAllMessagesResponse>* writer) override;

  grpc::Status GetNewMessages(
      grpc::ServerContext* context,
      const asphrdaemon::GetNewMessagesRequest* getNewMessagesRequest,
      asphrdaemon::GetNewMessagesResponse* getNewMessagesResponse) override;

  grpc::Status GetOutboxMessages(
      grpc::ServerContext* context,
      const asphrdaemon::GetOutboxMessagesRequest* getOutboxMessagesRequest,
      asphrdaemon::GetOutboxMessagesResponse* getOutboxMessagesResponse)
      override;

  grpc::Status GetSentMessages(
      grpc::ServerContext* context,
      const asphrdaemon::GetSentMessagesRequest* getSentMessagesRequest,
      asphrdaemon::GetSentMessagesResponse* getSentMessagesResponse) override;

  grpc::Status MessageSeen(
      grpc::ServerContext* context,
      const asphrdaemon::MessageSeenRequest* messageSeenRequest,
      asphrdaemon::MessageSeenResponse* messageSeenResponse) override;

  grpc::Status GetStatus(
      grpc::ServerContext* context,
      const asphrdaemon::GetStatusRequest* getStatusRequest,
      asphrdaemon::GetStatusResponse* getStatusResponse) override;

  grpc::Status GetLatency(
      grpc::ServerContext* context,
      const asphrdaemon::GetLatencyRequest* getLatencyRequest,
      asphrdaemon::GetLatencyResponse* getLatencyResponse) override;

  grpc::Status ChangeLatency(
      grpc::ServerContext* context,
      const asphrdaemon::ChangeLatencyRequest* changeLatencyRequest,
      asphrdaemon::ChangeLatencyResponse* changeLatencyResponse) override;

  grpc::Status Kill(grpc::ServerContext* context,
                    const asphrdaemon::KillRequest* killRequest,
                    asphrdaemon::KillResponse* killResponse) override;

 private:
  const Crypto crypto;
  shared_ptr<Config> config;
  shared_ptr<asphrserver::Server::Stub> stub;
  shared_ptr<Msgstore> msgstore;
};