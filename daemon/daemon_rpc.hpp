#pragma once

#include "config.hpp"
#include "crypto.hpp"
#include "schema/daemon.grpc.pb.h"
#include "schema/server.grpc.pb.h"

class DaemonRpc final : public asphrdaemon::Daemon::Service {
 public:
  DaemonRpc(const Crypto& crypto, Config& config,
            unique_ptr<asphrserver::Server::Stub>& stub)
      : crypto(crypto), config(config), stub(stub) {}

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

  grpc::Status GetNewMessages(
      grpc::ServerContext* context,
      const asphrdaemon::GetNewMessagesRequest* getNewMessagesRequest,
      asphrdaemon::GetNewMessagesResponse* getNewMessagesResponse) override;

 private:
  const Crypto& crypto;
  Config& config;
  unique_ptr<asphrserver::Server::Stub>& stub;
};