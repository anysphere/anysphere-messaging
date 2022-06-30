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

TEST_F(MultipleFriendsTest, SendThreeMessages) {
  ResetStub();

  auto friends = generate_friends_from_list_of_pairs(4, {
                                                            {0, 1},
                                                            {0, 2},
                                                            {0, 3},
                                                        });

  // user 1 sends message to 2-4
  for (size_t i = 1; i < friends.size(); i++) {
    SendMessageRequest request;
    request.set_unique_name(friends.at(i).unique_name);
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
        EXPECT_EQ(response.messages(0).m().unique_name(),
                  friends.at(0).unique_name);
        EXPECT_EQ(response.messages(0).m().message(),
                  asphr::StrCat("hello from 0 to ", i));
        to_receive.erase(i);
      }
    }
  }

  EXPECT_EQ(to_receive.size(), 0);
};

}  // namespace
}  // namespace asphr::testing