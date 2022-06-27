//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "daemon/crypto/crypto.hpp"
#include "daemon/global.hpp"
#include "pir/fast_pir/fast_pir_client.hpp"
#include "schema/daemon.grpc.pb.h"
#include "schema/server.grpc.pb.h"

class DaemonRpc final : public asphrdaemon::Daemon::Service {
 public:
  DaemonRpc(Global& G, shared_ptr<asphrserver::Server::Stub> stub)
      : G(G), stub(stub) {}

  grpc::Status RegisterUser(
      grpc::ServerContext* context,
      const asphrdaemon::RegisterUserRequest* registerUserRequest,
      asphrdaemon::RegisterUserResponse* registerUserResponse) override;

  grpc::Status GetFriendList(
      grpc::ServerContext* context,
      const asphrdaemon::GetFriendListRequest* getFriendListRequest,
      asphrdaemon::GetFriendListResponse* getFriendListResponse) override;

  grpc::Status GetPublicID(
      grpc::ServerContext* context,
      const asphrdaemon::GetPublicIDRequest* getPublicIDRequest,
      asphrdaemon::GetPublicIDResponse* getPublicIDResponse) override;

  grpc::Status AddSyncFriend(
      grpc::ServerContext* context,
      const asphrdaemon::AddSyncFriendRequest* addSyncFriendRequest,
      asphrdaemon::AddSyncFriendResponse* addSyncFriendResponse) override;

  grpc::Status SendAsyncFriendRequest(
      grpc::ServerContext* context,
      const asphrdaemon::SendAsyncFriendRequestRequest*
          sendAsyncFriendRequestRequest,
      asphrdaemon::SendAsyncFriendRequestResponse*
          sendAsyncFriendRequestResponse) override;

  grpc::Status GetOutgoingAsyncFriendRequests(
      grpc::ServerContext* context,
      const asphrdaemon::GetOutgoingAsyncFriendRequestsRequest*
          getOutgoingAsyncFriendRequestsRequest,
      asphrdaemon::GetOutgoingAsyncFriendRequestsResponse*
          getOutgoingAsyncFriendRequestsResponse) override;

  grpc::Status GetIncomingAsyncFriendRequests(
      grpc::ServerContext* context,
      const asphrdaemon::GetIncomingAsyncFriendRequestsRequest*
          getIncomingAsyncFriendRequestsRequest,
      asphrdaemon::GetIncomingAsyncFriendRequestsResponse*
          getIncomingAsyncFriendRequestsResponse) override;

  grpc::Status DecideAsyncFriendRequest(
      grpc::ServerContext* context,
      const asphrdaemon::DecideAsyncFriendRequestRequest*
          decideAsyncFriendRequestRequest,
      asphrdaemon::DecideAsyncFriendRequestResponse*
          decideAsyncFriendRequestResponse) override;

  grpc::Status RemoveFriend(
      grpc::ServerContext* context,
      const asphrdaemon::RemoveFriendRequest* removeFriendRequest,
      asphrdaemon::RemoveFriendResponse* removeFriendResponse) override;

  grpc::Status SendMessage(
      grpc::ServerContext* context,
      const asphrdaemon::SendMessageRequest* sendMessageRequest,
      asphrdaemon::SendMessageResponse* sendMessageResponse) override;

  grpc::Status GetMessages(
      grpc::ServerContext* context,
      const asphrdaemon::GetMessagesRequest* getMessagesRequest,
      asphrdaemon::GetMessagesResponse* getMessagesResponse) override;

  grpc::Status GetMessagesStreamed(
      grpc::ServerContext* context,
      const asphrdaemon::GetMessagesRequest* request,
      grpc::ServerWriter<asphrdaemon::GetMessagesResponse>* writer) override;

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
  Global& G;
  shared_ptr<asphrserver::Server::Stub> stub;

  // The RPC speaks in the FriendInfo Struct
  // The DB speaks in the Friend and Address Structs
  // we provide a way to translate between them
  auto convertStructRPCtoDB(asphrdaemon::FriendInfo& friend_info,
                            string message, int progress, string read_key,
                            string write_key)
      -> asphr::StatusOr<std::pair<db::FriendFragment, db::AddAddress>>;

  auto convertStructDBtoRPC(const db::Friend& db_friend,
                            const db::Address& db_address)
      -> asphr::StatusOr<std::pair<asphrdaemon::FriendInfo, string>>;
};