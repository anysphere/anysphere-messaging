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

TEST_F(MultipleFriendsTest, SendMultipleMessages) {
  ResetStub();

  FriendTestingInfo friend1, friend2;
  std::tie(friend1, friend2) = generate_two_friends();

  {
    SendMessageRequest request;
    request.add_unique_name(friend2.unique_name);
    request.set_message("hello from 1 to 2");
    asphrdaemon::SendMessageResponse response;
    auto status = friend1.rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    SendMessageRequest request;
    request.add_unique_name(friend2.unique_name);
    request.set_message("hello from 1 to 2, again!!!! :0");
    asphrdaemon::SendMessageResponse response;
    auto status = friend1.rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  friend1.t->retrieve();
  friend1.t->send();

  friend2.t->retrieve();
  friend2.t->send();

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend1.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2);  // +1 for invitation message.
    EXPECT_EQ(response.messages(0).from_unique_name(), friend1.unique_name);
    EXPECT_EQ(response.messages(0).message(), "hello from 1 to 2");
  }

  friend1.t->retrieve();
  friend1.t->send();

  friend2.t->retrieve();
  friend2.t->send();

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend1.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 3);  // +1 for invitation message.
    EXPECT_EQ(response.messages(0).from_unique_name(), friend1.unique_name);
    EXPECT_EQ(response.messages(0).message(),
              "hello from 1 to 2, again!!!! :0");
  }
};

}  // namespace
}  // namespace asphr::testing