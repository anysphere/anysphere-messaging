//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, SendMessage) {
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
    request.add_unique_name(friend1.unique_name);
    request.set_message("hello from 2 to 1");
    asphrdaemon::SendMessageResponse response;
    auto status = friend2.rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    friend1.t->send();
    friend2.t->send();
  }

  {
    friend1.t->retrieve();
    cout << "-----------------------------" << endl;
    friend2.t->retrieve();
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend1.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1 + friend1.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), friend2.unique_name);
    EXPECT_EQ(response.messages(0).from_display_name(), friend2.display_name);
    EXPECT_EQ(response.messages(0).other_recipients().size(), 0);
    EXPECT_EQ(response.messages(0).seen(), false);
    EXPECT_EQ(response.messages(0).delivered(), true);
    EXPECT_EQ(response.messages(0).message(), "hello from 2 to 1");
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1 + friend2.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), friend1.unique_name);
    EXPECT_EQ(response.messages(0).from_display_name(), friend1.display_name);
    EXPECT_EQ(response.messages(0).other_recipients().size(), 0);
    EXPECT_EQ(response.messages(0).seen(), false);
    EXPECT_EQ(response.messages(0).delivered(), true);
    EXPECT_EQ(response.messages(0).message(), "hello from 1 to 2");
  }
};

TEST_F(DaemonRpcTest, SendMessageToMultiplePeople) {
  ResetStub();

  // 1 and 2 should not need to be friends for 0 to send a message to both
  // we make sure 2 and 3 are friends s.t. we test both friends and non-friends
  auto friends =
      generate_friends_from_list_of_pairs(4, {{0, 1}, {0, 2}, {0, 3}, {2, 3}});

  {
    SendMessageRequest request;
    request.add_unique_name(friends[1].unique_name);
    request.add_unique_name(friends[2].unique_name);
    request.add_unique_name(friends[3].unique_name);
    request.set_message("hello from 0 to both 1 and 2 and 3");
    asphrdaemon::SendMessageResponse response;
    auto status = friends[0].rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    SendMessageRequest request;
    request.add_unique_name(friends[0].unique_name);
    request.set_message("hello from 1 to 0");
    asphrdaemon::SendMessageResponse response;
    auto status = friends[1].rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // it is undefined who gets the message first. so we execute three
  // rounds
  // in fact, we need four, because it may be the case that friend 0
  // is the last one in the round, and we need two rounds *after* friend 0
  // executes its first round
  execute_transmitter_round(friends);
  execute_transmitter_round(friends);
  execute_transmitter_round(friends);
  execute_transmitter_round(friends);

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friends[0].rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1 + friends[0].extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), friends[1].unique_name);
    EXPECT_EQ(response.messages(0).from_display_name(),
              friends[1].display_name);
    EXPECT_EQ(response.messages(0).other_recipients().size(), 0);
    EXPECT_EQ(response.messages(0).seen(), false);
    EXPECT_EQ(response.messages(0).delivered(), true);
    EXPECT_EQ(response.messages(0).message(), "hello from 1 to 0");
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friends[1].rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1 + friends[1].extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), friends[0].unique_name);
    EXPECT_EQ(response.messages(0).from_display_name(),
              friends[0].display_name);
    EXPECT_EQ(response.messages(0).other_recipients().size(), 2);
    EXPECT_EQ(response.messages(0).seen(), false);
    EXPECT_EQ(response.messages(0).delivered(), true);
    EXPECT_EQ(response.messages(0).message(),
              "hello from 0 to both 1 and 2 and 3");
    // sort other_recipients by their public id
    auto other_recipients = response.messages(0).other_recipients();
    std::sort(other_recipients.begin(), other_recipients.end(),
              [](const auto& a, const auto& b) {
                return a.public_id() < b.public_id();
              });
    // 1 is not friends with either 2 or 3...
    EXPECT_EQ(other_recipients[0].unique_name(), "");
    EXPECT_EQ(other_recipients[0].display_name(), "");
    EXPECT_EQ(other_recipients[0].public_id(),
              friends[2].public_id < friends[3].public_id
                  ? friends[2].public_id
                  : friends[3].public_id);
    EXPECT_EQ(other_recipients[1].unique_name(), "");
    EXPECT_EQ(other_recipients[1].display_name(), "");
    EXPECT_EQ(other_recipients[1].public_id(),
              friends[2].public_id < friends[3].public_id
                  ? friends[3].public_id
                  : friends[2].public_id);
  }
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friends[2].rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1 + friends[2].extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), friends[0].unique_name);
    EXPECT_EQ(response.messages(0).from_display_name(),
              friends[0].display_name);
    EXPECT_EQ(response.messages(0).other_recipients().size(), 2);
    EXPECT_EQ(response.messages(0).seen(), false);
    EXPECT_EQ(response.messages(0).delivered(), true);
    EXPECT_EQ(response.messages(0).message(),
              "hello from 0 to both 1 and 2 and 3");
    // sort other_recipients by their unique_name
    auto other_recipients = response.messages(0).other_recipients();
    std::sort(other_recipients.begin(), other_recipients.end(),
              [](const auto& a, const auto& b) {
                return a.public_id() < b.public_id();
              });
    // 1 should be first because they're not a friend
    EXPECT_EQ(other_recipients[0].unique_name(), "");
    EXPECT_EQ(other_recipients[0].display_name(), "");
    EXPECT_EQ(other_recipients[0].public_id(), friends[1].public_id);
    EXPECT_EQ(other_recipients[1].unique_name(), friends[3].unique_name);
    EXPECT_EQ(other_recipients[1].display_name(), friends[3].display_name);
    EXPECT_EQ(other_recipients[1].public_id(), friends[3].public_id);
  }
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friends[3].rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1 + friends[3].extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), friends[0].unique_name);
    EXPECT_EQ(response.messages(0).from_display_name(),
              friends[0].display_name);
    EXPECT_EQ(response.messages(0).other_recipients().size(), 2);
    EXPECT_EQ(response.messages(0).seen(), false);
    EXPECT_EQ(response.messages(0).delivered(), true);
    EXPECT_EQ(response.messages(0).message(),
              "hello from 0 to both 1 and 2 and 3");
    // sort other_recipients by their unique_name
    auto other_recipients = response.messages(0).other_recipients();
    std::sort(other_recipients.begin(), other_recipients.end(),
              [](const auto& a, const auto& b) {
                return a.public_id() < b.public_id();
              });
    // 1 should be first because they're not a friend
    EXPECT_EQ(other_recipients[0].unique_name(), "");
    EXPECT_EQ(other_recipients[0].display_name(), "");
    EXPECT_EQ(other_recipients[0].public_id(), friends[1].public_id);
    EXPECT_EQ(other_recipients[1].unique_name(), friends[2].unique_name);
    EXPECT_EQ(other_recipients[1].display_name(), friends[2].display_name);
    EXPECT_EQ(other_recipients[1].public_id(), friends[2].public_id);
  }
};

