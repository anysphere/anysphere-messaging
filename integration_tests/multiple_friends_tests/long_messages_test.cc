#include "../daemon_setup.hpp"

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

namespace asphr::testing {
namespace {

class MultipleFriendsTest : public DaemonRpcTest {};

TEST_F(MultipleFriendsTest, SendLongMessage) {
  ResetStub();

  vector<string> names = {"user1local", "user2local", "user3local",
                          "user4local"};
  vector<Crypto> cryptos;
  vector<shared_ptr<Config>> configs;
  for (size_t i = 0; i < names.size(); i++) {
    auto config = gen_config(string(generateTempDir()), generateTempFile());
    configs.push_back(config);
  }
  vector<unique_ptr<DaemonRpc>> rpcs;
  vector<unique_ptr<Transmitter>> ts;
  for (size_t i = 0; i < names.size(); i++) {
    cryptos.push_back(gen_crypto());
    auto msgstore = gen_msgstore(configs[i]);
    auto rpc_ptr =
        make_unique<DaemonRpc>(cryptos[i], configs[i], stub_, msgstore);
    rpcs.push_back(std::move(rpc_ptr));
    auto t_ptr =
        make_unique<Transmitter>(cryptos[i], configs[i], stub_, msgstore);
    ts.push_back(std::move(t_ptr));
  }

  for (size_t i = 0; i < names.size(); i++) {
    RegisterUserRequest request;
    request.set_name(names[i]);
    request.set_beta_key("asphr_magic");
    RegisterUserResponse response;
    rpcs[i]->RegisterUser(nullptr, &request, &response);
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
    request.set_name(names[1]);
    GenerateFriendKeyResponse response;
    auto status = rpcs[0]->GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user1_2_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name(names[2]);
    GenerateFriendKeyResponse response;
    auto status = rpcs[0]->GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user1_3_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name(names[3]);
    GenerateFriendKeyResponse response;
    auto status = rpcs[0]->GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user1_4_key = response.key();
  }

  // make users 2-4 add user 1
  {
    GenerateFriendKeyRequest request;
    request.set_name(names[0]);
    GenerateFriendKeyResponse response;
    auto status = rpcs[1]->GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user2_1_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name(names[0]);
    GenerateFriendKeyResponse response;
    auto status = rpcs[2]->GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user3_1_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name(names[0]);
    GenerateFriendKeyResponse response;
    auto status = rpcs[3]->GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
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
    request.set_name(names[1]);
    request.set_key(user2_1_key);
    AddFriendResponse response;
    auto status = rpcs[0]->AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    AddFriendRequest request;
    request.set_name(names[2]);
    request.set_key(user3_1_key);
    AddFriendResponse response;
    auto status = rpcs[0]->AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    AddFriendRequest request;
    request.set_name(names[3]);
    request.set_key(user4_1_key);
    AddFriendResponse response;
    auto status = rpcs[0]->AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // users 2-4 finish setting up user 1
  {
    AddFriendRequest request;
    request.set_name(names[0]);
    request.set_key(user1_2_key);
    AddFriendResponse response;
    auto status = rpcs[1]->AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    AddFriendRequest request;
    request.set_name(names[0]);
    request.set_key(user1_3_key);
    AddFriendResponse response;
    auto status = rpcs[2]->AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    AddFriendRequest request;
    request.set_name(names[0]);
    request.set_key(user1_4_key);
    AddFriendResponse response;
    auto status = rpcs[3]->AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  string long_message;
  for (size_t j = 0; j < GUARANTEED_SINGLE_MESSAGE_SIZE + 1; j++) {
    long_message += "a";
  }

  // user 1 sends message to 2-4
  for (size_t i = 1; i < names.size(); i++) {
    SendMessageRequest request;
    request.set_name(names[i]);
    request.set_message(long_message);
    asphrdaemon::SendMessageResponse response;
    auto status = rpcs[0]->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // execute a round
  unordered_set<int> to_receive = {1, 2, 3};

  for (auto round = 0; round < 3; round++) {
    // first round, none!
    for (size_t i = 0; i < names.size(); i++) {
      ts[i]->retrieve_messages();
      ts[i]->send_messages();
    }

    for (auto i : to_receive) {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpcs[i]->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 0);
    }

    // second round, one!
    for (size_t i = 0; i < names.size(); i++) {
      ts[i]->retrieve_messages();
      ts[i]->send_messages();
    }

    unordered_set<int> new_to_receive;

    for (auto i : to_receive) {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpcs[i]->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      if (response.messages_size() > 0) {
        EXPECT_EQ(response.messages_size(), 1);
        EXPECT_EQ(response.messages(0).from(), names[0]);
        EXPECT_EQ(response.messages(0).m().message(), long_message);
      } else {
        new_to_receive.insert(i);
      }
    }

    to_receive = new_to_receive;
  }

  EXPECT_EQ(to_receive.size(), 0);
};

}  // namespace
}  // namespace asphr::testing