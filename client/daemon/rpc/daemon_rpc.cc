//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "daemon_rpc.hpp"

#include "client/daemon/identifier/identifier.hpp"
#include "google/protobuf/util/time_util.h"

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

Status DaemonRpc::RegisterUser(
    ServerContext* context,
    const asphrdaemon::RegisterUserRequest* registerUserRequest,
    asphrdaemon::RegisterUserResponse* registerUserResponse) {
  ASPHR_LOG_INFO("RegisterUser() called.", rpc_call, "RegisterUser");

  if (G.db->has_registered()) {
    ASPHR_LOG_INFO("Already registered.", rpc_call, "RegisterUser");
    return Status(grpc::StatusCode::ALREADY_EXISTS, "already registered");
  }
  const auto name = registerUserRequest->name();
  const auto [invitation_public_key, invitation_private_key] =
      crypto::generate_invitation_keypair();

  const auto [kx_public_key, kx_secret_key] = crypto::generate_kx_keypair();
  const auto [pir_secret_key, pir_galois_keys] = generate_keys();

  auto beta_key = registerUserRequest->beta_key();

  // call register rpc to send the register request
  asphrserver::RegisterInfo request;
  request.set_invitation_public_key(invitation_public_key);
  request.set_beta_key(beta_key);

  asphrserver::RegisterResponse reply;
  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() +
                              std::chrono::seconds(60));

  Status status = stub->Register(&client_context, request, &reply);

  if (status.ok()) {
    ASPHR_LOG_INFO("Register success.", rpc_call, "RegisterUser");

    const auto authentication_token = reply.authentication_token();
    auto alloc_repeated = reply.allocation();
    const auto allocation =
        vector<int>(alloc_repeated.begin(), alloc_repeated.end());

    if (reply.authentication_token() == "") {
      ASPHR_LOG_ERR("Register failed: authentication token is empty.", rpc_call,
                    "RegisterUser");
      return Status(grpc::StatusCode::UNKNOWN, "authentication token is empty");
    }
    if (reply.allocation().empty()) {
      ASPHR_LOG_ERR("Register failed: allocation is empty.", rpc_call,
                    "RegisterUser");
      return Status(grpc::StatusCode::UNKNOWN, "allocation is empty");
    }
    if (reply.allocation().size() != 1) {
      ASPHR_LOG_ERR(
          "Register failed: allocation is not 1. We currently only support 1 "
          "allocation.",
          rpc_call, "RegisterUser");
      return Status(grpc::StatusCode::UNKNOWN, "allocation is empty");
    }
    //-------------------------------------------------------------------------
    // server side finished registration
    // update DB now
    // compute the public id here
    // NOTE: this functionality has been moved to the Identifier module
    //-------------------------------------------------------------------------

    auto public_id =
        PublicIdentifier(allocation.at(0), kx_public_key, invitation_public_key)
            .to_public_id();
    try {
      G.db->do_register(db::RegistrationFragment{
          .invitation_public_key = string_to_rust_u8Vec(invitation_public_key),
          .invitation_private_key =
              string_to_rust_u8Vec(invitation_private_key),
          .kx_public_key = string_to_rust_u8Vec(kx_public_key),
          .kx_private_key = string_to_rust_u8Vec(kx_secret_key),
          .allocation = allocation.at(0),
          .pir_secret_key = string_to_rust_u8Vec(pir_secret_key),
          .pir_galois_key = string_to_rust_u8Vec(pir_galois_keys),
          .authentication_token = authentication_token,
          .public_id = public_id,
      });
    } catch (const rust::Error& e) {
      ASPHR_LOG_ERR("Register failed in database.", error, e.what(), rpc_call,
                    "RegisterUser");
      return Status(grpc::StatusCode::UNKNOWN, e.what());
    }

  } else {
    ASPHR_LOG_ERR("Register failed.", error_code, status.error_code(),
                  error_message, status.error_message(), rpc_call,
                  "RegisterUser");
    return Status(grpc::StatusCode::UNAVAILABLE, status.error_message());
  }

  return Status::OK;
}

