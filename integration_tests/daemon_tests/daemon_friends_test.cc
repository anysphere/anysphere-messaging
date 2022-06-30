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

TEST_F(DaemonRpcTest, GetMyPublicID) {
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
    GetMyPublicIDRequest request;
    GetMyPublicIDResponse response;
    auto status = rpc->GetMyPublicID(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.public_id().size(), 0);
  }
};

}  // namespace
}  // namespace asphr::testing