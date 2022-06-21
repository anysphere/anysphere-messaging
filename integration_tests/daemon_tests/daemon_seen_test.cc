#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, SeenMessage) {
  ResetStub();

  auto [G1, rpc1, t1] = gen_person();
  auto [G2, rpc2, t2] = gen_person();

  {
    RegisterUserRequest request;
    request.set_name("user1local");
    request.set_beta_key("asphr_magic");
    RegisterUserResponse response;
    rpc1->RegisterUser(nullptr, &request, &response);
  }
  {
    RegisterUserRequest request;
    request.set_name("user2local");
    request.set_beta_key("asphr_magic");
    RegisterUserResponse response;
    rpc2->RegisterUser(nullptr, &request, &response);
  }

  string user1_key;
  string user2_key;

  {
    GenerateFriendKeyRequest request;
    request.set_unique_name("user2");
    GenerateFriendKeyResponse response;
    auto status = rpc1->GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user1_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_unique_name("user1");
    GenerateFriendKeyResponse response;
    auto status = rpc2->GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user2_key = response.key();
  }

  cout << "user1_key: " << user1_key << endl;
  cout << "user2_key: " << user2_key << endl;

  {
    AddFriendRequest request;
    request.set_unique_name("user2");
    request.set_key(user2_key);
    AddFriendResponse response;
    auto status = rpc1->AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    AddFriendRequest request;
    request.set_unique_name("user1");
    request.set_key(user1_key);
    AddFriendResponse response;
    auto status = rpc2->AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    SendMessageRequest request;
    request.set_unique_name("user2");
    request.set_message("hello from 1 to 2");
    asphrdaemon::SendMessageResponse response;
    auto status = rpc1->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    SendMessageRequest request;
    request.set_unique_name("user2");
    request.set_message("hello from 1 to 2, again!!!! :0");
    asphrdaemon::SendMessageResponse response;
    auto status = rpc1->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // retrieve-send is how messages are propagated!
  t1->retrieve();
  t1->send();

  t2->retrieve();
  t2->send();

  // 1 can impossibly receive anything
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc1->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  // 2 should have received the first message!
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc2->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).m().unique_name(), "user1");
    EXPECT_EQ(response.messages(0).m().message(), "hello from 1 to 2");
  }

  // 2 has sent the ACK for the first message, so 1 should safely send the next
  // message
  t1->retrieve();
  t1->send();

  t2->retrieve();
  t2->send();

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc1->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  int first_message_id;

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc2->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2);
    EXPECT_EQ(response.messages(0).m().unique_name(), "user1");
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
    EXPECT_EQ(response.messages(1).m().unique_name(), "user1");
    EXPECT_EQ(response.messages(1).m().message(), "hello from 1 to 2");
    first_message_id = response.messages(1).m().id();
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::NEW);
    GetMessagesResponse response;
    auto status = rpc2->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2);
    EXPECT_EQ(response.messages(0).m().unique_name(), "user1");
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
    EXPECT_EQ(response.messages(1).m().unique_name(), "user1");
    EXPECT_EQ(response.messages(1).m().message(), "hello from 1 to 2");
  }

  // now see the message!
  {
    MessageSeenRequest request;
    MessageSeenResponse response;
    request.set_id(first_message_id);
    auto status = rpc2->MessageSeen(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // all messages should be same, new messages should be different!
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc2->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2);
    EXPECT_EQ(response.messages(0).m().unique_name(), "user1");
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
    EXPECT_EQ(response.messages(1).m().unique_name(), "user1");
    EXPECT_EQ(response.messages(1).m().message(), "hello from 1 to 2");
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::NEW);
    GetMessagesResponse response;
    auto status = rpc2->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).m().unique_name(), "user1");
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
  }
};

}  // namespace
}  // namespace asphr::testing