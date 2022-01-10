#include <gtest/gtest.h>

#include "asphr/asphr.hpp"
#include "client/daemon/daemon_rpc.hpp"
#include "server/pir/fast_pir/fastpir.hpp"
#include "server/src/server_rpc.hpp"

using namespace asphrdaemon;

auto gen_crypto() -> Crypto {
  Crypto crypto;
  return crypto;
}

auto gen_config() -> Config {
  json config_json = {
      {"has_registered", false},
      {"friends", {}},
  };
  Config config(config_json);
  return config;
}

auto gen_ephemeral_config(const string& config_file_address)
    -> EphemeralConfig {
  auto config = EphemeralConfig{
      .config_file_address = config_file_address,
      .send_messages_file_address = "send_messages_file_address",
      .received_messages_file_address = "received_messages_file_address",
  };
  return config;
}

auto gen_server_rpc() {
  FastPIR pir;
  AccountManagerInMemory account_manager;
  return ServerRpc(std::move(pir), std::move(account_manager));
}

namespace asphr::testing {
namespace {

class DaemonRpcTest : public ::testing::Test {
  using ServerRpc = ServerRpc<FastPIR, AccountManagerInMemory>;

 protected:
  DaemonRpcTest() : service_(gen_server_rpc()) {}

  auto generateConfigFile() -> string {
    auto config_file_address = "TMPTMPTMP_config" +
                               std::to_string(config_file_addresses_.size()) +
                               ".json";
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    auto address = string(cwd) + "/" + config_file_address;
    config_file_addresses_.push_back(address);
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
};

TEST_F(DaemonRpcTest, Register) {
  ResetStub();
  auto crypto = gen_crypto();
  auto config = gen_config();
  auto ephConfig = gen_ephemeral_config(generateConfigFile());
  DaemonRpc rpc(crypto, config, stub_, ephConfig);

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
  auto config = gen_config();
  auto ephConfig = gen_ephemeral_config(generateConfigFile());
  DaemonRpc rpc(crypto, config, stub_, ephConfig);

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
  auto config = gen_config();
  auto ephConfig = gen_ephemeral_config(generateConfigFile());
  DaemonRpc rpc(crypto, config, stub_, ephConfig);

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
  auto config1 = gen_config();
  auto ephConfig1 = gen_ephemeral_config(generateConfigFile());
  DaemonRpc rpc1(crypto1, config1, stub_, ephConfig1);
  auto crypto2 = gen_crypto();
  auto config2 = gen_config();
  auto ephConfig2 = gen_ephemeral_config(generateConfigFile());
  DaemonRpc rpc2(crypto2, config2, stub_, ephConfig2);

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

}  // namespace
}  // namespace asphr::testing