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

  auto generate_two_friends() -> pair<FriendTestingInfo, FriendTestingInfo> {
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

    auto friend1 = FriendTestingInfo{move(G1), move(rpc1), move(t1)};
    auto friend2 = FriendTestingInfo{move(G2), move(rpc2), move(t2)};
    auto friends = make_pair(std::move(friend1), std::move(friend2));

    return friends;
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