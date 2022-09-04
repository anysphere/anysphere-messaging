//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "../daemon_setup.hpp"

// #define DIRECT_TRANSMISSION 1

namespace asphr::testing {
namespace {

// Test GRPC api AddAsyncFriend
// Test GRPC api GetIncomingAsyncFriendRequests
// Test GRPC api DecideAsyncFriendRequest
// Test GRPC api SendMessage after adding a friend

// Partitions:
// Number of existing friends: 0, 1
// Chunk of the database where we find the friend: first, middle, last
// Decision on the friend: accept, reject
// InvitationProgress: Incoming, OutgoingAsync, OutgoingSync, Complete
// Number of simultaneous requests: 1, >1
// Deleted: false, true

// Test two people are able to befriend each other
// by sending async requests to each other.
// TEST_F(DaemonRpcTest, AcceptInvitation) {
//   ResetStub();

//   auto&& [friend1, friend2] = generate_two_friends();
// }

// Friend1 -> Friend2
// Partition: Zerofriend basic flow.
// - one existing friends,
// - same chunks,
// - Progress = Incoming (friend2), OutgoingAsync (friend1), Complete (friend1,
// - friend2)
// - Not deleted
TEST_F(DaemonRpcTest, ZeroFriendAccept) {
  // Precondition:
  // Server is starting new everytime.
  // This means that p1, p2 will be allocated at index 0, 1
  // thus allow a request to be delivered with one call to the transmitter
  ResetStub();

  auto peoples = register_people(2);
  auto& p1 = peoples.at(0);  // sender
  auto& p2 = peoples.at(1);  // receiver

  // get the public id of the users
  string p1_id = get_public_id(p1);
  string p2_id = get_public_id(p2);

  // sender(p1) initialize a friend request to the receiver(p2)
  {
    AddAsyncFriendRequest request;
    AddAsyncFriendResponse response;

    request.set_unique_name("user2");
    request.set_display_name(p2.unique_name);
    request.set_public_id(p2_id);
    request.set_message("hello from user 1 to user 2");

    auto status = p1.rpc->AddAsyncFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // check that sender(p1) has receiver marked outgoingasync,
  // receiver(p2) has no friend request
  {
    GetOutgoingAsyncInvitationsRequest request;
    GetOutgoingAsyncInvitationsResponse response;

    auto status =
        p1.rpc->GetOutgoingAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), 1);
    auto& friend_info = response.invitations(0);
    // EXPECT_EQ(friend_info.unique_name(), p2.unique_name);
    EXPECT_EQ(friend_info.public_id(), p2_id);
  }
  {
    GetIncomingAsyncInvitationsRequest request;
    GetIncomingAsyncInvitationsResponse response;

    auto status =
        p2.rpc->GetIncomingAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), 0);
  }

  // start a timer
  auto start = std::chrono::high_resolution_clock::now();

#ifdef DIRECT_TRANSMISSION

  // send this request
  p1.t->transmit_async_friend_request();

  auto [start_index, end_index] =
      p2.t->update_async_invitation_retrieve_index();
  p2.t->retrieve_async_friend_request(start_index, end_index);

#else
  p1.t->send();
  p2.t->retrieve();
#endif

  // end the timer
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  auto duration_str = std::to_string(duration.count());
  cout << "Duration: " << duration_str << " ms" << endl;
  ASPHR_LOG_INFO("Time taken to process block:", duration, duration_str);

  // check that receiver(p2) has received the request
  {
    GetIncomingAsyncInvitationsRequest request;
    GetIncomingAsyncInvitationsResponse response;

    auto status =
        p2.rpc->GetIncomingAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), 1);
    const auto& friend_info = response.invitations(0);

    EXPECT_EQ(friend_info.public_id(), p1_id);
  }
  // receiver(p2) approves the request
  {
    AcceptAsyncInvitationRequest request;
    AcceptAsyncInvitationResponse response;

    request.set_unique_name("p1 local");
    request.set_display_name("ABCDEF");
    request.set_public_id(p1_id);

    auto status = p2.rpc->AcceptAsyncInvitation(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // verify that p2 has p1 as a friend
  {
    GetFriendListRequest request;
    GetFriendListResponse response;

    auto status = p2.rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.friend_infos_size(), 1);
    const auto& friend_info = response.friend_infos(0);

    // EXPECT_EQ(friend_info.unique_name(), p1.unique_name);
    EXPECT_EQ(friend_info.invitation_progress(), InvitationProgress::Complete);
    EXPECT_EQ(friend_info.public_id(), p1_id);
  }

  // TODO(stzh1555): check the control message flow.
}

