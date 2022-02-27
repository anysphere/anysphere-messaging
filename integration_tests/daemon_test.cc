#include <gtest/gtest.h>

#include "asphr/asphr.hpp"
#include "daemon/daemon_rpc.hpp"
#include "daemon/transmitter.hpp"
#include "google/protobuf/util/time_util.h"
#include "server/pir/fast_pir/fastpir.hpp"
#include "server/src/server_rpc.hpp"
#include "test_helpers.hpp"

/**
 * TODO: have a multiple rounds test.
 *
 *
 *
 **/

using namespace asphrdaemon;

namespace asphr::testing {
namespace {

class DaemonRpcTest : public ::testing::Test {
  using ServerRpc = ServerRpc<FastPIR, AccountManagerInMemory>;

 protected:
  DaemonRpcTest() : service_(gen_server_rpc()) {}

  auto generateTempFile() -> string {
    auto config_file_address = "TMPTMPTMP_config" +
                               std::to_string(config_file_addresses_.size()) +
                               ".json";
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    auto address = string(cwd) + "/" + config_file_address;
    config_file_addresses_.push_back(address);
    return address;
  }

  auto generateTempDir() -> std::filesystem::path {
    auto tmp_dir = "TMPTMPTMP_dirs" + std::to_string(temp_dirs_.size());
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    auto address = std::filesystem::path(cwd) / tmp_dir;
    std::filesystem::create_directory(address);
    temp_dirs_.push_back(address);
    return address;
  }

  void SetUp() override {
    int port = 43427;
    server_address_ << "localhost:" << port;
    // Setup server
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address_.str(),
                             grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    server_ = builder.BuildAndStart();
  }

  void TearDown() override {
    server_->Shutdown();

    for (auto f : config_file_addresses_) {
      if (remove(f.c_str()) != 0) {
        cerr << "Error deleting file";
      } else {
        cout << "File successfully deleted\n";
      }
    }
    for (auto f : temp_dirs_) {
      if (std::filesystem::remove_all(f) != 0) {
        cerr << "Error deleting file";
      } else {
        cout << "File successfully deleted\n";
      }
    }
  }

  void ResetStub() {
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(
        server_address_.str(), grpc::InsecureChannelCredentials());
    stub_ = asphrserver::Server::NewStub(channel);
  }

