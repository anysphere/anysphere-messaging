#include "server_rpc.hpp"

#include <gtest/gtest.h>

#include "schema/server.grpc.pb.h"
#include "server/pir/fast_pir/fastpir.hpp"

using namespace asphrserver;

TEST(ServerRpcTest, Register) {
  FastPIR pir;
  FastPIR pir_acks;
  AccountManagerInMemory account_manager("unused");
  auto rpc = ServerRpc<FastPIR, AccountManagerInMemory>(
      std::move(pir), std::move(pir_acks), std::move(account_manager));

  {
    RegisterInfo request;
    request.set_public_key("fake_public_key");
    RegisterResponse response;
    auto status = rpc.Register(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.public_key(), "fake_public_key");
    EXPECT_GE(response.allocation_size(), 1);
  }
};

TEST(ServerRpcTest, SendMessage) {
  FastPIR pir;
  FastPIR pir_acks;
  AccountManagerInMemory account_manager("unused");
  auto rpc = ServerRpc<FastPIR, AccountManagerInMemory>(
      std::move(pir), std::move(pir_acks), std::move(account_manager));

  string authentication_token;
  vector<int> allocation;
  {
    RegisterInfo request;
    request.set_public_key("fake_public_key");
    RegisterResponse response;
    auto status = rpc.Register(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.public_key(), "fake_public_key");
    EXPECT_GE(response.allocation_size(), 1);
    authentication_token = response.authentication_token();
    for (auto& i : response.allocation()) {
      allocation.push_back(i);
    }
  }

  {
    SendMessageInfo request;
    request.set_authentication_token(authentication_token);
    string message = "";
    for (size_t i = 0; i < MESSAGE_SIZE; i++) {
      message += "a";
    }
    string acks = "";
    for (size_t i = 0; i < MESSAGE_SIZE; i++) {
      acks += "k";
    }
    request.set_message(message);
    request.set_acks(acks);
    request.set_index(allocation.at(0));
    SendMessageResponse response;
    auto status = rpc.SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }
};

TEST(ServerRpcTest, SendMessageIncorrectSize) {
  FastPIR pir;
  FastPIR pir_acks;
  AccountManagerInMemory account_manager("unused");
  auto rpc = ServerRpc<FastPIR, AccountManagerInMemory>(
      std::move(pir), std::move(pir_acks), std::move(account_manager));

  string authentication_token;
  vector<int> allocation;
  {
    RegisterInfo request;
    request.set_public_key("fake_public_key");
    RegisterResponse response;
    auto status = rpc.Register(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.public_key(), "fake_public_key");
    EXPECT_GE(response.allocation_size(), 1);
    authentication_token = response.authentication_token();
    for (auto& i : response.allocation()) {
      allocation.push_back(i);
    }
  }

  {
    SendMessageInfo request;
    request.set_authentication_token(authentication_token);
    string message = "";
    for (size_t i = 0; i < MESSAGE_SIZE - 1; i++) {
      message += "a";
    }
    string acks = "";
    for (size_t i = 0; i < MESSAGE_SIZE; i++) {
      acks += "k";
    }
    request.set_message(message);
    request.set_acks(acks);
    request.set_index(allocation.at(0));
    SendMessageResponse response;
    auto status = rpc.SendMessage(nullptr, &request, &response);
    EXPECT_FALSE(status.ok());
  }
};

TEST(ServerRpcTest, SendMessageIncorrectSize2) {
  FastPIR pir;
  FastPIR pir_acks;
  AccountManagerInMemory account_manager("unused");
  auto rpc = ServerRpc<FastPIR, AccountManagerInMemory>(
      std::move(pir), std::move(pir_acks), std::move(account_manager));

  string authentication_token;
  vector<int> allocation;
  {
    RegisterInfo request;
    request.set_public_key("fake_public_key");
    RegisterResponse response;
    auto status = rpc.Register(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.public_key(), "fake_public_key");
    EXPECT_GE(response.allocation_size(), 1);
    authentication_token = response.authentication_token();
    for (auto& i : response.allocation()) {
      allocation.push_back(i);
    }
  }

  {
    SendMessageInfo request;
    request.set_authentication_token(authentication_token);
    string message = "";
    for (size_t i = 0; i < MESSAGE_SIZE; i++) {
      message += "a";
    }
    string acks = "";
    for (size_t i = 0; i < MESSAGE_SIZE + 1; i++) {
      acks += "k";
    }
    request.set_message(message);
    request.set_acks(acks);
    request.set_index(allocation.at(0));
    SendMessageResponse response;
    auto status = rpc.SendMessage(nullptr, &request, &response);
    EXPECT_FALSE(status.ok());
  }
};