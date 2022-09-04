//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "../daemon_setup.hpp"

using namespace asphrdaemon;

/**
 * Things to test:
 * - multiple friends transmitting (one person sends message to n people)
 * - multiple friends retrieval (n people send message to one person)
 * - long messages (that are guranteed to be chunked. Bigger than
 *   GUARANTEED_SINGLE_MESSAGE_SIZE)
 * - multiple friends with long messages to one person
 * - one person sends multiple messages to another person.
 **/

namespace asphr::testing {
namespace {

class MultipleFriendsTest : public DaemonRpcTest {};

TEST_F(MultipleFriendsTest, SendLongMessage) {
  ResetStub();

  auto friends = generate_friends_from_list_of_pairs(4, {
                                                            {0, 1},
                                                            {0, 2},
                                                            {0, 3},
                                                        });

  string long_message;
  for (size_t j = 0; j < GUARANTEED_SINGLE_MESSAGE_SIZE + 1; j++) {
    long_message += "a";
  }

  // user 1 sends message to 2-4
  for (size_t i = 1; i < friends.size(); i++) {
    SendMessageRequest request;
    request.add_unique_name(friends.at(i).unique_name);
    request.set_message(long_message);
    asphrdaemon::SendMessageResponse response;
    auto status = friends.at(0).rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // execute a round
  unordered_set<int> to_receive = {1, 2, 3};

  for (auto round = 0; round < 3; round++) {
    // first round, none!
    for (size_t i = 0; i < friends.size(); i++) {
      friends.at(i).t->retrieve();
      friends.at(i).t->send();
    }

    for (auto i : to_receive) {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status =
          friends.at(i).rpc->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
    }

    // second round, one!
    for (size_t i = 0; i < friends.size(); i++) {
      friends.at(i).t->retrieve();
      friends.at(i).t->send();
    }

    unordered_set<int> new_to_receive;

    for (auto i : to_receive) {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status =
          friends.at(i).rpc->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      if (response.messages_size() > 1) {
        EXPECT_EQ(response.messages_size(), 2);
        EXPECT_EQ(response.messages(0).from_unique_name(),
                  friends.at(0).unique_name);
        EXPECT_EQ(response.messages(0).message(), long_message);
      } else {
        new_to_receive.insert(i);
      }
    }

    to_receive = new_to_receive;
  }

  EXPECT_EQ(to_receive.size(), 0);
};

}  // namespace
}  // namespace asphr::testing