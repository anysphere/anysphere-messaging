#include "server_rpc.hpp"

#include <gtest/gtest.h>

#include "schema/server.grpc.pb.h"
#include "server/pir/fast_pir/fastpir.hpp"

using namespace asphrserver;
using namespace std::string_literals;

TEST(ServerRpcTest, Register) {
  FastPIR pir;
  FastPIR pir_acks;
  AccountManagerInMemory account_manager;
  auto rpc = ServerRpc<FastPIR, AccountManagerInMemory>(
      std::move(pir), std::move(pir_acks), std::move(account_manager));

  {
    RegisterInfo request;

    request.set_invitation_public_key("fake_public_key");
    request.set_beta_key("asphr_magic");
    RegisterResponse response;
    auto status = rpc.Register(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.public_key(), "fake_public_key");
    EXPECT_GE(response.allocation_size(), 1);
  }
};

TEST(ServerRpcTest, ExistingAccounts) {
  AccountManagerInMemory account_manager;

  string authentication_token;
  vector<int> allocation;

  {
    FastPIR pir;
    FastPIR pir_acks;
    auto rpc = ServerRpc<FastPIR, AccountManagerInMemory>(
        std::move(pir), std::move(pir_acks), std::move(account_manager));

    {
      RegisterInfo request;
      request.set_invitation_public_key("fake_public_key");
      request.set_beta_key("asphr_magic");
      RegisterResponse response;
      auto status = rpc.Register(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.public_key(), "fake_public_key");
      EXPECT_GE(response.allocation_size(), 1);
      authentication_token = response.authentication_token();
      for (const auto& i : response.allocation()) {
        allocation.push_back(i);
      }
    }

    {
      RegisterInfo request;
      request.set_invitation_public_key("fake_public_key2");
      request.set_beta_key("asphr_magic");
      RegisterResponse response;
      auto status = rpc.Register(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.public_key(), "fake_public_key2");
      EXPECT_GE(response.allocation_size(), 1);
    }

    account_manager = rpc.account_manager_FOR_TESTING_ONLY();
  }

  {
    FastPIR pir;
    FastPIR pir_acks;
    auto rpc = ServerRpc<FastPIR, AccountManagerInMemory>(
        std::move(pir), std::move(pir_acks), std::move(account_manager));

    {
      SendMessageInfo request;
      request.set_authentication_token(authentication_token);
      string message;
      for (size_t i = 0; i < MESSAGE_SIZE; i++) {
        message += "a";
      }
      string acks;
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
    {
      RegisterInfo request;
      request.set_invitation_public_key("fake_public_key3");
      request.set_beta_key("asphr_magic");
      RegisterResponse response;
      auto status = rpc.Register(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.public_key(), "fake_public_key3");
      EXPECT_GE(response.allocation_size(), 1);
    }
  }
};

TEST(ServerRpcTest, SendMessage) {
  FastPIR pir;
  FastPIR pir_acks;
  AccountManagerInMemory account_manager;
  auto rpc = ServerRpc<FastPIR, AccountManagerInMemory>(
      std::move(pir), std::move(pir_acks), std::move(account_manager));

  string authentication_token;
  vector<int> allocation;
  {
    RegisterInfo request;
    request.set_invitation_public_key("fake_public_key");
    request.set_beta_key("asphr_magic");
    RegisterResponse response;
    auto status = rpc.Register(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.public_key(), "fake_public_key");
    EXPECT_GE(response.allocation_size(), 1);
    authentication_token = response.authentication_token();
    for (const auto& i : response.allocation()) {
      allocation.push_back(i);
    }
  }

  {
    SendMessageInfo request;
    request.set_authentication_token(authentication_token);
    string message;
    for (size_t i = 0; i < MESSAGE_SIZE; i++) {
      message += "a";
    }
    string acks;
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
  AccountManagerInMemory account_manager;
  auto rpc = ServerRpc<FastPIR, AccountManagerInMemory>(
      std::move(pir), std::move(pir_acks), std::move(account_manager));

  string authentication_token;
  vector<int> allocation;
  {
    RegisterInfo request;
    request.set_invitation_public_key("fake_public_key");
    request.set_beta_key("asphr_magic");
    RegisterResponse response;
    auto status = rpc.Register(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.public_key(), "fake_public_key");
    EXPECT_GE(response.allocation_size(), 1);
    authentication_token = response.authentication_token();
    for (const auto& i : response.allocation()) {
      allocation.push_back(i);
    }
  }

  {
    SendMessageInfo request;
    request.set_authentication_token(authentication_token);
    string message;
    for (size_t i = 0; i < MESSAGE_SIZE - 1; i++) {
      message += "a";
    }
    string acks;
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
  AccountManagerInMemory account_manager;
  auto rpc = ServerRpc<FastPIR, AccountManagerInMemory>(
      std::move(pir), std::move(pir_acks), std::move(account_manager));

  string authentication_token;
  vector<int> allocation;
  {
    RegisterInfo request;
    request.set_invitation_public_key("fake_public_key");
    request.set_beta_key("asphr_magic");
    RegisterResponse response;
    auto status = rpc.Register(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.public_key(), "fake_public_key");
    EXPECT_GE(response.allocation_size(), 1);
    authentication_token = response.authentication_token();
    for (const auto& i : response.allocation()) {
      allocation.push_back(i);
    }
  }

  {
    SendMessageInfo request;
    request.set_authentication_token(authentication_token);
    string message;
    for (size_t i = 0; i < MESSAGE_SIZE; i++) {
      message += "a";
    }
    string acks;
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

TEST(ServerRpcTest, AddGetAsyncInvitationMalformed) {
  FastPIR pir;
  FastPIR pir_acks;
  AccountManagerInMemory account_manager;
  auto rpc = ServerRpc<FastPIR, AccountManagerInMemory>(
      std::move(pir), std::move(pir_acks), std::move(account_manager));

  string authentication_token;
  vector<int> allocation;
  auto public_key = "fake_pub\1\0\1lic_key"s;
  {
    RegisterInfo request;
    request.set_invitation_public_key(public_key);
    request.set_beta_key("asphr_magic");
    RegisterResponse response;
    auto status = rpc.Register(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.public_key(), public_key);
    EXPECT_GE(response.allocation_size(), 1);
    authentication_token = response.authentication_token();
    for (const auto& i : response.allocation()) {
      allocation.push_back(i);
    }
  }

  // good invitation
  string invitation_0 =
      "fake-invitation-which-is-encrypted-in-the-real-world\0\0-some-random-"
      "bytes"s;
  {
    AddAsyncInvitationInfo request;
    request.set_authentication_token(authentication_token);
    request.set_index(allocation.at(0));
    request.set_invitation(invitation_0);
    AddAsyncInvitationResponse response;
    auto status = rpc.AddAsyncInvitation(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // too long invitation
  {
    AddAsyncInvitationInfo request;
    request.set_authentication_token(authentication_token);
    request.set_index(allocation.at(0));
    auto str = ""s;
    for (int i = 0; i < MAX_INVITATION_LENGTH + 1; i++) {
      str += "a";
    }
    request.set_invitation(str);
    AddAsyncInvitationResponse response;
    auto status = rpc.AddAsyncInvitation(nullptr, &request, &response);
    EXPECT_FALSE(status.ok());
  }

  // honest user
  {
    GetAsyncInvitationsInfo request;
    request.set_start_index(allocation.at(0));
    request.set_end_index(allocation.at(0) +
                          MAX_ASYNC_INVITATION_BATCH_SIZE / 2);
    GetAsyncInvitationsResponse response;
    auto status = rpc.GetAsyncInvitations(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.invitations_size(), MAX_ASYNC_INVITATION_BATCH_SIZE / 2);
    EXPECT_EQ(response.invitations(0), invitation_0);
    EXPECT_EQ(response.invitation_public_key(0), public_key);
  }

  // bad indices
  {
    GetAsyncInvitationsInfo request;
    request.set_start_index(allocation.at(0));
    request.set_end_index(allocation.at(0) + MAX_ASYNC_INVITATION_BATCH_SIZE +
                          1);
    GetAsyncInvitationsResponse response;
    auto status = rpc.GetAsyncInvitations(nullptr, &request, &response);
    EXPECT_FALSE(status.ok());
  }
  {
    GetAsyncInvitationsInfo request;
    request.set_start_index(allocation.at(0));
    request.set_end_index(-1);
    GetAsyncInvitationsResponse response;
    auto status = rpc.GetAsyncInvitations(nullptr, &request, &response);
    EXPECT_FALSE(status.ok());
  }
  {
    GetAsyncInvitationsInfo request;
    request.set_start_index(-1);
    request.set_end_index(allocation.at(0));
    GetAsyncInvitationsResponse response;
    auto status = rpc.GetAsyncInvitations(nullptr, &request, &response);
    EXPECT_FALSE(status.ok());
  }
}