Status DaemonRpc::GetFriendList(
    ServerContext* context,
    const asphrdaemon::GetFriendListRequest* getFriendListRequest,
    asphrdaemon::GetFriendListResponse* getFriendListResponse) {
  ASPHR_LOG_INFO("GetFriendList() called.");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "GetFriendList");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    for (auto& s : G.db->get_friends()) {
      auto new_friend = getFriendListResponse->add_friend_infos();
      // we need public id here
      // so we need to query the address DB as well
      new_friend->set_unique_name(std::string(s.unique_name));
      new_friend->set_display_name(std::string(s.display_name));
      new_friend->set_public_id(std::string(s.public_id));
      new_friend->set_invitation_progress(
          asphrdaemon::InvitationProgress::Complete);
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Database failed.", error, e.what(), rpc_call,
                  "GetFriendList");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::GetMyPublicID(
    grpc::ServerContext* context,
    const asphrdaemon::GetMyPublicIDRequest* getMyPublicIDRequest,
    asphrdaemon::GetMyPublicIDResponse* getMyPublicIDResponse) {
  ASPHR_LOG_INFO("GetMyPublicID() called.");
  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "GetMyPublicID");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    // query the db for registration info
    auto registration_info = G.db->get_small_registration();
    // set the public ID in the response
    // public ID is constant, so no need to regenerate it
    getMyPublicIDResponse->set_public_id(
        std::string(registration_info.public_id));

    // we also need to generate a story
    // This will ideally be different each time.
    // so we call the story generation function each call
    int index = registration_info.allocation;
    string kx_public_key =
        rust_u8Vec_to_string(registration_info.kx_public_key);
    string story = SyncIdentifier(index, kx_public_key).to_story();
    getMyPublicIDResponse->set_story(story);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Database failed.", error, e.what(), rpc_call,
                  "GetMyPublicID");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }  // catch (const std::exception& e) {
     // ASPHR_LOG_ERR("Assertion Failed.", error, e.what(), rpc_call,
     //              "GetMyPublicID");
     // return Status(grpc::StatusCode::UNKNOWN, e.what());
  //}
  return Status::OK;
}

Status DaemonRpc::IsValidPublicID(
    grpc::ServerContext* context,
    const asphrdaemon::IsValidPublicIDRequest* isValidPublicIDRequest,
    asphrdaemon::IsValidPublicIDResponse* isValidPublicIDResponse) {
  ASPHR_LOG_INFO("IsValidPublicID() called.", rpc_call, "IsValidPublicID");

  auto public_id_maybe =
      PublicIdentifier::from_public_id(isValidPublicIDRequest->public_id());
  if (!public_id_maybe.ok()) {
    isValidPublicIDResponse->set_valid(false);
    return Status::OK;
  }
  isValidPublicIDResponse->set_valid(true);
  return Status::OK;
}

// ---------------------------------------
// ---------------------------------------
// ||   Start: Invitation Functions    ||
// ---------------------------------------
// ---------------------------------------

Status DaemonRpc::AddSyncFriend(
    grpc::ServerContext* context,
    const asphrdaemon::AddSyncFriendRequest* addSyncFriendRequest,
    asphrdaemon::AddSyncFriendResponse* addSyncFriendResponse) {
  ASPHR_LOG_INFO("AddSyncFriend() called.", rpc_call, "AddSyncFriend");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "AddSyncFriend");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto sync_id_maybe =
      SyncIdentifier::from_story(addSyncFriendRequest->story());
  if (!sync_id_maybe.ok()) {
    ASPHR_LOG_ERR("Failed to parse sync ID.", rpc_call, "AddSyncFriend",
                  error_message, sync_id_maybe.status().message(), error_code,
                  sync_id_maybe.status().code());
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid story");
  }
  auto sync_id = sync_id_maybe.value();

  auto reg = G.db->get_small_registration();
  auto [read_key, write_key] = crypto::derive_read_write_keys(
      rust_u8Vec_to_string(reg.kx_public_key),
      rust_u8Vec_to_string(reg.kx_private_key), sync_id.kx_public_key);

  // try to add them to the database
  try {
    auto outgoing_sync_invitation_params = db::AddOutgoingSyncInvitationParams{
        addSyncFriendRequest->unique_name(),
        addSyncFriendRequest->display_name(),
        addSyncFriendRequest->story(),
        string_to_rust_u8Vec(sync_id.kx_public_key),
        sync_id.index,
        string_to_rust_u8Vec(read_key),
        string_to_rust_u8Vec(write_key),
        MAX_FRIENDS};

    G.db->add_outgoing_sync_invitation(outgoing_sync_invitation_params);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to add outgoing sync invitation.", error, e.what(),
                  rpc_call, "AddSyncFriend");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

