#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, MsgstorePersistence) {
  ResetStub();

  auto config1_file = generateTempFile();
  auto config2_file = generateTempFile();

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
      request.set_unique_name("user1");
      request.set_message("hello from 2 to 1");
      asphrdaemon::SendMessageResponse response;
      auto status = rpc2->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      t1->send();
      t2->send();
    }

    {
      t1->retrieve();
      cout << "-----------------------------" << endl;
      t2->retrieve();
    }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc1->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).m().unique_name(), "user2");
      EXPECT_EQ(response.messages(0).m().message(), "hello from 2 to 1");
    }

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
  }

  // reload from file! and it should still work pls
  {
    auto [G1, rpc1, t1] = gen_person(config1_file);
    auto [G2, rpc2, t2] = gen_person(config2_file);

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc1->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).m().unique_name(), "user2");
      EXPECT_EQ(response.messages(0).m().message(), "hello from 2 to 1");
    }

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
  }
};

TEST_F(DaemonRpcTest, OutboxPersistence) {
  ResetStub();

  auto config1_file = generateTempFile();
  auto config2_file = generateTempFile();

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
      request.set_unique_name("user1");
      request.set_message("hello from 2 to 1");
      asphrdaemon::SendMessageResponse response;
      auto status = rpc2->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

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

  // reload from file! and it should still work pls
  {
    auto [g1, rpc1, t1] = gen_person(config1_file);
    auto [g2, rpc2, t2] = gen_person(config2_file);

    {
      SendMessageRequest request;
      request.set_unique_name("user2");
      request.set_message("hello from 1 to 2, again!");
      asphrdaemon::SendMessageResponse response;
      auto status = rpc1->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      SendMessageRequest request;
      request.set_unique_name("user1");
      request.set_message("hello from 2 to 1, again!");
      asphrdaemon::SendMessageResponse response;
      auto status = rpc2->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      t1->retrieve();
      t1->send();
    }

    g1->db->dump();

    {
      t2->retrieve();
      t2->send();
    }

    { t1->retrieve(); }

    g1->db->dump();

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc1->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).m().unique_name(), "user2");
      EXPECT_EQ(response.messages(0).m().message(), "hello from 2 to 1");
    }

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
  }

  {
    auto [g1, rpc1, t1] = gen_person(config1_file);
    auto [g2, rpc2, t2] = gen_person(config2_file);

    {
      t1->retrieve();
      t1->send();
    }

    g1->db->dump();

    {
      t2->retrieve();
      t2->send();
    }

    { t1->retrieve(); }

    g1->db->dump();

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc1->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 2);
      EXPECT_EQ(response.messages(0).m().unique_name(), "user2");
      EXPECT_EQ(response.messages(0).m().message(),
                "hello from 2 to 1, again!");
      EXPECT_EQ(response.messages(1).m().unique_name(), "user2");
      EXPECT_EQ(response.messages(1).m().message(), "hello from 2 to 1");
    }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc2->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 2);
      EXPECT_EQ(response.messages(0).m().unique_name(), "user1");
      EXPECT_EQ(response.messages(0).m().message(),
                "hello from 1 to 2, again!");
      EXPECT_EQ(response.messages(1).m().unique_name(), "user1");
      EXPECT_EQ(response.messages(1).m().message(), "hello from 1 to 2");
    }
  }
};

}  // namespace
}  // namespace asphr::testing