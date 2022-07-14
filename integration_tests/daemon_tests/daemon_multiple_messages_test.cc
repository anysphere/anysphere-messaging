#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, SendMultipleMessagesInBothDirections) {
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

  {
    SendMessageRequest request;
    request.add_unique_name(friend1.unique_name);
    request.set_message("hello from 2 to 1");
    asphrdaemon::SendMessageResponse response;
    auto status = friend2.rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  // retrieve-send is how messages are propagated!
  friend1.t->retrieve();
  friend1.t->send();

  friend2.t->retrieve();
  friend2.t->send();

  // 1 can impossibly receive anything!!
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

  // 1 should have received the first message by now
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend1.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 1);
    EXPECT_EQ(response.messages(0).from_unique_name(), "user2");
    EXPECT_EQ(response.messages(0).message(), "hello from 2 to 1");
  }

  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    auto status = friend2.rpc->GetMessages(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 2 + friend2.extra_messages);
    EXPECT_EQ(response.messages(0).from_unique_name(), "user1");
    cout << "message 1: " << response.messages(0).message() << endl;

    cout << "message 1 time: "
         << TimeUtil::ToString(response.messages(0).delivered_at()) << endl;
    cout << "message 2: " << response.messages(1).message() << endl;
    cout << "message 2 time: "
         << TimeUtil::ToString(response.messages(1).delivered_at()) << endl;
    EXPECT_EQ(response.messages(0).message(),
              "hello from 1 to 2, again!!!! :0");
  }
};

}  // namespace
}  // namespace asphr::testing