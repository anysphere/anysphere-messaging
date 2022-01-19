#include <gtest/gtest.h>

#include "asphr/asphr.hpp"
#include "client/daemon/daemon_rpc.hpp"
#include "client/daemon/ui_msg.hpp"
#include "server/pir/fast_pir/fastpir.hpp"
#include "server/src/server_rpc.hpp"

using namespace asphrdaemon;

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

TEST_F(DaemonRpcTest, Register) {
  ResetStub();
  auto crypto = gen_crypto();
  auto config = gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc(crypto, config, stub_);

  {
    RegisterUserRequest request;
    request.set_name("test");
    RegisterUserResponse response;
    rpc.RegisterUser(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }
};

TEST_F(DaemonRpcTest, GetFriendList) {
  ResetStub();
  auto crypto = gen_crypto();
  auto config = gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc(crypto, config, stub_);

  {
    RegisterUserRequest request;
    request.set_name("test");
    RegisterUserResponse response;
    rpc.RegisterUser(nullptr, &request, &response);
  }

  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    rpc.GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_EQ(response.friend_list_size(), 0);
  }
};

TEST_F(DaemonRpcTest, GenerateFriendKey) {
  ResetStub();
  auto crypto = gen_crypto();
  auto config = gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc(crypto, config, stub_);

  {
    RegisterUserRequest request;
    request.set_name("test");
    RegisterUserResponse response;
    rpc.RegisterUser(nullptr, &request, &response);
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name("friend_name");
    GenerateFriendKeyResponse response;
    rpc.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_GT(response.key().size(), 0);
  }
};

TEST_F(DaemonRpcTest, AddFriend) {
  ResetStub();

  auto crypto1 = gen_crypto();
  auto config1 = gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc1(crypto1, config1, stub_);
  auto crypto2 = gen_crypto();
  auto config2 = gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc2(crypto2, config2, stub_);

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
};

TEST_F(DaemonRpcTest, SendMessage) {
  ResetStub();

  auto crypto1 = gen_crypto();
  auto config1 = gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc1(crypto1, config1, stub_);
  auto crypto2 = gen_crypto();
  auto config2 = gen_config(string(generateTempDir()), generateTempFile());
  DaemonRpc rpc2(crypto2, config2, stub_);

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
    request.set_name("user1");
    request.set_message("hello from 2 to 1");
    asphrdaemon::SendMessageResponse response;
    rpc2.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
  }

  {
    process_ui_file(config1.send_file_address(), absl::Time(), stub_, crypto1,
                    config1);
    process_ui_file(config2.send_file_address(), absl::Time(), stub_, crypto2,
                    config2);
  }

  {
    retrieve_messages(config1.receive_file_address(), stub_, crypto1, config1);
    retrieve_messages(config2.receive_file_address(), stub_, crypto2, config2);
  }

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    rpc1.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(response.success());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).sender(), "user2");
    EXPECT_EQ(response.messages(0).message(), "hello from 2 to 1");
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
};

}  // namespace
}  // namespace asphr::testing