TEST_F(DaemonRpcTest, SendMultipleMessages) {
  using TimeUtil = google::protobuf::util::TimeUtil;
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
    EXPECT_EQ(response.messages_size(), 0);
  }

  // 2 should have received the first message!
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1 + friend2.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), friend1.unique_name);
    EXPECT_EQ(response.messages(0).message(), "hello from 1 to 2");
    EXPECT_EQ(response.messages(1).from_unique_name(), friend1.unique_name);
    EXPECT_EQ(response.messages(1).message(), "INVITATION-MESSAGE");
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

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2 + friend2.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), friend1.unique_name);
    cout << "message 1: " << response.messages(0).message() << endl;

    cout << "message 1 time: "
         << TimeUtil::ToString(response.messages(0).delivered_at()) << endl;
    cout << "message 2: " << response.messages(1).message() << endl;
    cout << "message 2 time: "
         << TimeUtil::ToString(response.messages(1).delivered_at()) << endl;
    EXPECT_EQ(response.messages(0).message(),
              "hello from 1 to 2, again!!!! :0");

    EXPECT_EQ(response.messages(2).from_unique_name(), friend1.unique_name);
    EXPECT_EQ(response.messages(2).message(), "INVITATION-MESSAGE");
  }
};

}  // namespace
}  // namespace asphr::testing