grpc::Status DaemonRpc::AddAsyncFriend(
    grpc::ServerContext* context,
    const asphrdaemon::AddAsyncFriendRequest* addAsyncFriendRequest,
    asphrdaemon::AddAsyncFriendResponse* addAsyncFriendResponse) {
  ASPHR_LOG_INFO("AddAsyncFriend() called.", rpc_call, "AddAsyncFriend");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "AddAsyncFriend");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  std::string message = addAsyncFriendRequest->message();

  if (message.size() > INVITATION_MESSAGE_MAX_PLAINTEXT_SIZE) {
    ASPHR_LOG_ERR("Message is too long.", rpc_call, "AddAsyncFriend",
                  max_length, INVITATION_MESSAGE_MAX_PLAINTEXT_SIZE);
    return Status(grpc::StatusCode::INVALID_ARGUMENT,
                  "friend request message is too long");
  }

  auto public_id_maybe =
      PublicIdentifier::from_public_id(addAsyncFriendRequest->public_id());
  if (!public_id_maybe.ok()) {
    ASPHR_LOG_ERR("Failed to parse public ID.", rpc_call, "AddAsyncFriend",
                  error_message, public_id_maybe.status().message(), error_code,
                  public_id_maybe.status().code());
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid public ID");
  }
  auto public_id = public_id_maybe.value();

  auto reg = G.db->get_small_registration();
  auto [read_key, write_key] = crypto::derive_read_write_keys(
      rust_u8Vec_to_string(reg.kx_public_key),
      rust_u8Vec_to_string(reg.kx_private_key), public_id.kx_public_key);

  // we can now push the request into the database
  try {
    auto outgoing_async_invitation_params =
        db::AddOutgoingAsyncInvitationParams{
            addAsyncFriendRequest->unique_name(),
            addAsyncFriendRequest->display_name(),
            addAsyncFriendRequest->public_id(),
            string_to_rust_u8Vec(public_id.invitation_public_key),
            string_to_rust_u8Vec(public_id.kx_public_key),
            addAsyncFriendRequest->message(),
            public_id.index,
            string_to_rust_u8Vec(read_key),
            string_to_rust_u8Vec(write_key),
            MAX_FRIENDS};
    G.db->add_outgoing_async_invitation(outgoing_async_invitation_params);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to add outgoing async invitation.", error, e.what(),
                  rpc_call, "AddAsyncFriend");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }
  ASPHR_LOG_INFO("AddAsyncFriend() done.", rpc_call, "AddAsyncFriend");
  return Status::OK;
}

grpc::Status DaemonRpc::GetOutgoingSyncInvitations(
    grpc::ServerContext* context,
    const asphrdaemon::GetOutgoingSyncInvitationsRequest*
        getOutgoingSyncInvitationsRequest,
    asphrdaemon::GetOutgoingSyncInvitationsResponse*
        getOutgoingSyncInvitationsResponse) {
  ASPHR_LOG_INFO("GetOutgoingSyncInvitations() called.", rpc_call,
                 "GetOutgoingSyncInvitations");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call,
                   "GetOutgoingSyncInvitations");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    auto outgoing_sync_invitations = G.db->get_outgoing_sync_invitations();
    for (auto outgoing_sync_invitation : outgoing_sync_invitations) {
      auto invitation = getOutgoingSyncInvitationsResponse->add_invitations();
      invitation->set_unique_name(string(outgoing_sync_invitation.unique_name));
      invitation->set_display_name(
          string(outgoing_sync_invitation.display_name));
      invitation->set_story(string(outgoing_sync_invitation.story));
      // convert outgoing_sync_invitation.sent_at
      auto sent_at = absl::FromUnixMicros(outgoing_sync_invitation.sent_at);
      auto timestamp_str = absl::FormatTime(sent_at);
      auto timestamp = invitation->mutable_sent_at();
      using TimeUtil = google::protobuf::util::TimeUtil;
      auto success = TimeUtil::FromString(timestamp_str, timestamp);
      if (!success) {
        ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                      rpc_call, "GetOutgoingSyncInvitations");
        return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
      }
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get outgoing friend requests.", error, e.what(),
                  rpc_call, "GetOutgoingSyncInvitations");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }
  return Status::OK;
}

Status DaemonRpc::GetOutgoingAsyncInvitations(
    grpc::ServerContext* context,
    const asphrdaemon::GetOutgoingAsyncInvitationsRequest*
        getOutgoingAsyncInvitationsRequest,
    asphrdaemon::GetOutgoingAsyncInvitationsResponse*
        getOutgoingAsyncInvitationsResponse) {
  ASPHR_LOG_INFO("GetOutgoingAsyncInvitations() called.", rpc_call,
                 "GetOutgoingAsyncInvitations");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call,
                   "GetOutgoingAsyncInvitations");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    auto outgoing_async_invitations = G.db->get_outgoing_async_invitations();
    for (auto outgoing_async_invitation : outgoing_async_invitations) {
      auto invitation = getOutgoingAsyncInvitationsResponse->add_invitations();
      invitation->set_unique_name(
          string(outgoing_async_invitation.unique_name));
      invitation->set_display_name(
          string(outgoing_async_invitation.display_name));
      invitation->set_public_id(string(outgoing_async_invitation.public_id));
      invitation->set_message(string(outgoing_async_invitation.message));
      // convert outgoing_async_invitation.sent_at
      auto sent_at = absl::FromUnixMicros(outgoing_async_invitation.sent_at);
      auto timestamp_str = absl::FormatTime(sent_at);
      auto timestamp = invitation->mutable_sent_at();
      using TimeUtil = google::protobuf::util::TimeUtil;
      auto success = TimeUtil::FromString(timestamp_str, timestamp);
      if (!success) {
        ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                      rpc_call, "GetOutgoingAsyncInvitations");
        return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
      }
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get outgoing friend requests.", error, e.what(),
                  rpc_call, "GetOutgoingAsyncFriendRequests");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }
  return Status::OK;
}

