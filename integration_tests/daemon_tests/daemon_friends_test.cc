#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, GetFriendList) {
  ResetStub();
  auto [G, rpc, t] = gen_person();

  {
    RegisterUserRequest request;
    request.set_name("test");
    request.set_beta_key("asphr_magic");
    RegisterUserResponse response;
    rpc->RegisterUser(nullptr, &request, &response);
  }

  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    auto status = rpc->GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.friend_infos_size(), 0);
  }
};

TEST_F(DaemonRpcTest, GenerateFriendKey) {
  ResetStub();
  auto [G, rpc, t] = gen_person();

  {
    RegisterUserRequest request;
    request.set_name("test");
    request.set_beta_key("asphr_magic");
    RegisterUserResponse response;
    rpc->RegisterUser(nullptr, &request, &response);
  }

  {
    GenerateFriendKeyRequest request;
    request.set_unique_name("friend_name");
    GenerateFriendKeyResponse response;
    auto status = rpc->GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
  }
};

}  // namespace
}  // namespace asphr::testing