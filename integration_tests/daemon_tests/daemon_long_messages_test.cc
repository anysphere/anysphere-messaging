#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, SendLongMessage) {
  ResetStub();

  FriendTestingInfo friend1, friend2;
  std::tie(friend1, friend2) = generate_two_friends();

  // these messages are too big to be in a single message, but small enough to
  // be contained in two messages.
  auto m1 = string("");
  auto m2 = string("");
  for (size_t i = 0; i < 3 * GUARANTEED_SINGLE_MESSAGE_SIZE / 2; i++) {
    absl::StrAppend(&m1, "a");
    absl::StrAppend(&m2, "b");
  }

  {
    SendMessageRequest request;
    request.add_unique_name(friend2.unique_name);
    request.set_message(m1);
    asphrdaemon::SendMessageResponse response;
    auto status = friend1.rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    SendMessageRequest request;
    request.add_unique_name(friend1.unique_name);
    request.set_message(m2);
    asphrdaemon::SendMessageResponse response;
    auto status = friend2.rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    friend1.t->retrieve();
    friend1.t->send();
  }

  {
    friend2.t->retrieve();
    friend2.t->send();
  }

  { friend1.t->retrieve(); }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend1.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), friend2.extra_messages);
  }

  {
    friend1.t->retrieve();
    friend1.t->send();
  }

  {
    friend2.t->retrieve();
    friend2.t->send();
  }

  { friend1.t->retrieve(); }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend1.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1 + friend1.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), "user2");
    EXPECT_EQ(response.messages(0).message(), m2);
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1 + friend2.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), "user1");
    EXPECT_EQ(response.messages(0).message(), m1);
  }
};

TEST_F(DaemonRpcTest, SendLongMessagePersistence) {
  ResetStub();

  auto config1_file = generateTempFile();
  auto config2_file = generateTempFile();

  // these messages are too big to be in a single message, but small enough to
  // be contained in two messages.
  auto m1 = string("");
  auto m2 = string("");
  for (size_t i = 0; i < 3 * GUARANTEED_SINGLE_MESSAGE_SIZE / 2; i++) {
    absl::StrAppend(&m1, "a");
    absl::StrAppend(&m2, "b");
  }

  {
    FriendTestingInfo friend1, friend2;
    std::tie(friend1, friend2) =
        generate_two_friends(config1_file, config2_file);

    {
      SendMessageRequest request;
      request.add_unique_name(friend2.unique_name);
      request.set_message(m1);
      asphrdaemon::SendMessageResponse response;
      auto status = friend1.rpc->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      SendMessageRequest request;
      request.add_unique_name(friend1.unique_name);
      request.set_message(m2);
      asphrdaemon::SendMessageResponse response;
      auto status = friend2.rpc->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      friend1.t->retrieve();
      friend1.t->send();
    }

    {
      friend2.t->retrieve();
      friend2.t->send();
    }

    { friend1.t->retrieve(); }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = friend1.rpc->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 0 + friend1.extra_messages);
    }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 0 + friend2.extra_messages);
    }
  }

  {
    auto [G1, rpc1, t1] = gen_person(config1_file);
    auto [G2, rpc2, t2] = gen_person(config2_file);

    {
      t1->retrieve();
      t1->send();
    }

    {
      t2->retrieve();
      t2->send();
    }

    { t1->retrieve(); }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc1->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).from_unique_name(), "user2");
      EXPECT_EQ(response.messages(0).message(), m2);
    }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc2->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 2);  // +1 for invitation message.
      EXPECT_EQ(response.messages(0).from_unique_name(), "user1");
      EXPECT_EQ(response.messages(0).message(), m1);
    }
  }
};

}  // namespace
}  // namespace asphr::testing