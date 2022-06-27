#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, AddFriend) {
  ResetStub();

  generate_two_friends();
};

TEST_F(DaemonRpcTest, AddFriendAndCheckFriendList) {
  ResetStub();

  // we cannot use generate_two_friends() here, because we want to
  // check the friend list in the middle

  auto [G1, rpc1, t1] = gen_person();
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
    GetFriendListRequest request;
    GetFriendListResponse response;
    auto status = rpc1->GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.friend_infos_size(), 0);
  }

  string user1_key;
  string user2_key;

  {
    GenerateFriendKeyRequest request;
    request.set_unique_name("user2");
    GenerateFriendKeyResponse response;
    auto status = rpc1->GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user1_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_unique_name("user1");
    GenerateFriendKeyResponse response;
    auto status = rpc2->GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user2_key = response.key();
  }

  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    auto status = rpc1->GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.friend_infos_size(), 1);
    EXPECT_EQ(response.friend_infos(0).unique_name(), "user2");
    EXPECT_EQ(response.friend_infos(0).enabled(), false);
  }

  cout << "user1_key: " << user1_key << endl;
  cout << "user2_key: " << user2_key << endl;

  {
    AddFriendRequest request;
    request.set_unique_name("user2");
    request.set_key(user2_key);
    AddFriendResponse response;
    auto status = rpc1->AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    AddFriendRequest request;
    request.set_unique_name("user1");
    request.set_key(user1_key);
    AddFriendResponse response;
    auto status = rpc2->AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    GetFriendListRequest request;
    GetFriendListResponse response;
    auto status = rpc1->GetFriendList(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.friend_infos_size(), 1);
    EXPECT_EQ(response.friend_infos(0).unique_name(), "user2");
    EXPECT_EQ(response.friend_infos(0).enabled(), true);
  }
};

}  // namespace
}  // namespace asphr::testing