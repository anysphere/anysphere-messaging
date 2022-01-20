#include <gtest/gtest.h>

#include "asphr/asphr.hpp"
#include "client/daemon/daemon_rpc.hpp"
#include "client/daemon/transmitter.hpp"
#include "server/pir/fast_pir/fastpir.hpp"
#include "server/src/server_rpc.hpp"

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

auto gen_crypto() -> Crypto {
  Crypto crypto;
  return crypto;
}

auto gen_config(string tmp_dir, string tmp_file) -> Config {
  json config_json = {
      {"has_registered", false}, {"friends", {}}, {"data_dir", tmp_dir}};
  Config config(config_json, tmp_file);
  return config;
}

auto gen_server_rpc() {
  FastPIR pir;
  FastPIR pir_acks;
  AccountManagerInMemory account_manager("in_memory");
  return ServerRpc(std::move(pir), std::move(pir_acks),
                   std::move(account_manager));
}

namespace asphr::testing {
namespace {

class MultipleFriendsTest : public ::testing::Test {
  using ServerRpc = ServerRpc<FastPIR, AccountManagerInMemory>;

 protected:
  MultipleFriendsTest() : service_(gen_server_rpc()) {}

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
    int port = 86651;
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

  std::unique_ptr<asphrserver::Server::Stub> stub_;
  std::unique_ptr<grpc::Server> server_;
  std::ostringstream server_address_;
  ServerRpc service_;
  vector<string> config_file_addresses_;
  vector<std::filesystem::path> temp_dirs_;
};

TEST_F(MultipleFriendsTest, SendThreeMessages) {
  ResetStub();

  auto crypto1 = gen_crypto();
  auto config1 = gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc1(crypto1, config1, stub_);
  Transmitter t1(crypto1, config1, stub_);

  auto friend_crypto2 = gen_crypto();
  auto friend_config2 =
      gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc2(friend_crypto2, friend_config2, stub_);
  Transmitter t2(friend_crypto2, friend_config2, stub_);

  auto friend_crypto3 = gen_crypto();
  auto friend_config3 =
      gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc3(friend_crypto3, friend_config3, stub_);
  Transmitter t3(friend_crypto3, friend_config3, stub_);

  auto friend_crypto4 = gen_crypto();
  auto friend_config4 =
      gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc4(friend_crypto4, friend_config4, stub_);
  Transmitter t4(friend_crypto4, friend_config4, stub_);

  {
    RegisterUserRequest request;
    request.set_name("user1local");
    RegisterUserResponse response;
    rpc1.RegisterUser(nullptr, &request, &response);
  }
  {
    RegisterUserRequest request;
    request.set_name("user2local");
    RegisterUserResponse response;
    rpc2.RegisterUser(nullptr, &request, &response);
  }

  {
    RegisterUserRequest request;
    request.set_name("user3local");
    RegisterUserResponse response;
    rpc3.RegisterUser(nullptr, &request, &response);
  }

  {
    RegisterUserRequest request;
    request.set_name("user4local");
    RegisterUserResponse response;
    rpc4.RegisterUser(nullptr, &request, &response);
  }

  string user1_2_key;
  string user1_3_key;
  string user1_4_key;
  string user2_1_key;
  string user3_1_key;
  string user4_1_key;

  // make user 1 add users 2-4
  {
    GenerateFriendKeyRequest request;
    request.set_name("user2");
    GenerateFriendKeyResponse response;
    rpc1.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_GT(response.key().size(), 0);
    user1_2_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name("user3");
    GenerateFriendKeyResponse response;
    rpc1.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_GT(response.key().size(), 0);
    user1_3_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name("user4");
    GenerateFriendKeyResponse response;
    rpc1.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_GT(response.key().size(), 0);
    user1_4_key = response.key();
  }

  // make users 2-4 add user 1
  {
    GenerateFriendKeyRequest request;
    request.set_name("user1");
    GenerateFriendKeyResponse response;
    rpc2.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_GT(response.key().size(), 0);
    user2_1_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name("user1");
    GenerateFriendKeyResponse response;
    rpc3.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_GT(response.key().size(), 0);
    user3_1_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name("user1");
    GenerateFriendKeyResponse response;
    rpc4.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_GT(response.key().size(), 0);
    user4_1_key = response.key();
  }

  cout << "user1_2_key: " << user1_2_key << endl;
  cout << "user1_3_key: " << user1_3_key << endl;
  cout << "user1_4_key: " << user1_4_key << endl;
  cout << "user2_1_key: " << user2_1_key << endl;
  cout << "user3_1_key: " << user3_1_key << endl;
  cout << "user4_1_key: " << user4_1_key << endl;

  // user 1 finishes setting up users 2-4.
  {
    AddFriendRequest request;
    request.set_name("user2");
    request.set_key(user2_1_key);
    AddFriendResponse response;
    rpc1.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  {
    AddFriendRequest request;
    request.set_name("user3");
    request.set_key(user3_1_key);
    AddFriendResponse response;
    rpc1.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  {
    AddFriendRequest request;
    request.set_name("user4");
    request.set_key(user4_1_key);
    AddFriendResponse response;
    rpc1.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  // users 2-4 finish setting up user 1
  {
    AddFriendRequest request;
    request.set_name("user1");
    request.set_key(user1_2_key);
    AddFriendResponse response;
    rpc2.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  {
    AddFriendRequest request;
    request.set_name("user1");
    request.set_key(user1_3_key);
    AddFriendResponse response;
    rpc3.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  {
    AddFriendRequest request;
    request.set_name("user1");
    request.set_key(user1_4_key);
    AddFriendResponse response;
    rpc4.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  // user 1 sends message to 2-4
  {
    SendMessageRequest request;
    request.set_name("user2");
    request.set_message("hello from 1 to 2");
    asphrdaemon::SendMessageResponse response;
    rpc1.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  {
    SendMessageRequest request;
    request.set_name("user3");
    request.set_message("hello from 1 to 3");
    asphrdaemon::SendMessageResponse response;
    rpc1.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  {
    SendMessageRequest request;
    request.set_name("user4");
    request.set_message("hello from 1 to 4");
    asphrdaemon::SendMessageResponse response;
    rpc1.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  { t1.send_messages(); }

  {
    t2.retrieve_messages();
    t3.retrieve_messages();
    t4.retrieve_messages();
  }

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    rpc2.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).sender(), "user1");
    EXPECT_EQ(response.messages(0).message(), "hello from 1 to 2");
  }

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    rpc3.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).sender(), "user1");
    EXPECT_EQ(response.messages(0).message(), "hello from 1 to 3");
  }

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    rpc4.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).sender(), "user1");
    EXPECT_EQ(response.messages(0).message(), "hello from 1 to 4");
  }
};

TEST_F(MultipleFriendsTest, SendMultipleMessages) {
  ResetStub();

  auto crypto1 = gen_crypto();
  auto config1 = gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc1(crypto1, config1, stub_);
  Transmitter t1(crypto1, config1, stub_);
  auto crypto2 = gen_crypto();
  auto config2 = gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc2(crypto2, config2, stub_);
  Transmitter t2(crypto2, config2, stub_);

  {
    RegisterUserRequest request;
    request.set_name("user1local");
    RegisterUserResponse response;
    rpc1.RegisterUser(nullptr, &request, &response);
  }
  {
    RegisterUserRequest request;
    request.set_name("user2local");
    RegisterUserResponse response;
    rpc2.RegisterUser(nullptr, &request, &response);
  }

  string user1_key;
  string user2_key;

  {
    GenerateFriendKeyRequest request;
    request.set_name("user2");
    GenerateFriendKeyResponse response;
    rpc1.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_GT(response.key().size(), 0);
    user1_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name("user1");
    GenerateFriendKeyResponse response;
    rpc2.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
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
    rpc1.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  {
    AddFriendRequest request;
    request.set_name("user1");
    request.set_key(user1_key);
    AddFriendResponse response;
    rpc2.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  {
    SendMessageRequest request;
    request.set_name("user2");
    request.set_message("hello from 1 to 2");
    asphrdaemon::SendMessageResponse response;
    rpc1.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  {
    SendMessageRequest request;
    request.set_name("user2");
    request.set_message("hello from 1 to 2, again!!!! :0");
    asphrdaemon::SendMessageResponse response;
    rpc1.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  {
    t1.send_messages();
    t2.send_messages();
  }

  {
    t1.retrieve_messages();
    t2.retrieve_messages();
  }

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    rpc1.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_EQ(response.messages_size(), 0);
  }

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    rpc2.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).sender(), "user1");
    EXPECT_EQ(response.messages(0).message(), "hello from 1 to 2");
  }

  {
    t1.send_messages();
    t2.send_messages();
  }

  {
    t1.retrieve_messages();
    t2.retrieve_messages();
  }

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    rpc1.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_EQ(response.messages_size(), 0);
  }

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    rpc2.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_EQ(response.messages_size(), 2);
    EXPECT_EQ(response.messages(0).sender(), "user1");
    EXPECT_EQ(response.messages(0).message(),
              "hello from 1 to 2, again!!!! :0");
  }
};

}  // namespace
}  // namespace asphr::testing