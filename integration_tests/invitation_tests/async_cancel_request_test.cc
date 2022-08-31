#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, CancelAsyncInvitation) {
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

  // cancel the invitation
  {
    CancelAsyncInvitationRequest request;
    CancelAsyncInvitationResponse response;

    request.set_public_id(p2_id);

    auto status = p1.rpc->CancelAsyncInvitation(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }
  // check that there is no invitation left
  {
    GetOutgoingAsyncInvitationsRequest request;
    GetOutgoingAsyncInvitationsResponse response;

    auto status =
        p1.rpc->GetOutgoingAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), 0);
  }
  {
    GetIncomingAsyncInvitationsRequest request;
    GetIncomingAsyncInvitationsResponse response;

    auto status =
        p2.rpc->GetIncomingAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), 0);
  }
}

}  // namespace
}  // namespace asphr::testing