// Friend1 -> Friend2
// Partition: Zerofriend basic flow.
// - 1 existing friends,
// - same chunks,
// - Progress = Incoming (friend2), OutgoingAsync (friend1), Complete (friend1,
// - friend2)
// - Not deleted
TEST_F(DaemonRpcTest, OneFriendAccept) {
  // Precondition:
  // Server is starting new everytime.
  // This means that p1, p2 will be allocated at index 0, 1
  // thus allow a request to be delivered with one call to the transmitter
  ResetStub();

  auto peoples = register_people(4);
  auto& p1 = peoples.at(0);  // sender
  auto& p2 = peoples.at(1);  // receiver
  auto& p3 = peoples.at(2);  // third party
  auto& p4 = peoples.at(3);  // fourth party

  // make p1 a friend of p3
  connect_friends_both_async(p1, p3);
  // make p2 a friend of p4
  connect_friends_both_async(p2, p4);

  // get the public id of the users
  string p1_id = get_public_id(p1);
  string p2_id = get_public_id(p2);

  // sender(p1) initialize a friend request to the receiver(p2)
  {
    AddAsyncFriendRequest request;
    AddAsyncFriendResponse response;

    request.set_unique_name("user2");
    request.set_display_name(p2.unique_name);
    request.set_public_id(p2_id);
    request.set_message("hello from user 1 to user 2");

    auto status = p1.rpc->AddAsyncFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // check that sender(p1) has receiver marked outgoingasync,
  // receiver(p2) has no friend request
  {
    GetOutgoingAsyncInvitationsRequest request;
    GetOutgoingAsyncInvitationsResponse response;

    auto status =
        p1.rpc->GetOutgoingAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), 1);
    const auto& invitation = response.invitations(0);

    // EXPECT_EQ(friend_info.unique_name(), p2.unique_name);
    EXPECT_EQ(invitation.public_id(), p2_id);
  }
  {
    GetIncomingAsyncInvitationsRequest request;
    GetIncomingAsyncInvitationsResponse response;

    auto status =
        p2.rpc->GetIncomingAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), 0);
  }

  // start a timer
  auto start = std::chrono::high_resolution_clock::now();

#ifdef DIRECT_TRANSMISSION

  // send this request
  p1.t->transmit_async_friend_request();

  auto [start_index, end_index] =
      p2.t->update_async_invitation_retrieve_index();
  p2.t->retrieve_async_friend_request(start_index, end_index);

#else
  p1.t->send();
  p2.t->retrieve();
#endif

  // end the timer
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  auto duration_str = std::to_string(duration.count());
  cout << "Duration: " << duration_str << " ms" << endl;
  ASPHR_LOG_INFO("Time taken to process block:", duration, duration_str);

  // check that receiver(p2) has received the request
  {
    GetIncomingAsyncInvitationsRequest request;
    GetIncomingAsyncInvitationsResponse response;

    auto status =
        p2.rpc->GetIncomingAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), 1);
    const auto& invitation = response.invitations(0);

    // unique name has not been determined yet.
    // nothing to check here?
    EXPECT_EQ(invitation.public_id(), p1_id);
  }
  // receiver(p2) approves the request
  {
    AcceptAsyncInvitationRequest request;
    AcceptAsyncInvitationResponse response;

    request.set_unique_name("p1 local");
    request.set_display_name("lyrica");
    request.set_public_id(p1_id);

    auto status = p2.rpc->AcceptAsyncInvitation(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // verify that p2 has p1 as a friend
  {
    GetFriendListRequest request;
    GetFriendListResponse response;

    auto status = p2.rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.friend_infos_size(), 2);
    // get it with the name
    const auto& friend_info = response.friend_infos(1);

    // EXPECT_EQ(friend_info.unique_name(), p1.unique_name);
    EXPECT_EQ(friend_info.invitation_progress(), InvitationProgress::Complete);

    // BUG
    EXPECT_EQ(friend_info.public_id(), p1_id);
  }
}