Status DaemonRpc::GetIncomingAsyncInvitations(
    grpc::ServerContext* context,
    const asphrdaemon::GetIncomingAsyncInvitationsRequest*
        getIncomingAsyncInvitationsRequest,
    asphrdaemon::GetIncomingAsyncInvitationsResponse*
        getIncomingAsyncInvitationsResponse) {
  // essentially a clone of the getoutgoingfriendrequests rpc
  ASPHR_LOG_INFO("GetIncomingAsyncInvitations() called.", rpc_call,
                 "GetIncomingAsyncInvitations");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call,
                   "GetIncomingAsyncInvitations");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    auto incoming_invitations = G.db->get_incoming_invitations();
    for (auto incoming_invitation : incoming_invitations) {
      auto invitation = getIncomingAsyncInvitationsResponse->add_invitations();
      invitation->set_public_id(string(incoming_invitation.public_id));
      invitation->set_message(string(incoming_invitation.message));
      auto received_at = absl::FromUnixMicros(incoming_invitation.received_at);
      auto timestamp_str = absl::FormatTime(received_at);
      auto timestamp = invitation->mutable_received_at();
      using TimeUtil = google::protobuf::util::TimeUtil;
      auto success = TimeUtil::FromString(timestamp_str, timestamp);
      if (!success) {
        ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                      rpc_call, "GetIncomingAsyncInvitations");
        return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
      }
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get incoming invitations.", error, e.what(),
                  rpc_call, "GetIncomingAsyncInvitations");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }
  return Status::OK;
}

Status DaemonRpc::AcceptAsyncInvitation(
    grpc::ServerContext* context,
    const asphrdaemon::AcceptAsyncInvitationRequest*
        acceptAsyncInvitationRequest,
    asphrdaemon::AcceptAsyncInvitationResponse* acceptAsyncInvitationResponse) {
  ASPHR_LOG_INFO("AcceptAsyncInvitation() called.", rpc_call,
                 "AcceptAsyncInvitation");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call,
                   "AcceptAsyncInvitation");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }
  try {
    auto public_id_maybe = PublicIdentifier::from_public_id(
        acceptAsyncInvitationRequest->public_id());
    if (!public_id_maybe.ok()) {
      ASPHR_LOG_ERR("Failed to parse public ID.", rpc_call,
                    "AcceptAsyncInvitation", error_message,
                    public_id_maybe.status().message(), error_code,
                    public_id_maybe.status().code());
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid public ID");
    }
    auto public_id = public_id_maybe.value();

    auto reg = G.db->get_small_registration();
    auto [read_key, write_key] = crypto::derive_read_write_keys(
        rust_u8Vec_to_string(reg.kx_public_key),
        rust_u8Vec_to_string(reg.kx_private_key), public_id.kx_public_key);

    auto accept_incoming_invitation_params = db::AcceptIncomingInvitationParams{
        acceptAsyncInvitationRequest->public_id(),
        acceptAsyncInvitationRequest->unique_name(),
        acceptAsyncInvitationRequest->display_name(),
        string_to_rust_u8Vec(public_id.invitation_public_key),
        string_to_rust_u8Vec(public_id.kx_public_key),
        public_id.index,
        string_to_rust_u8Vec(read_key),
        string_to_rust_u8Vec(write_key),
        MAX_FRIENDS};

    G.db->accept_incoming_invitation(accept_incoming_invitation_params);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to accept async friend request.", error, e.what(),
                  rpc_call, "AcceptAsyncInvitation");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::RejectAsyncInvitation(
    grpc::ServerContext* context,
    const asphrdaemon::RejectAsyncInvitationRequest*
        rejectAsyncInvitationRequest,
    asphrdaemon::RejectAsyncInvitationResponse* rejectAsyncInvitationResponse) {
  ASPHR_LOG_INFO("RejectAsyncInvitation() called.", rpc_call,
                 "RejectAsyncInvitation");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call,
                   "RejectAsyncInvitation");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    // call rust db to reject the friend request
    G.db->deny_incoming_invitation(rejectAsyncInvitationRequest->public_id());
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to reject async friend request.", error, e.what(),
                  rpc_call, "RejectAsyncInvitation");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::CancelAsyncInvitation(
    grpc::ServerContext* context,
    const asphrdaemon::CancelAsyncInvitationRequest*
        cancelAsyncInvitationRequest,
    asphrdaemon::CancelAsyncInvitationResponse* cancelAsyncInvitationResponse) {
  ASPHR_LOG_INFO("CancelAsyncInvitation() called.", rpc_call,
                 "CancelAsyncInvitation");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call,
                   "CancelAsyncInvitation");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    G.db->remove_outgoing_async_invitation(
        cancelAsyncInvitationRequest->public_id());
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to cancel async friend request.", error, e.what(),
                  rpc_call, "CancelAsyncInvitation");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::RemoveFriend(
    ServerContext* context,
    const asphrdaemon::RemoveFriendRequest* removeFriendRequest,
    asphrdaemon::RemoveFriendResponse* removeFriendResponse) {
  ASPHR_LOG_INFO("RemoveFriend() called.", rpc_call, "RemoveFriend",
                 friend_unique_name, removeFriendRequest->unique_name());

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "RemoveFriend");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    G.db->delete_friend(removeFriendRequest->unique_name());
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to remove friend.", error, e.what(), rpc_call,
                  "RemoveFriend");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

