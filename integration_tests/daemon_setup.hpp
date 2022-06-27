#pragma once

#include <gtest/gtest.h>

#include "asphr/asphr.hpp"
#include "daemon/db/db.hpp"
#include "daemon/global.hpp"
#include "daemon/rpc/daemon_rpc.hpp"
#include "daemon/transmitter/transmitter.hpp"
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

struct FriendTestingInfo {
  string unique_name;
  unique_ptr<Global> G;
  unique_ptr<DaemonRpc> rpc;
  unique_ptr<Transmitter> t;
};

using namespace asphrdaemon;

namespace asphr::testing {
namespace {

class DaemonRpcTest : public ::testing::Test {
  using ServerRpc = ServerRpc<FastPIR, AccountManagerInMemory>;

 protected:
  DaemonRpcTest() : service_(gen_server_rpc()) {}

  auto generateTempFile() -> string {
    auto config_file_address = string("TMPTMPTMP_config");
    for (auto i = 0; i < 50; i++) {
      config_file_address += static_cast<char>(absl::Uniform(bitgen_, 97, 122));
    }
    config_file_address += ".json";

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
    auto tmp_dir = string("TMPTMPTMP_dirs");
    for (auto i = 0; i < 50; i++) {
      tmp_dir += static_cast<char>(absl::Uniform(bitgen_, 97, 122));
    }

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

  auto gen_person() -> std::tuple<unique_ptr<Global>, unique_ptr<DaemonRpc>,
                                  unique_ptr<Transmitter>> {
    return gen_person(generateTempFile());
  }

  auto gen_person(string db_file)
      -> std::tuple<unique_ptr<Global>, unique_ptr<DaemonRpc>,
                    unique_ptr<Transmitter>> {
    auto G = make_unique<Global>(db_file);
    auto rpc = make_unique<DaemonRpc>(*G, stub_);
    auto t = make_unique<Transmitter>(*G, stub_);
    return std::make_tuple(std::move(G), std::move(rpc), std::move(t));
  }

  auto generate_friends_from_list_of_pairs(int n, vector<pair<int, int>> pairs)
      -> vector<FriendTestingInfo> {
    vector<string> db_files;
    for (auto i = 0; i < n; i++) {
      db_files.push_back(generateTempFile());
    }
    return generate_friends_from_list_of_pairs(db_files, pairs);
  }

  auto generate_friends_from_list_of_pairs(vector<string> db_files,
                                           vector<pair<int, int>> pairs)
      -> vector<FriendTestingInfo> {
    vector<FriendTestingInfo> friends;
    for (size_t i = 0; i < db_files.size(); i++) {
      auto [G, rpc, t] = gen_person(db_files.at(i));
      auto name = absl::StrCat("user", i + 1);
      {
        RegisterUserRequest request;
        request.set_name(name);
        request.set_beta_key("asphr_magic");
        RegisterUserResponse response;
        rpc->RegisterUser(nullptr, &request, &response);
      }
      friends.push_back({name, std::move(G), std::move(rpc), std::move(t)});
    }
    for (const auto& p : pairs) {
      auto& friend1 = friends.at(p.first);
      auto& friend2 = friends.at(p.second);
      string user1_key;
      string user2_key;
      {
        GenerateFriendKeyRequest request;
        request.set_unique_name(friend2.unique_name);
        GenerateFriendKeyResponse response;
        auto status =
            friend1.rpc->GenerateFriendKey(nullptr, &request, &response);
        EXPECT_TRUE(status.ok());
        EXPECT_GT(response.key().size(), 0);
        user1_key = response.key();
      }

      {
        GenerateFriendKeyRequest request;
        request.set_unique_name(friend1.unique_name);
        GenerateFriendKeyResponse response;
        auto status =
            friend2.rpc->GenerateFriendKey(nullptr, &request, &response);
        EXPECT_TRUE(status.ok());
        EXPECT_GT(response.key().size(), 0);
        user2_key = response.key();
      }

      {
        AddFriendRequest request;
        request.set_unique_name(friend2.unique_name);
        request.set_key(user2_key);
        AddFriendResponse response;
        auto status = friend1.rpc->AddFriend(nullptr, &request, &response);
        EXPECT_TRUE(status.ok());
      }

      {
        AddFriendRequest request;
        request.set_unique_name(friend1.unique_name);
        request.set_key(user1_key);
        AddFriendResponse response;
        auto status = friend2.rpc->AddFriend(nullptr, &request, &response);
        EXPECT_TRUE(status.ok());
      }
    }
    return friends;
  }

  auto generate_two_friends() -> pair<FriendTestingInfo, FriendTestingInfo> {
    return generate_two_friends(generateTempFile(), generateTempFile());
  }

  auto generate_two_friends(string db_file1, string db_file2)
      -> pair<FriendTestingInfo, FriendTestingInfo> {
    auto v =
        generate_friends_from_list_of_pairs({db_file1, db_file2}, {
                                                                      {0, 1},
                                                                  });
    ASPHR_ASSERT_EQ(v.size(), static_cast<size_t>(2));
    return {std::move(v.at(0)), std::move(v.at(1))};
  }

  void SetUp() override {
    // get a random port number
    // https://github.com/yegor256/random-tcp-port might be useful sometime in
    // the future.
    const int port = absl::Uniform(bitgen_, 10'000, 65'000);
    server_address_ << "localhost:" << port;
    // Setup server
    grpc::ServerBuilder builder;
    cout << "server address: " << server_address_.str() << endl;
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
  absl::BitGen bitgen_;
  std::shared_ptr<asphrserver::Server::Stub> stub_;
  std::unique_ptr<grpc::Server> server_;
  std::ostringstream server_address_;
  ServerRpc service_;
  vector<string> config_file_addresses_;
  vector<std::filesystem::path> temp_dirs_;
};
}  // namespace
}  // namespace asphr::testing