// Friend1 -> Friend2
// Partition: Zerofriend basic flow.
// - 0 existing friends,
// - same chunks,
// - Progress = Incoming (friend2), OutgoingAsync (friend1), Complete (both)
// - Not deleted
TEST_F(DaemonRpcTest, DISABLED_ZeroFriendFarAway) {
  // Precondition:
  // Server is starting new everytime.
  // This means that p1, p2 will be allocated at index 0, 1
  // thus allow a request to be delivered with one call to the transmitter
  ResetStub();

  auto const BLOCK_DISTANCE = 1;

  // Sualeh's guess is that this is the slow part of the test.
  const auto peoples =
      register_people(BLOCK_DISTANCE * ASYNC_FRIEND_REQUEST_BATCH_SIZE + 11);
  auto& p1 = peoples.at(0);  // sender
  // get the last user
  auto& p2 = peoples.at(peoples.size() - 2);  // receiver

  // get the public id of the users
  string p1_id = get_public_id(p1);
  string p2_id = get_public_id(p2);

  // sender(p1) initialize a friend request to the receiver(p2)
  {
    AddAsyncFriendRequest request;
    AddAsyncFriendResponse response;

    request.set_unique_name("user2");
    request.set_display_name(p2.unique_name);
    request.set_public_id(p2_id);
    request.set_message("hello from user 1 to user 2");

    auto status = p1.rpc->AddAsyncFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // check that sender(p1) has receiver marked outgoingasync,
  // receiver(p2) has no friend request
  {
    GetOutgoingAsyncInvitationsRequest request;
    GetOutgoingAsyncInvitationsResponse response;

    auto status =
        p1.rpc->GetOutgoingAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), 1);
    const auto& friend_info = response.invitations(0);
    // EXPECT_EQ(friend_info.unique_name(), p2.unique_name);
    EXPECT_EQ(friend_info.public_id(), p2_id);
  }
  {
    GetIncomingAsyncInvitationsRequest request;
    GetIncomingAsyncInvitationsResponse response;

    auto status =
        p2.rpc->GetIncomingAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), 0);
  }

#ifdef DIRECT_TRANSMISSION

  // send this request
  p1.t->transmit_async_friend_request();

  for (int i = 0; i < BLOCK_DISTANCE + 1; i++) {
    auto [start_index, end_index] =
        p2.t->update_async_invitation_retrieve_index();
    p2.t->retrieve_async_friend_request(start_index, end_index);
  }

#else
  p1.t->send();

  for (int i = 0; i < BLOCK_DISTANCE + 1; i++) {
    p2.t->retrieve();
  }
#endif

  // check that receiver(p2) has received the request
  {
    GetIncomingAsyncInvitationsRequest request;
    GetIncomingAsyncInvitationsResponse response;

    auto status =
        p2.rpc->GetIncomingAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), 1);
    auto& friend_info = response.invitations(0);

    EXPECT_EQ(friend_info.public_id(), p1_id);
  }
  // receiver(p2) approves the request
  {
    AcceptAsyncInvitationRequest request;
    AcceptAsyncInvitationResponse response;

    request.set_unique_name("p1 local");
    request.set_display_name("ABCDEF");
    request.set_public_id(p1_id);

    auto status = p2.rpc->AcceptAsyncInvitation(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // verify that p2 has p1 as a friend
  {
    GetFriendListRequest request;
    GetFriendListResponse response;

    auto status = p2.rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.friend_infos_size(), 1);
    const auto& friend_info = response.friend_infos(0);

    // EXPECT_EQ(friend_info.unique_name(), p1.unique_name);
    EXPECT_EQ(friend_info.invitation_progress(), InvitationProgress::Complete);
    EXPECT_EQ(friend_info.public_id(), p1_id);
  }
}

// Possibly important partitions for the future:
// - What if the server returns malicious data?

}  // namespace
}  // namespace asphr::testing