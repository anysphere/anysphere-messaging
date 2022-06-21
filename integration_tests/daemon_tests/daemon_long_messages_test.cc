#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, SendLongMessage) {
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

  // these messages are too big to be in a single message, but small enough to
  // be contained in two messages.
  auto m1 = string("");
  auto m2 = string("");
  for (size_t i = 0; i < 3 * GUARANTEED_SINGLE_MESSAGE_SIZE / 2; i++) {
    absl::StrAppend(&m1, "a");
    absl::StrAppend(&m2, "b");
  }

  {
    SendMessageRequest request;
    request.set_unique_name("user2");
    request.set_message(m1);
    asphrdaemon::SendMessageResponse response;
    auto status = rpc1->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    SendMessageRequest request;
    request.set_unique_name("user1");
    request.set_message(m2);
    asphrdaemon::SendMessageResponse response;
    auto status = rpc2->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    t1->retrieve();
    t1->send();
  }

  {
    t2->retrieve();
    t2->send();
  }

  { t1->retrieve(); }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc1->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc2->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  {
    t1->retrieve();
    t1->send();
  }

  {
    t2->retrieve();
    t2->send();
  }

  { t1->retrieve(); }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc1->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).m().unique_name(), "user2");
    EXPECT_EQ(response.messages(0).m().message(), m2);
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpc2->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).m().unique_name(), "user1");
    EXPECT_EQ(response.messages(0).m().message(), m1);
  }
};

TEST_F(DaemonRpcTest, SendLongMessagePersistence) {
  ResetStub();

  auto config1_file = generateTempFile();
  auto config2_file = generateTempFile();

  // these messages are too big to be in a single message, but small enough to
  // be contained in two messages.
  auto m1 = string("");
  auto m2 = string("");
  for (size_t i = 0; i < 3 * GUARANTEED_SINGLE_MESSAGE_SIZE / 2; i++) {
    absl::StrAppend(&m1, "a");
    absl::StrAppend(&m2, "b");
  }

  {
    auto [G1, rpc1, t1] = gen_person(config1_file);
    auto [G2, rpc2, t2] = gen_person(config2_file);

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
      request.set_message(m1);
      asphrdaemon::SendMessageResponse response;
      auto status = rpc1->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      SendMessageRequest request;
      request.set_unique_name("user1");
      request.set_message(m2);
      asphrdaemon::SendMessageResponse response;
      auto status = rpc2->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      t1->retrieve();
      t1->send();
    }

    {
      t2->retrieve();
      t2->send();
    }

    { t1->retrieve(); }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc1->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 0);
    }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc2->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 0);
    }
  }

  {
    auto [G1, rpc1, t1] = gen_person(config1_file);
    auto [G2, rpc2, t2] = gen_person(config2_file);

    {
      t1->retrieve();
      t1->send();
    }

    {
      t2->retrieve();
      t2->send();
    }

    { t1->retrieve(); }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc1->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).m().unique_name(), "user2");
      EXPECT_EQ(response.messages(0).m().message(), m2);
    }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc2->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).m().unique_name(), "user1");
      EXPECT_EQ(response.messages(0).m().message(), m1);
    }
  }
};

}  // namespace
}  // namespace asphr::testing