#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, MsgstorePersistence) {
  ResetStub();

  auto config1_file = generateTempFile();
  auto config2_file = generateTempFile();

  {
    auto crypto1 = gen_crypto();
    auto config1 = gen_config(string(generateTempDir()), config1_file);
    auto msgstore1 = gen_msgstore(config1);
    DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
    Transmitter t1(crypto1, config1, stub_, msgstore1);
    auto crypto2 = gen_crypto();
    auto config2 = gen_config(string(generateTempDir()), config2_file);
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
      request.set_name("user1");
      request.set_message("hello from 2 to 1");
      asphrdaemon::SendMessageResponse response;
      auto status = rpc2.SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      t1.send_messages();
      t2.send_messages();
    }

    {
      t1.retrieve_messages();
      cout << "-----------------------------" << endl;
      t2.retrieve_messages();
    }

    {
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpc1.GetAllMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).from(), "user2");
      EXPECT_EQ(response.messages(0).m().message(), "hello from 2 to 1");
    }

    {
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpc2.GetAllMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).from(), "user1");
      EXPECT_EQ(response.messages(0).m().message(), "hello from 1 to 2");
    }
  }

  // reload from file! and it should still work pls
  {
    auto crypto1 = gen_crypto();
    auto config1 = make_shared<Config>(config1_file);
    auto msgstore1 = gen_msgstore(config1);
    DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
    Transmitter t1(crypto1, config1, stub_, msgstore1);
    auto crypto2 = gen_crypto();
    auto config2 = make_shared<Config>(config2_file);
    auto msgstore2 = gen_msgstore(config2);
    DaemonRpc rpc2(crypto2, config2, stub_, msgstore2);
    Transmitter t2(crypto2, config2, stub_, msgstore2);

    {
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpc1.GetAllMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).from(), "user2");
      EXPECT_EQ(response.messages(0).m().message(), "hello from 2 to 1");
    }

    {
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpc2.GetAllMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).from(), "user1");
      EXPECT_EQ(response.messages(0).m().message(), "hello from 1 to 2");
    }
  }
};

TEST_F(DaemonRpcTest, OutboxPersistence) {
  ResetStub();

  auto config1_file = generateTempFile();
  auto config2_file = generateTempFile();

  {
    auto crypto1 = gen_crypto();
    auto config1 = gen_config(string(generateTempDir()), config1_file);
    auto msgstore1 = gen_msgstore(config1);
    DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
    Transmitter t1(crypto1, config1, stub_, msgstore1);
    auto crypto2 = gen_crypto();
    auto config2 = gen_config(string(generateTempDir()), config2_file);
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
      request.set_name("user1");
      request.set_message("hello from 2 to 1");
      asphrdaemon::SendMessageResponse response;
      auto status = rpc2.SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpc1.GetAllMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 0);
    }

    {
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpc2.GetAllMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 0);
    }
  }

  // reload from file! and it should still work pls
  {
    auto crypto1 = gen_crypto();
    auto config1 = make_shared<Config>(config1_file);
    auto msgstore1 = gen_msgstore(config1);
    DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
    Transmitter t1(crypto1, config1, stub_, msgstore1);
    auto crypto2 = gen_crypto();
    auto config2 = make_shared<Config>(config2_file);
    auto msgstore2 = gen_msgstore(config2);
    DaemonRpc rpc2(crypto2, config2, stub_, msgstore2);
    Transmitter t2(crypto2, config2, stub_, msgstore2);

    {
      SendMessageRequest request;
      request.set_name("user2");
      request.set_message("hello from 1 to 2, again!");
      asphrdaemon::SendMessageResponse response;
      auto status = rpc1.SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      SendMessageRequest request;
      request.set_name("user1");
      request.set_message("hello from 2 to 1, again!");
      asphrdaemon::SendMessageResponse response;
      auto status = rpc2.SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      t1.retrieve_messages();
      t1.send_messages();
    }

    {
      t2.retrieve_messages();
      t2.send_messages();
    }

    { t1.retrieve_messages(); }

    {
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpc1.GetAllMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).from(), "user2");
      EXPECT_EQ(response.messages(0).m().message(), "hello from 2 to 1");
    }

    {
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpc2.GetAllMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).from(), "user1");
      EXPECT_EQ(response.messages(0).m().message(), "hello from 1 to 2");
    }
  }

  {
    auto crypto1 = gen_crypto();
    auto config1 = make_shared<Config>(config1_file);
    auto msgstore1 = gen_msgstore(config1);
    DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
    Transmitter t1(crypto1, config1, stub_, msgstore1);
    auto crypto2 = gen_crypto();
    auto config2 = make_shared<Config>(config2_file);
    auto msgstore2 = gen_msgstore(config2);
    DaemonRpc rpc2(crypto2, config2, stub_, msgstore2);
    Transmitter t2(crypto2, config2, stub_, msgstore2);

    {
      t1.retrieve_messages();
      t1.send_messages();
    }

    {
      t2.retrieve_messages();
      t2.send_messages();
    }

    { t1.retrieve_messages(); }

    {
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpc1.GetAllMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 2);
      EXPECT_EQ(response.messages(0).from(), "user2");
      EXPECT_EQ(response.messages(0).m().message(),
                "hello from 2 to 1, again!");
      EXPECT_EQ(response.messages(1).from(), "user2");
      EXPECT_EQ(response.messages(1).m().message(), "hello from 2 to 1");
    }

    {
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpc2.GetAllMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 2);
      EXPECT_EQ(response.messages(0).from(), "user1");
      EXPECT_EQ(response.messages(0).m().message(),
                "hello from 1 to 2, again!");
      EXPECT_EQ(response.messages(1).from(), "user1");
      EXPECT_EQ(response.messages(1).m().message(), "hello from 1 to 2");
    }
  }
};

}  // namespace
}  // namespace asphr::testing