// ---------------------------------------
// ---------------------------------------
// ||     End: Invitation Functions     ||
// ---------------------------------------
// ---------------------------------------

Status DaemonRpc::SendMessage(
    ServerContext* context,
    const asphrdaemon::SendMessageRequest* sendMessageRequest,
    asphrdaemon::SendMessageResponse* sendMessageResponse) {
  const auto names = sendMessageRequest->unique_name();
  const vector<string> unique_names(names.begin(), names.end());
  ASPHR_LOG_INFO("SendMessage() called.", rpc_call, "SendMessage",
                 friend_unique_name, absl::StrJoin(unique_names, ", "), message,
                 sendMessageRequest->message());

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "SendMessage");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto message = sendMessageRequest->message();

  rust::Vec<rust::String> unique_names_vec;
  for (const auto& unique_name : unique_names) {
    unique_names_vec.push_back(rust::String(unique_name));
  }

  try {
    G.db->queue_message_to_send(unique_names_vec, message,
                                GUARANTEED_SINGLE_MESSAGE_SIZE);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to send message.", error, e.what(), rpc_call,
                  "SendMessage");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::GetMessages(
    ServerContext* context,
    const asphrdaemon::GetMessagesRequest* getMessagesRequest,
    asphrdaemon::GetMessagesResponse* getMessagesResponse) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  ASPHR_LOG_INFO("GetMessages() called.", rpc_call, "GetMessages");

  if (!G.db->has_registered()) {
    ASPHR_LOG_ERR("Need to register first.", rpc_call, "GetMessages");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto filter = getMessagesRequest->filter();

  if (filter != asphrdaemon::GetMessagesRequest::ALL &&
      filter != asphrdaemon::GetMessagesRequest::NEW) {
    ASPHR_LOG_ERR("Invalid filter.", rpc_call, "GetMessages", filter, filter);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid filter");
  }

  try {
    auto messages = G.db->get_received_messages(db::MessageQuery{
        .limit = -1,  // TODO: make this configurable
        .filter = filter == asphrdaemon::GetMessagesRequest::ALL
                      ? db::MessageFilter::All
                      : db::MessageFilter::New,
        .delivery_status = db::DeliveryStatus::Delivered,
        .sort_by = db::SortBy::DeliveredAt,
        .after = 0});

    for (auto& m : messages) {
      auto message_info = getMessagesResponse->add_messages();

      // auto baseMessage = message_info->mutable_m();
      // baseMessage->set_id(m.uid);
      // baseMessage->set_message(std::string(m.content));
      // baseMessage->set_unique_name(std::string(m.from_unique_name));
      // baseMessage->set_display_name(std::string(m.from_display_name));
      message_info->set_uid(m.uid);
      message_info->set_message(std::string(m.content));

      message_info->set_from_unique_name(std::string(m.from_unique_name));
      message_info->set_from_display_name(std::string(m.from_display_name));

      message_info->set_seen(m.seen);
      message_info->set_delivered(m.delivered);

      auto other_recipiends = m.other_recipients;
      for (auto& f : other_recipiends) {
        auto maybe_friend = message_info->add_other_recipients();
        maybe_friend->set_public_id(std::string(f.public_id));
        maybe_friend->set_unique_name(std::string(f.unique_name));
        maybe_friend->set_display_name(std::string(f.display_name));
      }

      auto delivered_at = absl::FromUnixMicros(m.delivered_at);
      auto timestamp_str = absl::FormatTime(delivered_at);
      auto timestamp = message_info->mutable_delivered_at();

      auto success = TimeUtil::FromString(timestamp_str, timestamp);
      if (!success) {
        ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                      rpc_call, "GetMessages");
        return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
      }
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get messages.", error, e.what(), rpc_call,
                  "GetMessages");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

// WARNING: this method is subtle. please take a moment to understand
// what's going on before modifying it.
Status DaemonRpc::GetMessagesStreamed(
    ServerContext* context,
    const asphrdaemon::GetMessagesRequest* getMessagesRequest,
    ServerWriter<asphrdaemon::GetMessagesResponse>* writer) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  ASPHR_LOG_INFO("GetStreamedMessages() called.", rpc_call,
                 "GetStreamedMessages");

  if (!G.db->has_registered()) {
    ASPHR_LOG_ERR("Need to register first.", rpc_call, "GetStreamedMessages");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto filter = getMessagesRequest->filter();

  // guarantee: the caller will get each message in the order of the
  // delivered_at timestamp. each message will be delivered exactly once.
  // we do 0 here because the db.get_most_recent_delivered_at returns 0 if no
  // message found
  absl::Time last_delivered_at = absl::FromUnixMicros(0);

  try {
    auto messages = G.db->get_received_messages(db::MessageQuery{
        .limit = -1,  // TODO: make this configurable
        .filter = filter == asphrdaemon::GetMessagesRequest::ALL
                      ? db::MessageFilter::All
                      : db::MessageFilter::New,
        .delivery_status = db::DeliveryStatus::Delivered,
        .sort_by = db::SortBy::DeliveredAt,
        .after = 0,
    });

    asphrdaemon::GetMessagesResponse response;

    for (auto& m : messages) {
      auto message_info = response.add_messages();

      // auto baseMessage = message_info->mutable_m();
      // baseMessage->set_id(m.uid);
      // baseMessage->set_message(std::string(m.content));
      // baseMessage->set_unique_name(std::string(m.from_unique_name));
      // baseMessage->set_display_name(std::string(m.from_display_name));
      message_info->set_uid(m.uid);
      message_info->set_message(std::string(m.content));

      message_info->set_from_unique_name(std::string(m.from_unique_name));
      message_info->set_from_display_name(std::string(m.from_display_name));

      message_info->set_seen(m.seen);
      message_info->set_delivered(m.delivered);

      auto other_recipiends = m.other_recipients;
      for (auto& f : other_recipiends) {
        auto maybe_friend = message_info->add_other_recipients();
        maybe_friend->set_public_id(std::string(f.public_id));
        maybe_friend->set_unique_name(std::string(f.unique_name));
        maybe_friend->set_display_name(std::string(f.display_name));
      }

      auto delivered_at = absl::FromUnixMicros(m.delivered_at);
      if (delivered_at > last_delivered_at) {
        last_delivered_at = delivered_at;
      }
      auto timestamp_str = absl::FormatTime(delivered_at);
      auto timestamp = message_info->mutable_delivered_at();
      auto success = TimeUtil::FromString(timestamp_str, timestamp);
      if (!success) {
        ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                      rpc_call, "GetMessagesStreamed");
        return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
      }
    }

    writer->Write(response);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get messages.", error, e.what(), rpc_call,
                  "GetMessagesStreamed");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  // keep the connection open forever
  while (!context->IsCancelled()) {
    {
      std::unique_lock<std::mutex> l(G.message_notification_cv_mutex);
      G.message_notification_cv.wait_for(l, std::chrono::seconds(60 * 60), [&] {
        return G.db->get_most_recent_received_delivered_at() !=
               absl::ToUnixMicros(last_delivered_at);
      });
      if (G.db->get_most_recent_received_delivered_at() ==
          absl::ToUnixMicros(last_delivered_at)) {
        continue;  // continue the loop to check if we are cancelled, because
                   // it has been an hour
      }
    }

    // same code as above. only difference: the .after = last_delivered_at
    try {
      auto messages = G.db->get_received_messages(db::MessageQuery{
          .limit = -1,  // TODO: make this configurable
          .filter = filter == asphrdaemon::GetMessagesRequest::ALL
                        ? db::MessageFilter::All
                        : db::MessageFilter::New,
          .delivery_status = db::DeliveryStatus::Delivered,
          .sort_by = db::SortBy::DeliveredAt,
          .after = absl::ToUnixMicros(last_delivered_at),
      });

      asphrdaemon::GetMessagesResponse response;

      for (auto& m : messages) {
        auto message_info = response.add_messages();

        // auto baseMessage = message_info->mutable_m();
        // baseMessage->set_id(m.uid);
        // baseMessage->set_message(std::string(m.content));
        // baseMessage->set_unique_name(std::string(m.from_unique_name));
        // baseMessage->set_display_name(std::string(m.from_display_name));
        message_info->set_uid(m.uid);
        message_info->set_message(std::string(m.content));

        message_info->set_from_unique_name(std::string(m.from_unique_name));
        message_info->set_from_display_name(std::string(m.from_display_name));

        message_info->set_seen(m.seen);
        message_info->set_delivered(m.delivered);

        auto other_recipiends = m.other_recipients;
        for (auto& f : other_recipiends) {
          auto maybe_friend = message_info->add_other_recipients();
          maybe_friend->set_public_id(std::string(f.public_id));
          maybe_friend->set_unique_name(std::string(f.unique_name));
          maybe_friend->set_display_name(std::string(f.display_name));
        }

        auto delivered_at = absl::FromUnixMicros(m.delivered_at);
        if (delivered_at > last_delivered_at) {
          last_delivered_at = delivered_at;
        }
        auto timestamp_str = absl::FormatTime(delivered_at);
        auto timestamp = message_info->mutable_delivered_at();
        auto success = TimeUtil::FromString(timestamp_str, timestamp);
        if (!success) {
          ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                        rpc_call, "GetMessagesStreamed");
          return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
        }
      }

      writer->Write(response);
    } catch (const rust::Error& e) {
      ASPHR_LOG_ERR("Failed to get messages.", error, e.what(), rpc_call,
                    "GetMessagesStreamed");
      return Status(grpc::StatusCode::UNKNOWN, e.what());
    }
  }

  return Status::OK;
}

