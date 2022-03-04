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

class DaemonRpcFastTest : public ::testing::Test {
  using ServerRpc = ServerRpc<FastPIR, AccountManagerInMemory>;

 protected:
  DaemonRpcFastTest() : service_(gen_server_rpc()) {}

  auto generateTempFile() -> string {
    auto config_file_address = "TMPTMPTMP_config" +
                               std::to_string(config_file_addresses_.size()) +
                               ".json";

    const auto large_cwd_size = 1024;
    std::array<char, large_cwd_size> cwd{};
    auto* status = getcwd(cwd.data(), cwd.size());
    if (status == nullptr) {
      throw std::runtime_error("getcwd() failed");
    }

    std::string cwd_str(cwd.data());
    auto address = cwd_str + "/" + config_file_address;
    config_file_addresses_.push_back(address);
    return address;
  }

  auto generateTempDir() -> std::filesystem::path {
    auto tmp_dir = "TMPTMPTMP_dirs" + std::to_string(temp_dirs_.size());

    const auto large_cwd_size = 1024;
    std::array<char, large_cwd_size> cwd{};
    auto* status = getcwd(cwd.data(), cwd.size());
    if (status == nullptr) {
      throw std::runtime_error("getcwd() failed");
    }

    std::string cwd_str(cwd.data());
    auto address = std::filesystem::path(cwd_str) / tmp_dir;
    std::filesystem::create_directory(address);
    temp_dirs_.push_back(address);
    return address;
  }

  void SetUp() override {
    const int kRandomPort = 43427;

    int port = kRandomPort;
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

    for (const auto& f : config_file_addresses_) {
      if (remove(f.c_str()) != 0) {
        cerr << "Error deleting file";
      } else {
        cout << "File successfully deleted\n";
      }
    }
    for (const auto& f : temp_dirs_) {
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

 public:
  std::shared_ptr<asphrserver::Server::Stub> stub_;
  std::unique_ptr<grpc::Server> server_;
  std::ostringstream server_address_;
  ServerRpc service_;
  vector<string> config_file_addresses_;
  vector<std::filesystem::path> temp_dirs_;
};

TEST_F(DaemonRpcFastTest, GetFriendListUnauthenticated) {
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

TEST_F(DaemonRpcFastTest, SetLatency) {
  ResetStub();
  auto crypto = gen_crypto();
  auto config = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore = gen_msgstore(config);
  DaemonRpc rpc(crypto, config, stub_, msgstore);

  {
    GetLatencyRequest get_latency_request;
    GetLatencyResponse get_latency_response;

    ChangeLatencyRequest request;
    ChangeLatencyResponse response;

    EXPECT_TRUE(
        rpc.GetLatency(nullptr, &get_latency_request, &get_latency_response)
            .ok());
    EXPECT_EQ(get_latency_response.latency_seconds(), 60);

    const int kLatency = 5;

    request.set_latency_seconds(kLatency);
    auto status = rpc.ChangeLatency(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());

    EXPECT_TRUE(
        rpc.GetLatency(nullptr, &get_latency_request, &get_latency_response)
            .ok());
    EXPECT_EQ(get_latency_response.latency_seconds(), kLatency);
  }
};

TEST_F(DaemonRpcFastTest, SetLatencyEmpty) {
  ResetStub();
  auto crypto = gen_crypto();
  auto config = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore = gen_msgstore(config);
  DaemonRpc rpc(crypto, config, stub_, msgstore);

  {
    GetLatencyRequest get_latency_request;
    GetLatencyResponse get_latency_response;

    ChangeLatencyRequest request;
    ChangeLatencyResponse response;

    EXPECT_TRUE(
        rpc.GetLatency(nullptr, &get_latency_request, &get_latency_response)
            .ok());
    EXPECT_EQ(get_latency_response.latency_seconds(), 60);

    request.set_latency_seconds(0);
    auto status = rpc.ChangeLatency(nullptr, &request, &response);
    EXPECT_FALSE(status.ok());
  }
};

TEST_F(DaemonRpcFastTest, KillDaemon) {
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
  const int kWaitTime = 1000;
  killed = config->wait_until_killed_or_seconds(kWaitTime);
  EXPECT_TRUE(killed);
}

TEST_F(DaemonRpcFastTest, LoadAndUnloadConfig) {
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

TEST_F(DaemonRpcFastTest, LoadAndUnloadConfigAndReceiveHalfFriend) {
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

}  // namespace
}  // namespace asphr::testing
