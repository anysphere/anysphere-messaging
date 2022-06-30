//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "daemon/crypto/crypto.hpp"
#include "daemon/global.hpp"
#include "daemon/identifier/identifier.hpp"
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

  grpc::Status GetMyPublicID(
      grpc::ServerContext* context,
      const asphrdaemon::GetMyPublicIDRequest* getMyPublicIDRequest,
      asphrdaemon::GetMyPublicIDResponse* getMyPublicIDResponse) override;

  grpc::Status GetFriendList(
      grpc::ServerContext* context,
      const asphrdaemon::GetFriendListRequest* getFriendListRequest,
      asphrdaemon::GetFriendListResponse* getFriendListResponse) override;

  grpc::Status RemoveFriend(
      grpc::ServerContext* context,
      const asphrdaemon::RemoveFriendRequest* removeFriendRequest,
      asphrdaemon::RemoveFriendResponse* removeFriendResponse) override;

  // Invitations (async + sync) section. Sync \similarTo InPerson.
  grpc::Status AddSyncFriend(
      grpc::ServerContext* context,
      const asphrdaemon::AddSyncFriendRequest* addSyncFriendRequest,
      asphrdaemon::AddSyncFriendResponse* addSyncFriendResponse) override;

  grpc::Status AddAsyncFriend(
      grpc::ServerContext* context,
      const asphrdaemon::AddAsyncFriendRequest* addAsyncFriendRequest,
      asphrdaemon::AddAsyncFriendResponse* addAsyncFriendResponse) override;

  grpc::Status GetOutgoingSyncInvitations(
      grpc::ServerContext* context,
      const asphrdaemon::GetOutgoingSyncInvitationsRequest*
          getOutgoingSyncInvitationsRequest,
      asphrdaemon::GetOutgoingSyncInvitationsResponse*
          getOutgoingSyncInvitationsResponse) override;

  grpc::Status GetOutgoingAsyncInvitations(
      grpc::ServerContext* context,
      const asphrdaemon::GetOutgoingAsyncInvitationsRequest*
          getOutgoingAsyncInvitationsRequest,
      asphrdaemon::GetOutgoingAsyncInvitationsResponse*
          getOutgoingAsyncInvitationsResponse) override;

  grpc::Status GetIncomingAsyncInvitations(
      grpc::ServerContext* context,
      const asphrdaemon::GetIncomingAsyncInvitationsRequest*
          getIncomingAsyncInvitationsRequest,
      asphrdaemon::GetIncomingAsyncInvitationsResponse*
          getIncomingAsyncInvitationsResponse) override;

  grpc::Status AcceptAsyncInvitation(
      grpc::ServerContext* context,
      const asphrdaemon::AcceptAsyncInvitationRequest*
          acceptAsyncInvitationRequest,
      asphrdaemon::AcceptAsyncInvitationResponse* acceptAsyncInvitationResponse)
      override;

  grpc::Status RejectAsyncInvitation(
      grpc::ServerContext* context,
      const asphrdaemon::RejectAsyncInvitationRequest*
          rejectAsyncInvitationRequest,
      asphrdaemon::RejectAsyncInvitationResponse* rejectAsyncInvitationResponse)
      override;

  grpc::Status SendMessage(
      grpc::ServerContext* context,
      const asphrdaemon::SendMessageRequest* sendMessageRequest,
      asphrdaemon::SendMessageResponse* sendMessageResponse) override;

  // Get Messages section
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

  // GetStatus returns the status of the daemon. Things like how long since the
  // last send/receive cycle, etc
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
};