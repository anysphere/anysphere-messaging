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