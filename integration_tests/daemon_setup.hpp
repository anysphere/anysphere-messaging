//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

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

struct FriendTestingInfo {
  string unique_name;
  string display_name;
  string public_id;
  int extra_messages;  // number of extra messages (such as invitation messages)
                       // that should be ignored in the tests
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

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //||                           Registration Helpers                         ||
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------

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

  auto execute_transmitter_round(vector<FriendTestingInfo>& friends) {
    for (auto& friend_info : friends) {
      friend_info.t->retrieve();
      friend_info.t->send();
    }
  }

  auto generate_friends_from_list_of_pairs(int n, vector<pair<int, int>> pairs)
      -> vector<FriendTestingInfo> {
    vector<string> db_files;
    for (auto i = 0; i < n; i++) {
      db_files.push_back(generateTempFile());
    }
    return generate_friends_from_list_of_pairs(db_files, pairs);
  }

  // friend2 will have an additional message, the invitation message.
  auto connect_friends_both_async(const FriendTestingInfo& friend1,
                                  const FriendTestingInfo& friend2) {
    // we use the one-sided async invitation procedure to connect the friends
    string friend1_id;
    string friend2_id;
    int friend1_allocation;
    {
      // get user 1's id via rpc call
      GetMyPublicIDRequest request;
      GetMyPublicIDResponse response;
      friend1.rpc->GetMyPublicID(nullptr, &request, &response);
      friend1_id = response.public_id();
    }
    {
      // get user 1's registration information
      auto friend1_registration = friend1.G->db->get_registration();
      friend1_allocation = friend1_registration.allocation;
    }
    {
      // get user 2's id via rpc call
      GetMyPublicIDRequest request;
      GetMyPublicIDResponse response;
      friend2.rpc->GetMyPublicID(nullptr, &request, &response);
      friend2_id = response.public_id();
    }

    ASPHR_LOG_INFO("Registration", friend1_id, friend1_id);
    ASPHR_LOG_INFO("Registration", friend2_id, friend2_id);
    // User 1 send user 2 a request
    {
      AddAsyncFriendRequest request;
      request.set_unique_name(friend2.unique_name);
      request.set_display_name(friend2.display_name);
      request.set_public_id(friend2_id);
      request.set_message("INVITATION-MESSAGE");
      AddAsyncFriendResponse response;
      auto status = friend1.rpc->AddAsyncFriend(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      // crash fast if cannot establish friend request
      if (!status.ok()) {
        FAIL() << "Failed to establish friend request";
      }
    }

    friend1.t->send();
    friend2.t->retrieve_async_invitations(friend1_allocation,
                                          friend1_allocation + 1);
    // user 2 should have obtained the friend request from user1
    // user 2 now approve the friend request.
    {
      AcceptAsyncInvitationRequest request;
      request.set_unique_name(friend1.unique_name);
      request.set_display_name(friend1.display_name);
      request.set_public_id(friend1_id);
      AcceptAsyncInvitationResponse response;
      auto status =
          friend2.rpc->AcceptAsyncInvitation(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      // crash fast if cannot approve friend request
      if (!status.ok()) {
        FAIL() << "Failed to approve friend request";
      }
    }
    ASPHR_LOG_INFO("Friend 2 has accepted the request from friend 1!");
    // check that user 2 has user 1 as a friend
    {
      GetFriendListRequest request;
      GetFriendListResponse response;
      friend2.rpc->GetFriendList(nullptr, &request, &response);
      int friend_index = -1;
      for (int i = 0; i < response.friend_infos_size(); i++) {
        if (response.friend_infos(i).unique_name() == friend1.unique_name) {
          EXPECT_EQ(friend_index, -1);
          friend_index = i;
        }
      }
      EXPECT_EQ(response.friend_infos(friend_index).unique_name(),
                friend1.unique_name);
      EXPECT_EQ(response.friend_infos(friend_index).display_name(),
                friend1.display_name);
      EXPECT_EQ(response.friend_infos(friend_index).public_id(), friend1_id);
      EXPECT_EQ(response.friend_infos(friend_index).invitation_progress(),
                asphrdaemon::InvitationProgress::Complete);
    }
    // retrieive system control message
    friend2.t->retrieve();
    // ACK the system control message
    friend2.t->send();
    // user 1 retrive ACK of system control message
    // and promotes user 2 to a friend
    friend1.t->retrieve();
    // user 1 should have user 2 as a friend
    {
      GetFriendListRequest request;
      GetFriendListResponse response;
      friend1.rpc->GetFriendList(nullptr, &request, &response);
      ASPHR_LOG_INFO("Friend 1 has " +
                     std::to_string(response.friend_infos_size()) + " friends");
      int friend_index = -1;
      for (int i = 0; i < response.friend_infos_size(); i++) {
        if (response.friend_infos(i).unique_name() == friend2.unique_name) {
          EXPECT_EQ(friend_index, -1);
          friend_index = i;
        }
      }
      EXPECT_EQ(response.friend_infos(friend_index).unique_name(),
                friend2.unique_name);
      EXPECT_EQ(response.friend_infos(friend_index).display_name(),
                friend2.display_name);
      EXPECT_EQ(response.friend_infos(friend_index).public_id(), friend2_id);
    }
    // reset transmitter
    friend1.t->reset_async_scanner(0);
    friend2.t->reset_async_scanner(0);
  }

  auto register_people(int n) -> vector<FriendTestingInfo> {
    vector<string> db_files;
    for (auto i = 0; i < n; i++) {
      db_files.push_back(generateTempFile());
    }
    return register_people(db_files);
  }

  auto register_people(vector<string> db_files) -> vector<FriendTestingInfo> {
    vector<FriendTestingInfo> friends;
    for (size_t i = 0; i < db_files.size(); i++) {
      auto [G, rpc, t] = gen_person(db_files.at(i));
      auto name = absl::StrCat("user", i + 1);
      auto display_name = absl::StrCat("User ", i + 1);
      {
        RegisterUserRequest request;
        request.set_name(name);
        request.set_beta_key("asphr_magic");
        RegisterUserResponse response;
        rpc->RegisterUser(nullptr, &request, &response);
      }
      // get user 1's id via rpc call
      GetMyPublicIDRequest request;
      GetMyPublicIDResponse response;
      rpc->GetMyPublicID(nullptr, &request, &response);
      auto public_id = response.public_id();
      friends.push_back({name, display_name, public_id, 0, std::move(G),
                         std::move(rpc), std::move(t)});
    }
    return friends;
  }

  // Due to the simultaneous read capacity constraint, this method might break
  // down for more than three users.
  auto generate_friends_from_list_of_pairs(vector<string> db_files,
                                           vector<pair<int, int>> pairs)
      -> vector<FriendTestingInfo> {
    std::map<string, int> friend_count = {};
    auto friends = register_people(db_files);
    for (const auto& f : friends) {
      friend_count[f.unique_name] = 0;
    }
    for (const auto& p : pairs) {
      auto& friend1 = friends.at(p.first);
      friend_count[friend1.unique_name]++;
      auto& friend2 = friends.at(p.second);
      friend_count[friend2.unique_name]++;
      connect_friends_both_async(friend1, friend2);
      friend2.extra_messages++;
    }

    for (const auto& f : friends) {
      if (friend_count.at(f.unique_name) > 2) {
        ASPHR_LOG_INFO(
            "WARNING: You have created a user with more than 2 friends");
        ASPHR_LOG_INFO(
            "WARNING: when a user has more than 2 friends, reading message "
            "becomes non-deterministic for that user");
        ASPHR_LOG_INFO("WARNING: please adjust test accordingly");
      }
    }
    cout
        << "End of Setup: Users successfully connected to each other.\n"
        << "=================================================================\n"
        << "=================================================================\n"
        << endl;
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

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //||                          Setup/Teardown Server                         ||
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void SetUp() override {
    // get a random port number
    // https://github.com/yegor256/random-tcp-port might be useful sometime in
    // the future.
    const int port = absl::Uniform(bitgen_, 10'000, 65'000);
    server_address_ << "localhost:" << port;
    // Setup server
    grpc::ServerBuilder builder;
    ASPHR_LOG_INFO("Server initializing",
                   "server address: ", server_address_.str());
    builder.AddListeningPort(server_address_.str(),
                             grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    server_ = builder.BuildAndStart();
  }

  void TearDown() override {
    server_->Shutdown();

    for (const auto& f : config_file_addresses_) {
      if (remove(f.c_str()) != 0) {
        ASPHR_LOG_ERR("Error deleting file");
      } else {
        ASPHR_LOG_INFO("File successfully deleted\n");
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

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //||                            Utility Functions                           ||
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  auto get_public_id(const FriendTestingInfo& friend_info) -> std::string {
    GetMyPublicIDRequest request;
    GetMyPublicIDResponse response;

    auto status = friend_info.rpc->GetMyPublicID(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    return response.public_id();
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