Status DaemonRpc::GetOutboxMessages(
    ServerContext* context,
    const asphrdaemon::GetOutboxMessagesRequest* getOutboxMessagesRequest,
    asphrdaemon::GetOutboxMessagesResponse* getOutboxMessagesResponse) {
  // TODO: somehow merge this with GetSentMessaegs because they are very
  // similar
  using TimeUtil = google::protobuf::util::TimeUtil;
  ASPHR_LOG_INFO("GetOutboxMessages() called.", rpc_call, "GetOutboxMessages");

  if (!G.db->has_registered()) {
    ASPHR_LOG_ERR("Need to register first.", rpc_call, "GetOutboxMessages");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    auto messages = G.db->get_sent_messages(
        db::MessageQuery{.limit = -1,  // TODO: make this configurable
                         .filter = db::MessageFilter::All,
                         .delivery_status = db::DeliveryStatus::Undelivered,
                         .sort_by = db::SortBy::SentAt,
                         .after = 0});

    for (auto& m : messages) {
      auto message_info = getOutboxMessagesResponse->add_messages();

      // auto baseMessage = message_info->mutable_m();
      // baseMessage->set_id(m.uid);
      // baseMessage->set_message(std::string(m.content));
      // baseMessage->set_unique_name(std::string(m.to_unique_name));
      // baseMessage->set_display_name(std::string(m.to_display_name));
      // message_info->set_delivered(m.delivered);

      message_info->set_uid(m.uid);
      message_info->set_message(std::string(m.content));

      auto to_friends = m.to_friends;
      for (auto& f : to_friends) {
        auto outgoing_friend = message_info->add_to_friends();
        outgoing_friend->set_unique_name(std::string(f.unique_name));
        outgoing_friend->set_display_name(std::string(f.display_name));
        outgoing_friend->set_delivered(f.delivered);

        {
          auto delivered_at = absl::FromUnixMicros(f.delivered_at);
          auto timestamp_str = absl::FormatTime(delivered_at);
          auto timestamp = outgoing_friend->mutable_delivered_at();
          auto success = TimeUtil::FromString(timestamp_str, timestamp);
          if (!success) {
            ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                          rpc_call, "GetSentMessages");
            return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
          }
        }
      }

      {
        auto sent_at = absl::FromUnixMicros(m.sent_at);
        auto timestamp_str = absl::FormatTime(sent_at);
        auto timestamp = message_info->mutable_sent_at();
        auto success = TimeUtil::FromString(timestamp_str, timestamp);
        if (!success) {
          ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                        rpc_call, "GetSentMessages");
          return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
        }
      }
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get messages.", error, e.what(), rpc_call,
                  "GetMessages");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::GetSentMessages(
    ServerContext* context,
    const asphrdaemon::GetSentMessagesRequest* getSentMessagesRequest,
    asphrdaemon::GetSentMessagesResponse* getSentMessagesResponse) {
  using TimeUtil = google::protobuf::util::TimeUtil;
  ASPHR_LOG_INFO("GetSentMessages() called.", rpc_call, "GetSentMessages");

  if (!G.db->has_registered()) {
    ASPHR_LOG_ERR("Need to register first.", rpc_call, "GetSentMessages");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  try {
    auto messages = G.db->get_sent_messages(
        db::MessageQuery{.limit = -1,  // TODO: make this configurable
                         .filter = db::MessageFilter::All,
                         .delivery_status = db::DeliveryStatus::Delivered,
                         .sort_by = db::SortBy::SentAt,
                         .after = 0});

    for (auto& m : messages) {
      auto message_info = getSentMessagesResponse->add_messages();

      // auto baseMessage = message_info->mutable_m();
      // baseMessage->set_id(m.uid);
      // baseMessage->set_message(std::string(m.content));
      // baseMessage->set_unique_name(std::string(m.to_unique_name));
      // baseMessage->set_display_name(std::string(m.to_display_name));
      // message_info->set_delivered(m.delivered);

      message_info->set_uid(m.uid);
      message_info->set_message(std::string(m.content));

      auto to_friends = m.to_friends;
      for (auto& f : to_friends) {
        auto outgoing_friend = message_info->add_to_friends();
        outgoing_friend->set_unique_name(std::string(f.unique_name));
        outgoing_friend->set_display_name(std::string(f.display_name));
        outgoing_friend->set_delivered(f.delivered);

        {
          auto delivered_at = absl::FromUnixMicros(f.delivered_at);
          auto timestamp_str = absl::FormatTime(delivered_at);
          auto timestamp = outgoing_friend->mutable_delivered_at();
          auto success = TimeUtil::FromString(timestamp_str, timestamp);
          if (!success) {
            ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                          rpc_call, "GetSentMessages");
            return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
          }
        }
      }

      {
        auto sent_at = absl::FromUnixMicros(m.sent_at);
        auto timestamp_str = absl::FormatTime(sent_at);
        auto timestamp = message_info->mutable_sent_at();
        auto success = TimeUtil::FromString(timestamp_str, timestamp);
        if (!success) {
          ASPHR_LOG_ERR("Failed to parse timestamp.", error, timestamp_str,
                        rpc_call, "GetSentMessages");
          return Status(grpc::StatusCode::UNKNOWN, "invalid timestamp");
        }
      }
    }
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get messages.", error, e.what(), rpc_call,
                  "GetMessages");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

Status DaemonRpc::MessageSeen(
    ServerContext* context,
    const asphrdaemon::MessageSeenRequest* messageSeenRequest,
    asphrdaemon::MessageSeenResponse* messageSeenResponse) {
  ASPHR_LOG_INFO("MessageSeen() called.", rpc_call, "MessageSeen");

  if (!G.db->has_registered()) {
    ASPHR_LOG_INFO("Need to register first.", rpc_call, "MessageSeen");
    return Status(grpc::StatusCode::UNAUTHENTICATED, "not registered");
  }

  auto message_id = messageSeenRequest->id();

  try {
    G.db->mark_message_as_seen(message_id);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to mark message as seen.", error, e.what(), rpc_call,
                  "MessageSeen");
    return Status(grpc::StatusCode::UNKNOWN, e.what());
  }

  return Status::OK;
}