  std::shared_ptr<asphrserver::Server::Stub> stub_;
  std::unique_ptr<grpc::Server> server_;
  std::ostringstream server_address_;
  ServerRpc service_;
  vector<string> config_file_addresses_;
  vector<std::filesystem::path> temp_dirs_;
};

TEST_F(DaemonRpcTest, GetFriendListUnauthenticated) {
  ResetStub();
  auto crypto = gen_crypto();
  auto config = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore = gen_msgstore(config);
  DaemonRpc rpc(crypto, config, stub_, msgstore);

  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    auto status = rpc.GetFriendList(nullptr, &request, &response);
    EXPECT_FALSE(status.ok());
  }
};

TEST_F(DaemonRpcTest, KillDaemon) {
  ResetStub();
  auto crypto = gen_crypto();
  auto config = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore = gen_msgstore(config);
  DaemonRpc rpc(crypto, config, stub_, msgstore);

  auto killed = config->wait_until_killed_or_seconds(0);
  EXPECT_FALSE(killed);

  {
    KillRequest request;
    KillResponse response;
    rpc.Kill(nullptr, &request, &response);
  }

  killed = config->wait_until_killed_or_seconds(1000);
  EXPECT_TRUE(killed);
}

TEST_F(DaemonRpcTest, LoadAndUnloadConfig) {
  ResetStub();
  auto config_file_address = generateTempFile();

  {
    auto crypto = gen_crypto();
    auto config = gen_config(string(generateTempDir()), config_file_address);
    auto msgstore = gen_msgstore(config);
    DaemonRpc rpc(crypto, config, stub_, msgstore);

    {
      RegisterUserRequest request;
      request.set_name("test");
      request.set_beta_key("asphr_magic");
      RegisterUserResponse response;
      auto status = rpc.RegisterUser(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }
  }

  {
    // re-create config from the file!
    auto config = make_shared<Config>(config_file_address);
    auto crypto = gen_crypto();
    auto msgstore = gen_msgstore(config);
    DaemonRpc rpc(crypto, config, stub_, msgstore);

    {
      GetFriendListRequest request;
      GetFriendListResponse response;
      auto status = rpc.GetFriendList(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.friend_infos_size(), 0);
    }
  }
}

TEST_F(DaemonRpcTest, LoadAndUnloadConfigAndReceiveHalfFriend) {
  ResetStub();
  auto config_file_address = generateTempFile();

  {
    auto crypto1 = gen_crypto();
    auto config1 = gen_config(string(generateTempDir()), config_file_address);
    auto msgstore1 = gen_msgstore(config1);
    DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
    auto crypto2 = gen_crypto();
    auto config2 = gen_config(string(generateTempDir()), generateTempFile());
    auto msgstore2 = gen_msgstore(config2);
    DaemonRpc rpc2(crypto2, config2, stub_, msgstore2);

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

    {
      GenerateFriendKeyRequest request;
      request.set_name("user2");
      GenerateFriendKeyResponse response;
      auto status = rpc1.GenerateFriendKey(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_GT(response.key().size(), 0);
    }

    {
      GenerateFriendKeyRequest request;
      request.set_name("user1");
      GenerateFriendKeyResponse response;
      auto status = rpc2.GenerateFriendKey(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_GT(response.key().size(), 0);
    }
  }

  {
    // re-create config from the file!
    auto config = make_shared<Config>(config_file_address);
    auto crypto = gen_crypto();
    auto msgstore = gen_msgstore(config);
    Transmitter t(crypto, config, stub_, msgstore);

    t.retrieve_messages();
    t.send_messages();

    t.retrieve_messages();
    t.send_messages();
  }
}

TEST_F(DaemonRpcTest, LoadAndUnloadConfigAndReceive) {
  ResetStub();
  auto config_file_address1 = generateTempFile();
  auto config_file_address2 = generateTempFile();

  {
    auto crypto1 = gen_crypto();
    auto config1 = gen_config(string(generateTempDir()), config_file_address1);
    auto msgstore1 = gen_msgstore(config1);
    DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
    auto crypto2 = gen_crypto();
    auto config2 = gen_config(string(generateTempDir()), config_file_address2);
    auto msgstore2 = gen_msgstore(config2);
    DaemonRpc rpc2(crypto2, config2, stub_, msgstore2);

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
  }

  {
    // re-create config from the file!
    auto crypto1 = gen_crypto();
    auto config1 = make_shared<Config>(config_file_address1);
    auto msgstore1 = gen_msgstore(config1);
    DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
    Transmitter t1(crypto1, config1, stub_, msgstore1);

    auto crypto2 = gen_crypto();
    auto config2 = make_shared<Config>(config_file_address2);
    auto msgstore2 = gen_msgstore(config2);
    DaemonRpc rpc2(crypto2, config2, stub_, msgstore2);
    Transmitter t2(crypto2, config2, stub_, msgstore2);

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
}

TEST_F(DaemonRpcTest, Register) {
  ResetStub();
  auto crypto = gen_crypto();
  auto config = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore = gen_msgstore(config);
  DaemonRpc rpc(crypto, config, stub_, msgstore);

  {
    RegisterUserRequest request;
    request.set_name("test");
    request.set_beta_key("asphr_magic");
    RegisterUserResponse response;
    auto status = rpc.RegisterUser(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }
};

TEST_F(DaemonRpcTest, GetFriendList) {
  ResetStub();
  auto crypto = gen_crypto();
  auto config = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore = gen_msgstore(config);
  DaemonRpc rpc(crypto, config, stub_, msgstore);

  {
    RegisterUserRequest request;
    request.set_name("test");
    request.set_beta_key("asphr_magic");
    RegisterUserResponse response;
    rpc.RegisterUser(nullptr, &request, &response);
  }

  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    auto status = rpc.GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.friend_infos_size(), 0);
  }
};

TEST_F(DaemonRpcTest, GenerateFriendKey) {
  ResetStub();
  auto crypto = gen_crypto();
  auto config = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore = gen_msgstore(config);
  DaemonRpc rpc(crypto, config, stub_, msgstore);

  {
    RegisterUserRequest request;
    request.set_name("test");
    request.set_beta_key("asphr_magic");
    RegisterUserResponse response;
    rpc.RegisterUser(nullptr, &request, &response);
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name("friend_name");
    GenerateFriendKeyResponse response;
    auto status = rpc.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
  }
};

TEST_F(DaemonRpcTest, AddFriend) {
  ResetStub();

  auto crypto1 = gen_crypto();
  auto config1 = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore1 = gen_msgstore(config1);
  DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
  auto crypto2 = gen_crypto();
  auto config2 = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore2 = gen_msgstore(config2);
  DaemonRpc rpc2(crypto2, config2, stub_, msgstore2);

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
};

TEST_F(DaemonRpcTest, AddFriendAndCheckFriendList) {
  ResetStub();

  auto crypto1 = gen_crypto();
  auto config1 = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore1 = gen_msgstore(config1);
  DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
  auto crypto2 = gen_crypto();
  auto config2 = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore2 = gen_msgstore(config2);
  DaemonRpc rpc2(crypto2, config2, stub_, msgstore2);

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

  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    auto status = rpc1.GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.friend_infos_size(), 0);
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

  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    auto status = rpc1.GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.friend_infos_size(), 1);
    EXPECT_EQ(response.friend_infos(0).name(), "user2");
    EXPECT_EQ(response.friend_infos(0).enabled(), false);
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
    GetFriendListRequest request;
    GetFriendListResponse response;
    auto status = rpc1.GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.friend_infos_size(), 1);
    EXPECT_EQ(response.friend_infos(0).name(), "user2");
    EXPECT_EQ(response.friend_infos(0).enabled(), true);
  }
};

