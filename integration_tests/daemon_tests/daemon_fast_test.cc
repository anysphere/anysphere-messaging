#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

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

TEST_F(DaemonRpcTest, SetLatency) {
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

TEST_F(DaemonRpcTest, SetLatencyEmpty) {
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
  const int kWaitTime = 1000;
  killed = config->wait_until_killed_or_seconds(kWaitTime);
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

}  // namespace
}  // namespace asphr::testing
