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

  auto crypto1 = gen_crypto();
  auto config1 = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore1 = gen_msgstore(config1);
  DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
  Transmitter t1(crypto1, config1, stub_, msgstore1);
  auto crypto2 = gen_crypto();
  auto config2 = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore2 = gen_msgstore(config2);
  DaemonRpc rpc2(crypto2, config2, stub_, msgstore2);
  Transmitter t2(crypto2, config2, stub_, msgstore2);

  {
    RegisterUserRequest request;
    request.set_name("user1local");
    request.set_beta_key("asphr_magic");
    RegisterUserResponse response;
    rpc1.RegisterUser(nullptr, &request, &response);
  }
  {
    RegisterUserRequest request;
    request.set_name("user2local");
    request.set_beta_key("asphr_magic");
    RegisterUserResponse response;
    rpc2.RegisterUser(nullptr, &request, &response);
  }

  string user1_key;
  string user2_key;

  {
    GenerateFriendKeyRequest request;
    request.set_name("user2");
    GenerateFriendKeyResponse response;
    auto status = rpc1.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user1_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name("user1");
    GenerateFriendKeyResponse response;
    auto status = rpc2.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user2_key = response.key();
  }

  cout << "user1_key: " << user1_key << endl;
  cout << "user2_key: " << user2_key << endl;

  {
    AddFriendRequest request;
    request.set_name("user2");
    request.set_key(user2_key);
    AddFriendResponse response;
    auto status = rpc1.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    AddFriendRequest request;
    request.set_name("user1");
    request.set_key(user1_key);
    AddFriendResponse response;
    auto status = rpc2.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    SendMessageRequest request;
    request.set_name("user2");
    request.set_message("hello from 1 to 2");
    asphrdaemon::SendMessageResponse response;
    auto status = rpc1.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    SendMessageRequest request;
    request.set_name("user2");
    request.set_message("hello from 1 to 2, again!!!! :0");
    asphrdaemon::SendMessageResponse response;
    auto status = rpc1.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  t1.retrieve_messages();
  t1.send_messages();

  t2.retrieve_messages();
  t2.send_messages();

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc1.GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc2.GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).from(), "user1");
    EXPECT_EQ(response.messages(0).m().message(), "hello from 1 to 2");
  }

  t1.retrieve_messages();
  t1.send_messages();

  t2.retrieve_messages();
  t2.send_messages();

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc1.GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc2.GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2);
    EXPECT_EQ(response.messages(0).from(), "user1");
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
  }
};

}  // namespace
}  // namespace asphr::testing