auto DaemonRpc::GetStatus(ServerContext* context,
                          const asphrdaemon::GetStatusRequest* getStatusRequest,
                          asphrdaemon::GetStatusResponse* getStatusResponse)
    -> Status {
  ASPHR_LOG_INFO("GetStatus() called.", rpc_call, "GetStatus");

  try {
    getStatusResponse->set_registered(G.db->has_registered());
    getStatusResponse->set_release_hash(RELEASE_COMMIT_HASH);
    getStatusResponse->set_latency_seconds(G.db->get_latency());
    getStatusResponse->set_server_address(
        std::string(G.db->get_server_address()));
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get status.", error, e.what(), rpc_call,
                  "GetStatus");
    return Status(grpc::StatusCode::RESOURCE_EXHAUSTED, e.what());
  }

  return Status::OK;
}

auto DaemonRpc::GetLatency(
    ServerContext* context,
    const asphrdaemon::GetLatencyRequest* getLatencyRequest,
    asphrdaemon::GetLatencyResponse* getLatencyResponse) -> Status {
  ASPHR_LOG_INFO("GetLatency() called.", rpc_call, "GetLatency");

  try {
    getLatencyResponse->set_latency_seconds(G.db->get_latency());
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to get latency.", error, e.what(), rpc_call,
                  "GetLatency");
    return Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "get latency failed");
  }

  return Status::OK;
}

