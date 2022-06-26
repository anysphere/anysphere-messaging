#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, GetFriendListUnauthenticated) {
  ResetStub();
  auto [G, rpc, t] = gen_person();

  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    auto status = rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_FALSE(status.ok());
  }
};

TEST_F(DaemonRpcTest, SetLatency) {
  ResetStub();
  auto [G, rpc, t] = gen_person();

  {
    GetLatencyRequest get_latency_request;
    GetLatencyResponse get_latency_response;

    ChangeLatencyRequest request;
    ChangeLatencyResponse response;

    EXPECT_TRUE(
        rpc->GetLatency(nullptr, &get_latency_request, &get_latency_response)
            .ok());
    EXPECT_EQ(get_latency_response.latency_seconds(), 60);

    const int kLatency = 5;

    request.set_latency_seconds(kLatency);
    auto status = rpc->ChangeLatency(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());

    EXPECT_TRUE(
        rpc->GetLatency(nullptr, &get_latency_request, &get_latency_response)
            .ok());
    EXPECT_EQ(get_latency_response.latency_seconds(), kLatency);
  }
};

TEST_F(DaemonRpcTest, SetLatencyEmpty) {
  ResetStub();
  auto [G, rpc, t] = gen_person();

  {
    GetLatencyRequest get_latency_request;
    GetLatencyResponse get_latency_response;

    ChangeLatencyRequest request;
    ChangeLatencyResponse response;

    EXPECT_TRUE(
        rpc->GetLatency(nullptr, &get_latency_request, &get_latency_response)
            .ok());
    EXPECT_EQ(get_latency_response.latency_seconds(), 60);

    request.set_latency_seconds(0);
    auto status = rpc->ChangeLatency(nullptr, &request, &response);
    EXPECT_FALSE(status.ok());
  }
};

TEST_F(DaemonRpcTest, LoadAndUnloadConfig) {
  ResetStub();
  auto db_file = generateTempFile();

  {
    auto [G, rpc, t] = gen_person(db_file);

    {
      RegisterUserRequest request;
      request.set_name("test");
      request.set_beta_key("asphr_magic");
      RegisterUserResponse response;
      auto status = rpc->RegisterUser(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }
  }

  {
    // re-create config from the file!
    auto [G, rpc, t] = gen_person(db_file);

    {
      GetFriendListRequest request;
      GetFriendListResponse response;
      auto status = rpc->GetFriendList(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.friend_infos_size(), 0);
    }
  }
}

TEST_F(DaemonRpcTest, LoadAndUnloadConfigAndReceiveHalfFriend) {
  ResetStub();
  auto db_file = generateTempFile();

  {
    auto [G1, rpc1, t1] = gen_person(db_file);
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

    {
      GenerateFriendKeyRequest request;
      request.set_unique_name("user2");
      GenerateFriendKeyResponse response;
      auto status = rpc1->GenerateFriendKey(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_GT(response.key().size(), 0);
    }

    {
      GenerateFriendKeyRequest request;
      request.set_unique_name("user1");
      GenerateFriendKeyResponse response;
      auto status = rpc2->GenerateFriendKey(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_GT(response.key().size(), 0);
    }
  }

  {
    // re-create config from the file!
    auto [G, rpc, t] = gen_person(db_file);

    t->retrieve();
    t->send();

    t->retrieve();
    t->send();
  }
}

}  // namespace
}  // namespace asphr::testing
