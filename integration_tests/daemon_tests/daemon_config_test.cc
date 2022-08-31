#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, LoadAndUnloadConfigAndReceive) {
  ResetStub();
  auto db_file1 = generateTempFile();
  auto db_file2 = generateTempFile();

  { generate_two_friends(db_file1, db_file2); }

  {
    // re-create db from the file!
    auto [G1, rpc1, t1] = gen_person(db_file1);
    auto [G2, rpc2, t2] = gen_person(db_file2);

    {
      SendMessageRequest request;
      request.add_unique_name("user2");
      request.set_message("hello from 1 to 2");
      asphrdaemon::SendMessageResponse response;
      auto status = rpc1->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      SendMessageRequest request;
      request.add_unique_name("user1");
      request.set_message("hello from 2 to 1");
      asphrdaemon::SendMessageResponse response;
      auto status = rpc2->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    {
      t1->send();
      t2->send();
    }

    {
      t1->retrieve();
      cout << "-----------------------------" << endl;
      t2->retrieve();
    }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc1->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 1);
      EXPECT_EQ(response.messages(0).from_unique_name(), "user2");
      EXPECT_EQ(response.messages(0).message(), "hello from 2 to 1");
    }

    {
      GetMessagesRequest request;
      request.set_filter(GetMessagesRequest::ALL);
      GetMessagesResponse response;
      auto status = rpc2->GetMessages(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(response.messages_size(), 2);  // +1 for invitation message
      EXPECT_EQ(response.messages(0).from_unique_name(), "user1");
      EXPECT_EQ(response.messages(0).message(), "hello from 1 to 2");
    }
  }
}

}  // namespace
}  // namespace asphr::testing