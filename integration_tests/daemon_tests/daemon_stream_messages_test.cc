#include "../daemon_setup.hpp"

namespace asphr::testing {
namespace {

TEST_F(DaemonRpcTest, StreamMessages) {
  ResetStub();

  FriendTestingInfo friend1, friend2;
  std::tie(friend1, friend2) = generate_two_friends();

  {
    SendMessageRequest request;
    request.set_name("user1");
    request.set_message("FIRST");
    asphrdaemon::SendMessageResponse response;
    auto status = friend2.rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }

  {
    friend1.t->send_messages();
    friend2.t->send_messages();
  }

  {
    friend1.t->retrieve_messages();
    friend2.t->retrieve_messages();
  }

  // for streaming to work in tests we need to create a real stub
  const int rpc1_port = absl::Uniform(bitgen_, 10'000, 65'000);
  std::ostringstream rpc1_addr;
  rpc1_addr << "localhost:" << rpc1_port;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(rpc1_addr.str(), grpc::InsecureServerCredentials());
  builder.RegisterService(&(*friend1.rpc));
  auto rpc1_server = builder.BuildAndStart();
  std::shared_ptr<grpc::Channel> rpc1_channel =
      grpc::CreateChannel(rpc1_addr.str(), grpc::InsecureChannelCredentials());
  auto rpc1_stub = asphrdaemon::Daemon::NewStub(rpc1_channel);

  // lock-step the threads using a condition variable and a counter
  // to ensure that the messages are received in the correct order
  std::mutex m;
  std::condition_variable cv;
  int counter = 0;
  int max_rounds = 4;

  // we call the streaming rpc and the sending rpc in different threads
  std::thread receiveThread([&]() {
    // first receive the pre-sent messages
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    grpc::ClientContext context;
    auto stream = rpc1_stub->GetMessagesStreamed(&context, request);
    while (true) {
      std::unique_lock<std::mutex> lk(m);
      if (counter > max_rounds) {
        break;
      }
      if (counter % 2 == 0) {
        if (counter == 0) {
          GetMessagesResponse response;
          stream->Read(&response);
          EXPECT_EQ(response.messages_size(), 1);
          EXPECT_EQ(response.messages(0).from(), "user2");
          EXPECT_EQ(response.messages(0).m().message(), "FIRST");
        } else if (counter == 2) {
          {
            GetMessagesResponse response;
            stream->Read(&response);
            EXPECT_EQ(response.messages_size(), 1);
            EXPECT_EQ(response.messages(0).from(), "user2");
            EXPECT_EQ(response.messages(0).m().message(), "SECOND.first");
          }
          {
            GetMessagesResponse response;
            stream->Read(&response);
            EXPECT_EQ(response.messages_size(), 1);
            EXPECT_EQ(response.messages(0).from(), "user2");
            EXPECT_EQ(response.messages(0).m().message(), "SECOND.second");
          }
        } else if (counter == 4) {
          GetMessagesResponse response;
          stream->Read(&response);
          EXPECT_EQ(response.messages_size(), 1);
          EXPECT_EQ(response.messages(0).from(), "user2");
          EXPECT_EQ(response.messages(0).m().message(), "THIRD");
        }
        counter++;
        cv.notify_all();
      } else {
        cv.wait(lk, [&]() { return counter % 2 == 0; });
      }
    }
    context.TryCancel();
    auto status = stream->Finish();
  });

  std::thread sendThread([&]() {
    while (true) {
      std::unique_lock<std::mutex> lk(m);
      if (counter > max_rounds) {
        break;
      }
      if (counter % 2 == 1) {
        if (counter == 1) {
          {
            SendMessageRequest request;
            request.set_name("user1");
            request.set_message("SECOND.first");
            asphrdaemon::SendMessageResponse response;
            auto status =
                friend2.rpc->SendMessage(nullptr, &request, &response);
            EXPECT_TRUE(status.ok());
          }
          {
            SendMessageRequest request;
            request.set_name("user1");
            request.set_message("SECOND.second");
            asphrdaemon::SendMessageResponse response;
            auto status =
                friend2.rpc->SendMessage(nullptr, &request, &response);
            EXPECT_TRUE(status.ok());
          }
          {
            friend1.t->retrieve_messages();
            friend1.t->send_messages();
          }
          {
            friend2.t->retrieve_messages();
            friend2.t->send_messages();
          }
          {
            friend1.t->retrieve_messages();
            friend1.t->send_messages();
          }
          {
            friend2.t->retrieve_messages();
            friend2.t->send_messages();
          }
          {
            friend1.t->retrieve_messages();
            friend1.t->send_messages();
          }
        } else if (counter == 3) {
          SendMessageRequest request;
          request.set_name("user1");
          request.set_message("THIRD");
          asphrdaemon::SendMessageResponse response;
          auto status = friend2.rpc->SendMessage(nullptr, &request, &response);
          EXPECT_TRUE(status.ok());
          {
            friend1.t->retrieve_messages();
            friend1.t->send_messages();
          }
          {
            friend2.t->retrieve_messages();
            friend2.t->send_messages();
          }
          {
            friend1.t->retrieve_messages();
            friend1.t->send_messages();
          }
        }
        counter++;
        cv.notify_all();
      } else {
        cv.wait(lk, [&]() { return counter % 2 == 1; });
      }
    }
  });

  receiveThread.join();
  sendThread.join();

  // make sure we haven't messed up the original get all messages because
  // request that would be sad
  {
    GetMessagesRequest request;
    request.set_filter(GetMessagesRequest::ALL);
    GetMessagesResponse response;
    grpc::ClientContext context;
    auto status = rpc1_stub->GetMessages(&context, request, &response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.messages_size(), 4);
    EXPECT_EQ(response.messages(0).from(), "user2");
    EXPECT_EQ(response.messages(0).m().message(), "THIRD");
    EXPECT_EQ(response.messages(1).from(), "user2");
    EXPECT_EQ(response.messages(1).m().message(), "SECOND.second");
    EXPECT_EQ(response.messages(2).from(), "user2");
    EXPECT_EQ(response.messages(2).m().message(), "SECOND.first");
    EXPECT_EQ(response.messages(3).from(), "user2");
    EXPECT_EQ(response.messages(3).m().message(), "FIRST");
  }

  // a final message is needed to close the thread
  {
    SendMessageRequest request;
    request.set_name("user1");
    request.set_message("FOUR");
    asphrdaemon::SendMessageResponse response;
    auto status = friend2.rpc->SendMessage(nullptr, &request, &response);
    EXPECT_TRUE(status.ok());
  }
  {
    friend1.t->retrieve_messages();
    friend1.t->send_messages();
  }
  {
    friend2.t->retrieve_messages();
    friend2.t->send_messages();
  }
  {
    friend1.t->retrieve_messages();
    friend1.t->send_messages();
  }

  rpc1_server->Shutdown();
};

}  // namespace
}  // namespace asphr::testing