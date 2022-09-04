//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, SeenMessage) {
  ResetStub();

  FriendTestingInfo friend1, friend2;
  std::tie(friend1, friend2) = generate_two_friends();

  {
    SendMessageRequest request;
    request.add_unique_name(friend2.unique_name);
    request.set_message("hello from 1 to 2");
    asphrdaemon::SendMessageResponse response;
    auto status = friend1.rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    SendMessageRequest request;
    request.add_unique_name(friend2.unique_name);
    request.set_message("hello from 1 to 2, again!!!! :0");
    asphrdaemon::SendMessageResponse response;
    auto status = friend1.rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // retrieve-send is how messages are propagated!
  friend1.t->retrieve();
  friend1.t->send();

  friend2.t->retrieve();
  friend2.t->send();

  // 1 can impossibly receive anything
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend1.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0 + friend1.extra_messages);
  }

  // 2 should have received the first message!
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1 + friend2.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), "user1");
    EXPECT_EQ(response.messages(0).message(), "hello from 1 to 2");
  }

  // 2 has sent the ACK for the first message, so 1 should safely send the next
  // message
  friend1.t->retrieve();
  friend1.t->send();

  friend2.t->retrieve();
  friend2.t->send();

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend1.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 0);
  }

  int first_message_id;

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2 + friend2.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), "user1");
    EXPECT_EQ(response.messages(0).message(),
              "hello from 1 to 2, again!!!! :0");
    EXPECT_EQ(response.messages(1).from_unique_name(), "user1");
    EXPECT_EQ(response.messages(1).message(), "hello from 1 to 2");
    first_message_id = response.messages(1).uid();
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::NEW);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2 + friend2.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), "user1");
    EXPECT_EQ(response.messages(0).message(),
              "hello from 1 to 2, again!!!! :0");
    EXPECT_EQ(response.messages(1).from_unique_name(), "user1");
    EXPECT_EQ(response.messages(1).message(), "hello from 1 to 2");
  }

  // now see the message!
  {
    MessageSeenRequest request;
    MessageSeenResponse response;
    request.set_id(first_message_id);
    auto status = friend2.rpc->MessageSeen(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // all messages should be same, new messages should be different!
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2 + friend2.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), "user1");
    EXPECT_EQ(response.messages(0).message(),
              "hello from 1 to 2, again!!!! :0");
    EXPECT_EQ(response.messages(1).from_unique_name(), "user1");
    EXPECT_EQ(response.messages(1).message(), "hello from 1 to 2");
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::NEW);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1 + friend2.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), "user1");
    EXPECT_EQ(response.messages(0).message(),
              "hello from 1 to 2, again!!!! :0");
  }
};

}  // namespace
}  // namespace asphr::testing