auto DaemonRpc::ChangeLatency(
    grpc::ServerContext* context,
    const asphrdaemon::ChangeLatencyRequest* changeLatencyRequest,
    asphrdaemon::ChangeLatencyResponse* changeLatencyResponse) -> Status {
  ASPHR_LOG_INFO("ChangeLatency() called.", rpc_call, "ChangeLatency");

  auto new_latency = changeLatencyRequest->latency_seconds();

  if (new_latency <= 0) {
    ASPHR_LOG_ERR("Invalid latency <= 0.", new_latency, new_latency, rpc_call,
                  "ChangeLatency");
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid latency");
  }

  try {
    G.db->set_latency(new_latency);
  } catch (const rust::Error& e) {
    ASPHR_LOG_ERR("Failed to change latency.", error, e.what(), rpc_call,
                  "ChangeLatency");
    return Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "set latency failed");
  }

  return Status::OK;
}

auto DaemonRpc::Kill(ServerContext* context,
                     const asphrdaemon::KillRequest* killRequest,
                     asphrdaemon::KillResponse* killResponse) -> Status {
  ASPHR_LOG_INFO("Kill() called.", rpc_call, "Kill");
  ASPHR_LOG_INFO("Daemon is shutting down.", rpc_call, "Kill");
  // exit normally! being killed is not a bad thing.
  // the daemon manager will restart us which is great :)
  //
  // the OS will clean up after us, so we don't need to worry
  ASPHR_LOG_ERR("Exiting.", status_code, 0);
  std::exit(0);
  return Status::OK;
}