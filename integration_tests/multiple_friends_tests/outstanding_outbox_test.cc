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

TEST_F(MultipleFriendsTest, ReceiveMessageEvenIfOutstandingOutboxMessage) {
  ResetStub();

  vector<string> names = {"user1local", "user2local", "user3local"};
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
  string user2_1_key;
  string user3_1_key;

  // make user 1 add users 2-3
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

  // make users 2-3 add user 1
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

  cout << "user1_2_key: " << user1_2_key << endl;
  cout << "user1_3_key: " << user1_3_key << endl;
  cout << "user2_1_key: " << user2_1_key << endl;
  cout << "user3_1_key: " << user3_1_key << endl;

  // user 1 finishes setting up users 2-3.
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

  // users 2-3 finish setting up user 1
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

  // user 1 sends message to 2, never to be received because 2 is offline
  {
    SendMessageRequest request;
    request.set_name(names[1]);
    request.set_message(absl::StrCat("hello from 0 to ", 1));
    asphrdaemon::SendMessageResponse response;
    auto status = rpcs[0]->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    // user 3 sends a message to user 1, which should eventually be received!
    SendMessageRequest request;
    request.set_name(names[0]);
    request.set_message(absl::StrCat("hello from 2 to ", 0));
    asphrdaemon::SendMessageResponse response;
    auto status = rpcs[2]->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // execute 12 rounds. this may fail!! and in that case we just want to re-run
  // the test
  const int kNumRounds = 12;
  for (auto round = 0; round < kNumRounds; round++) {
    for (size_t i = 0; i < names.size(); i++) {
      // user 2 is offline!
      if (i == 1) {
        continue;
      }
      ts[i]->retrieve_messages();
      ts[i]->send_messages();
    }
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpcs[0]->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    if (response.messages_size() > 0) {
      break;
    }
  }
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = rpcs[0]->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).from(), names[2]);
    EXPECT_EQ(response.messages(0).m().message(),
              asphr::StrCat("hello from 2 to ", 0));
  }
};

}  // namespace
}  // namespace asphr::testing