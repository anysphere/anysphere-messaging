#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, AddFriendSync) {
  ResetStub();
  auto friends = register_people(2);
  auto& friend1 = friends.at(0);
  auto& friend2 = friends.at(1);

  string user1_story;
  string user2_story;
  {
    // get user 1's id via rpc call
    GetMyPublicIDRequest request;
    GetMyPublicIDResponse response;
    friend1.rpc->GetMyPublicID(nullptr, &request, &response);
    user1_story = response.story();
  }
  {
    // get user 2's id via rpc call
    GetMyPublicIDRequest request;
    GetMyPublicIDResponse response;
    friend2.rpc->GetMyPublicID(nullptr, &request, &response);
    user2_story = response.story();
  }
  {
    AddSyncFriendRequest request;
    request.set_unique_name(friend2.unique_name);
    request.set_display_name(friend2.display_name);
    request.set_story(user2_story);
    AddSyncFriendResponse response;
    friend1.rpc->AddSyncFriend(nullptr, &request, &response);
  }

  // check that we have an outgoing friend request
  {
    GetOutgoingSyncInvitationsRequest request;
    GetOutgoingSyncInvitationsResponse response;
    friend1.rpc->GetOutgoingSyncInvitations(nullptr, &request, &response);
    EXPECT_EQ(response.invitations_size(), 1);
    EXPECT_EQ(response.invitations(0).unique_name(), friend2.unique_name);
    EXPECT_EQ(response.invitations(0).display_name(), friend2.display_name);
    EXPECT_EQ(response.invitations(0).story(), user2_story);
  }
  // friend2 should have no friend request
  {
    GetOutgoingSyncInvitationsRequest request;
    GetOutgoingSyncInvitationsResponse response;
    friend2.rpc->GetOutgoingSyncInvitations(nullptr, &request, &response);
    EXPECT_EQ(response.invitations_size(), 0);
  }

  // now make the sync friend request in the other direction
  {
    AddSyncFriendRequest request;
    request.set_unique_name(friend1.unique_name);
    request.set_display_name(friend1.display_name);
    request.set_story(user1_story);
    AddSyncFriendResponse response;
    friend2.rpc->AddSyncFriend(nullptr, &request, &response);
  }
  {
    GetOutgoingSyncInvitationsRequest request;
    GetOutgoingSyncInvitationsResponse response;
    friend2.rpc->GetOutgoingSyncInvitations(nullptr, &request, &response);
    EXPECT_EQ(response.invitations_size(), 1);
    EXPECT_EQ(response.invitations(0).unique_name(), friend1.unique_name);
    EXPECT_EQ(response.invitations(0).display_name(), friend1.display_name);
    EXPECT_EQ(response.invitations(0).story(), user1_story);
  }

  {
    friend1.t->retrieve();
    friend1.t->send();
  }
  // nothing should've happened
  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    friend1.rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_EQ(response.friend_infos_size(), 0);
  }
  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    friend2.rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_EQ(response.friend_infos_size(), 0);
  }
  {
    friend2.t->retrieve();
    friend2.t->send();
  }
  // now 2 should be friends with 1
  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    friend1.rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_EQ(response.friend_infos_size(), 0);
  }
  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    friend2.rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_EQ(response.friend_infos_size(), 1);
    EXPECT_EQ(response.friend_infos(0).unique_name(), friend1.unique_name);
    EXPECT_EQ(response.friend_infos(0).display_name(), friend1.display_name);
    EXPECT_EQ(response.friend_infos(0).public_id(), get_public_id(friend1));
    EXPECT_EQ(response.friend_infos(0).invitation_progress(),
              asphrdaemon::InvitationProgress::Complete);
  }
  {
    friend1.t->retrieve();
    friend1.t->send();
  }
  // now 1 should be friends with 2
  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    friend1.rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_EQ(response.friend_infos_size(), 1);
    EXPECT_EQ(response.friend_infos(0).unique_name(), friend2.unique_name);
    EXPECT_EQ(response.friend_infos(0).display_name(), friend2.display_name);
    EXPECT_EQ(response.friend_infos(0).public_id(), get_public_id(friend2));
    EXPECT_EQ(response.friend_infos(0).invitation_progress(),
              asphrdaemon::InvitationProgress::Complete);
  }
}

TEST_F(DaemonRpcTest, AddFriendAndCheckFriendList) {
  ResetStub();
  // this test is already implemented in the two peron constructor
  auto [user1, user2] = generate_two_friends();

  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    user1.rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_EQ(response.friend_infos_size(), 1);
  }
  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    user2.rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_EQ(response.friend_infos_size(), 1);
  }
};

}  // namespace
}  // namespace asphr::testing