TEST_F(DaemonRpcTest, SendMessage) {
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
};

TEST_F(DaemonRpcTest, SendMultipleMessages) {
  using TimeUtil = google::protobuf::util::TimeUtil;
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

  // retrieve-send is how messages are propagated!
  t1.retrieve_messages();
  t1.send_messages();

  t2.retrieve_messages();
  t2.send_messages();

  // 1 can impossibly receive anything
  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc1.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  // 2 should have received the first message!
  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc2.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).from(), "user1");
    EXPECT_EQ(response.messages(0).m().message(), "hello from 1 to 2");
  }

  // 2 has sent the ACK for the first message, so 1 should safely send the next
  // message
  t1.retrieve_messages();
  t1.send_messages();

  t2.retrieve_messages();
  t2.send_messages();

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
    EXPECT_EQ(response.messages_size(), 2);
    EXPECT_EQ(response.messages(0).from(), "user1");
    cout << "message 1: " << response.messages(0).m().message() << endl;

    cout << "message 1 time: "
         << TimeUtil::ToString(response.messages(0).received_timestamp())
         << endl;
    cout << "message 2: " << response.messages(1).m().message() << endl;
    cout << "message 2 time: "
         << TimeUtil::ToString(response.messages(1).received_timestamp())
         << endl;
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
  }
};

TEST_F(DaemonRpcTest, SendMultipleMessagesInBothDirections) {
  using TimeUtil = google::protobuf::util::TimeUtil;
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

  {
    SendMessageRequest request;
    request.set_name("user1");
    request.set_message("hello from 2 to 1");
    asphrdaemon::SendMessageResponse response;
    auto status = rpc2.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // retrieve-send is how messages are propagated!
  t1.retrieve_messages();
  t1.send_messages();

  t2.retrieve_messages();
  t2.send_messages();

  // 1 can impossibly receive anything!!
  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc1.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  // 2 should have received the first message!
  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc2.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).from(), "user1");
    EXPECT_EQ(response.messages(0).m().message(), "hello from 1 to 2");
  }

  // 2 has sent the ACK for the first message, so 1 should safely send the next
  // message
  t1.retrieve_messages();
  t1.send_messages();

  t2.retrieve_messages();
  t2.send_messages();

  // 1 should have received the first message by now
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
    EXPECT_EQ(response.messages_size(), 2);
    EXPECT_EQ(response.messages(0).from(), "user1");
    cout << "message 1: " << response.messages(0).m().message() << endl;

    cout << "message 1 time: "
         << TimeUtil::ToString(response.messages(0).received_timestamp())
         << endl;
    cout << "message 2: " << response.messages(1).m().message() << endl;
    cout << "message 2 time: "
         << TimeUtil::ToString(response.messages(1).received_timestamp())
         << endl;
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
  }
};

TEST_F(DaemonRpcTest, SeenMessage) {
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

  // retrieve-send is how messages are propagated!
  t1.retrieve_messages();
  t1.send_messages();

  t2.retrieve_messages();
  t2.send_messages();

  // 1 can impossibly receive anything
  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc1.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  // 2 should have received the first message!
  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc2.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).from(), "user1");
    EXPECT_EQ(response.messages(0).m().message(), "hello from 1 to 2");
  }

  // 2 has sent the ACK for the first message, so 1 should safely send the next
  // message
  t1.retrieve_messages();
  t1.send_messages();

  t2.retrieve_messages();
  t2.send_messages();

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc1.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  string first_message_id;

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc2.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2);
    EXPECT_EQ(response.messages(0).from(), "user1");
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
    EXPECT_EQ(response.messages(1).from(), "user1");
    EXPECT_EQ(response.messages(1).m().message(), "hello from 1 to 2");
    first_message_id = response.messages(1).m().id();
  }

  {
    GetNewMessagesRequest request;
    GetNewMessagesResponse response;
    auto status = rpc2.GetNewMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2);
    EXPECT_EQ(response.messages(0).from(), "user1");
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
    EXPECT_EQ(response.messages(1).from(), "user1");
    EXPECT_EQ(response.messages(1).m().message(), "hello from 1 to 2");
  }

  // now see the message!
  {
    MessageSeenRequest request;
    MessageSeenResponse response;
    request.set_id(first_message_id);
    auto status = rpc2.MessageSeen(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // all messages should be same, new messages should be different!
  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc2.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2);
    EXPECT_EQ(response.messages(0).from(), "user1");
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
    EXPECT_EQ(response.messages(1).from(), "user1");
    EXPECT_EQ(response.messages(1).m().message(), "hello from 1 to 2");
  }

  {
    GetNewMessagesRequest request;
    GetNewMessagesResponse response;
    auto status = rpc2.GetNewMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).from(), "user1");
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
  }
};

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