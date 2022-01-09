#include <gtest/gtest.h>

#include "asphr/asphr.hpp"
#include "client/daemon/daemon_rpc.hpp"
#include "server/pir/fast_pir/fastpir.hpp"
#include "server/src/server_rpc.hpp"

using namespace asphrdaemon;

auto gen_crypto() -> Crypto {
  Crypto crypto;
  return crypto;
}

auto gen_config() -> Config {
  json config_json = {
      {"has_registered", false},
      {"friends", {}},
  };
  Config config(config_json);
  return config;
}

auto gen_ephemeral_config() -> EphemeralConfig {
  auto config = EphemeralConfig{
      .config_file_address = "config_file_address",
      .send_messages_file_address = "send_messages_file_address",
      .received_messages_file_address = "received_messages_file_address",
  };
  return config;
}

auto gen_server_rpc() {
  FastPIR pir;
  AccountManagerInMemory account_manager;
  return ServerRpc(std::move(pir), std::move(account_manager));
}

namespace asphr::testing {
namespace {

class DaemonRpcTest : public ::testing::Test {
  using ServerRpc = ServerRpc<FastPIR, AccountManagerInMemory>;

 protected:
  DaemonRpcTest() : service_(gen_server_rpc()) {}

  void SetUp() override {
    int port = 86651;
    server_address_ << "localhost:" << port;
    // Setup server
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address_.str(),
                             grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    server_ = builder.BuildAndStart();
  }

  void TearDown() override { server_->Shutdown(); }

  void ResetStub() {
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(
        server_address_.str(), grpc::InsecureChannelCredentials());
    stub_ = asphrserver::Server::NewStub(channel);
  }

  std::unique_ptr<asphrserver::Server::Stub> stub_;
  std::unique_ptr<grpc::Server> server_;
  std::ostringstream server_address_;
  ServerRpc service_;
};

TEST_F(DaemonRpcTest, Register) {
  ResetStub();
  RegisterUserRequest request;
  request.set_name("test");

  RegisterUserResponse response;

  auto crypto = gen_crypto();
  auto config = gen_config();
  auto ephConfig = gen_ephemeral_config();

  DaemonRpc rpc(crypto, config, stub_, ephConfig);
  cout << "response.success() = " << response.success() << endl;
  rpc.RegisterUser(nullptr, &request, &response);

  EXPECT_TRUE(response.success());
};

}  // namespace
}  // namespace asphr::testing