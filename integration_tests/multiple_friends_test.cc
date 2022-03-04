#include <gtest/gtest.h>

#include "asphr/asphr.hpp"
#include "daemon/daemon_rpc.hpp"
#include "daemon/transmitter.hpp"
#include "server/pir/fast_pir/fastpir.hpp"
#include "server/src/server_rpc.hpp"
#include "test_helpers.hpp"

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

class MultipleFriendsTest : public ::testing::Test {
  using ServerRpc = ServerRpc<FastPIR, AccountManagerInMemory>;

 protected:
  MultipleFriendsTest() : service_(gen_server_rpc()) {}

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
    // TODO(sualeh): do NOT do this. pick a good random unused port
    const int port = 43421;
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

TEST_F(MultipleFriendsTest, SendThreeMessages) {
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

  // user 1 sends message to 2-4
  for (size_t i = 1; i < names.size(); i++) {
    SendMessageRequest request;
    request.set_name(names[i]);
    request.set_message(absl::StrCat("hello from 0 to ", i));
    asphrdaemon::SendMessageResponse response;
    auto status = rpcs[0]->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // execute a round
  unordered_set<int> to_receive = {1, 2, 3};

  for (auto round = 0; round < 3; round++) {
    for (size_t i = 0; i < names.size(); i++) {
      ts[i]->retrieve_messages();
      ts[i]->send_messages();
    }

    for (size_t i = 1; i < names.size(); i++) {
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpcs[i]->GetAllMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      if (response.messages_size() > 0) {
        EXPECT_EQ(response.messages_size(), 1);
        EXPECT_EQ(response.messages(0).from(), names[0]);
        EXPECT_EQ(response.messages(0).m().message(),
                  asphr::StrCat("hello from 0 to ", i));
        to_receive.erase(i);
      }
    }
  }

  EXPECT_EQ(to_receive.size(), 0);
};

TEST_F(MultipleFriendsTest, SendMultipleMessages) {
  ResetStub();

  auto crypto1 = gen_crypto();
  auto config1 = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore1 = gen_msgstore(config1);
  DaemonRpc rpc1(crypto1, config1, stub_, msgstore1);
  Transmitter t1(crypto1, config1, stub_, msgstore1);
  auto crypto2 = gen_crypto();
  auto config2 = gen_config(string(generateTempDir()), generateTempFile());
  auto msgstore2 = gen_msgstore(config2);
  DaemonRpc rpc2(crypto2, config2, stub_, msgstore2);
  Transmitter t2(crypto2, config2, stub_, msgstore2);

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

  string user1_key;
  string user2_key;

  {
    GenerateFriendKeyRequest request;
    request.set_name("user2");
    GenerateFriendKeyResponse response;
    auto status = rpc1.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(response.key().size(), 0);
    user1_key = response.key();
  }

  {
    GenerateFriendKeyRequest request;
    request.set_name("user1");
    GenerateFriendKeyResponse response;
    auto status = rpc2.GenerateFriendKey(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
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
    auto status = rpc1.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    AddFriendRequest request;
    request.set_name("user1");
    request.set_key(user1_key);
    AddFriendResponse response;
    auto status = rpc2.AddFriend(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    SendMessageRequest request;
    request.set_name("user2");
    request.set_message("hello from 1 to 2");
    asphrdaemon::SendMessageResponse response;
    auto status = rpc1.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    SendMessageRequest request;
    request.set_name("user2");
    request.set_message("hello from 1 to 2, again!!!! :0");
    asphrdaemon::SendMessageResponse response;
    auto status = rpc1.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  t1.retrieve_messages();
  t1.send_messages();

  t2.retrieve_messages();
  t2.send_messages();

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc1.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc2.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).from(), "user1");
    EXPECT_EQ(response.messages(0).m().message(), "hello from 1 to 2");
  }

  t1.retrieve_messages();
  t1.send_messages();

  t2.retrieve_messages();
  t2.send_messages();

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc1.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpc2.GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2);
    EXPECT_EQ(response.messages(0).from(), "user1");
    EXPECT_EQ(response.messages(0).m().message(),
              "hello from 1 to 2, again!!!! :0");
  }
};

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
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpcs[i]->GetAllMessages(nullptr, &request, &response);
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
      GetAllMessagesRequest request;
      GetAllMessagesResponse response;
      auto status = rpcs[i]->GetAllMessages(nullptr, &request, &response);
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
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpcs[0]->GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    if (response.messages_size() > 0) {
      break;
    }
  }
  {
    GetAllMessagesRequest request;
    GetAllMessagesResponse response;
    auto status = rpcs[0]->GetAllMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).from(), names[2]);
    EXPECT_EQ(response.messages(0).m().message(),
              asphr::StrCat("hello from 2 to ", 0));
  }
};

}  // namespace
}  // namespace asphr::testing