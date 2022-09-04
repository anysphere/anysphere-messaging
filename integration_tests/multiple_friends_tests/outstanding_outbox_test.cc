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

TEST_F(MultipleFriendsTest, ReceiveMessageEvenIfOutstandingOutboxMessage) {
  ResetStub();

  auto friends = generate_friends_from_list_of_pairs(3, {
                                                            {0, 1},
                                                            {0, 2},
                                                        });

  // user 1 sends message to 2, never to be received because 2 is offline
  {
    SendMessageRequest request;
    request.add_unique_name(friends.at(1).unique_name);
    request.set_message(absl::StrCat("hello from 0 to ", 1));
    asphrdaemon::SendMessageResponse response;
    auto status = friends.at(0).rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    // user 3 sends a message to user 1, which should eventually be received!
    SendMessageRequest request;
    request.add_unique_name(friends.at(0).unique_name);
    request.set_message(absl::StrCat("hello from 2 to ", 0));
    asphrdaemon::SendMessageResponse response;
    auto status = friends.at(2).rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // execute 12 rounds. this may fail!! and in that case we just want to re-run
  // the test
  const int kNumRounds = 12;
  for (auto round = 0; round < kNumRounds; round++) {
    for (size_t i = 0; i < friends.size(); i++) {
      // user 2 is offline!
      if (i == 1) {
        continue;
      }
      friends.at(i).t->retrieve();
      friends.at(i).t->send();
    }
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friends.at(0).rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    if (response.messages_size() > 0) {
      break;
    }
  }
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friends.at(0).rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).from_unique_name(),
              friends.at(2).unique_name);
    EXPECT_EQ(response.messages(0).message(),
              asphr::StrCat("hello from 2 to ", 0));
  }
};

}  // namespace
}  // namespace asphr::testing