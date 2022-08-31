#include "attacker_template_test.hpp"

using namespace asphrdaemon;

/**
 * Things to test:
 * - the attacker template is working.
 **/

namespace asphr::testing {
namespace {
using TestTemplateClass =
    class AttackTest<AttackerTemplate<FastPIR, AccountManagerInMemory>>;
TEST_F(TestTemplateClass, AttackerTemplate) {
  // everything here should be exactly the same as in three_messages_test.cc
  // the only difference is that in the logs, the attacker will print "Hello
  // from MITM!"

  ResetStub();

  auto friends = generate_friends_from_list_of_pairs(4, {
                                                            {0, 1},
                                                            {0, 2},
                                                            {0, 3},
                                                        });

  // user 1 sends message to 2-4
  for (size_t i = 1; i < friends.size(); i++) {
    SendMessageRequest request;
    request.add_unique_name(friends.at(i).unique_name);
    request.set_message(absl::StrCat("hello from 0 to ", i));
    asphrdaemon::SendMessageResponse response;
    auto status = friends.at(0).rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // execute a round
  unordered_set<int> to_receive = {1, 2, 3};

  for (auto round = 0; round < 3; round++) {
    for (size_t i = 0; i < friends.size(); i++) {
      friends.at(i).t->retrieve();
      friends.at(i).t->send();
    }

    for (size_t i = 1; i < friends.size(); i++) {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status =
          friends.at(i).rpc->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      if (response.messages_size() > 1) {        // +1 for invitation message
        EXPECT_EQ(response.messages_size(), 2);  // +1 for invitation message
        EXPECT_EQ(response.messages(0).from_unique_name(),
                  friends.at(0).unique_name);
        EXPECT_EQ(response.messages(0).message(),
                  asphr::StrCat("hello from 0 to ", i));
        to_receive.erase(i);
      }
    }
  }

  EXPECT_EQ(to_receive.size(), 0);
};

// Test that a simple modification of attacker template
// is working as expected.
using TestDoSClass =
    class AttackTest<DoSAttacker<FastPIR, AccountManagerInMemory>>;
TEST_F(TestDoSClass, DoSAttacker) {
  // In this case async friending should fail.
  // because control message is not delivered.
  ResetStub();
  auto two_people = register_people(2);
  if (two_people.size() != 2) {
    FAIL();
  }
  auto& friend1 = two_people.at(0);
  auto& friend2 = two_people.at(1);
  // Do the usual async friending process
  // below we havea clone of connect_friends_both_async
  string friend1_id;
  string friend2_id;
  int friend1_allocation;
  {
    // get user 1's id via rpc call
    GetMyPublicIDRequest request;
    GetMyPublicIDResponse response;
    friend1.rpc->GetMyPublicID(nullptr, &request, &response);
    friend1_id = response.public_id();
  }
  {
    // get user 1's registration information
    auto friend1_registration = friend1.G->db->get_registration();
    friend1_allocation = friend1_registration.allocation;
  }
  {
    // get user 2's id via rpc call
    GetMyPublicIDRequest request;
    GetMyPublicIDResponse response;
    friend2.rpc->GetMyPublicID(nullptr, &request, &response);
    friend2_id = response.public_id();
  }

  ASPHR_LOG_INFO("Registration", friend1_id, friend1_id);
  ASPHR_LOG_INFO("Registration", friend2_id, friend2_id);
  // User 1 send user 2 a request
  {
    AddAsyncFriendRequest request;
    request.set_unique_name(friend2.unique_name);
    request.set_display_name(friend2.display_name);
    request.set_public_id(friend2_id);
    request.set_message("INVITATION-MESSAGE");
    AddAsyncFriendResponse response;
    auto status = friend1.rpc->AddAsyncFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    // crash fast if cannot establish friend request
    if (!status.ok()) {
      FAIL();
    }
  }

  friend1.t->send();
  friend2.t->retrieve_async_invitations(friend1_allocation,
                                        friend1_allocation + 1);
  // user 2 should have obtained the friend request from user1
  // user 2 now approve the friend request.
  {
    AcceptAsyncInvitationRequest request;
    request.set_unique_name(friend1.unique_name);
    request.set_display_name(friend1.display_name);
    request.set_public_id(friend1_id);
    AcceptAsyncInvitationResponse response;
    auto status =
        friend2.rpc->AcceptAsyncInvitation(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    // crash fast if cannot approve friend request
    if (!status.ok()) {
      FAIL();
    }
  }
  ASPHR_LOG_INFO("Friend 2 has accepted the request from friend 1!");
  // check that user 2 has user 1 as a friend
  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    friend2.rpc->GetFriendList(nullptr, &request, &response);
    int friend_index = -1;
    for (int i = 0; i < response.friend_infos_size(); i++) {
      if (response.friend_infos(i).unique_name() == friend1.unique_name) {
        EXPECT_EQ(friend_index, -1);
        friend_index = i;
      }
    }
    EXPECT_EQ(response.friend_infos(friend_index).unique_name(),
              friend1.unique_name);
    EXPECT_EQ(response.friend_infos(friend_index).display_name(),
              friend1.display_name);
    EXPECT_EQ(response.friend_infos(friend_index).public_id(), friend1_id);
    EXPECT_EQ(response.friend_infos(friend_index).invitation_progress(),
              asphrdaemon::InvitationProgress::Complete);
  }
  // --------------------------------------------------
  // --------------------------------------------------
  // Everything should be good up to this point.
  // However, the control messaging is not delivered.
  // So the tests below will fail.
  // retrieive system control message
  friend2.t->retrieve();
  // ACK the system control message
  friend2.t->send();
  // user 1 retrive ACK of system control message
  // and promotes user 2 to a friend
  friend1.t->retrieve();
  // user 1 should NOThave user 2 as a friend
  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    friend1.rpc->GetFriendList(nullptr, &request, &response);
    ASPHR_LOG_INFO("Friend 1 has " +
                   std::to_string(response.friend_infos_size()) + " friends");
    int friend_index = -1;
    for (int i = 0; i < response.friend_infos_size(); i++) {
      if (response.friend_infos(i).unique_name() == friend2.unique_name) {
        EXPECT_EQ(friend_index, -1);
        friend_index = i;
      }
    }
    EXPECT_EQ(friend_index, -1);
  }
};
}  // namespace
}  // namespace asphr::testing
