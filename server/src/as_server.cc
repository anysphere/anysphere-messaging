#include <grpcpp/grpcpp.h>

#include <ctime>
#include <iostream>
#include <memory>
#include <string>

#ifdef BAZEL_BUILD
#include "schema/messenger.grpc.pb.h"
#else
#include "schema/messenger.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using messenger::Messenger;

// draft of structure:
//    - there are two kinds of databases. one contains all the messages and only the messages, and is the one
//      we do PIR on. the other contains auxiliary non-private information, like (authentication_token -> allocation) map,
//      and possibly other things. we care a lot about the latter database to be persistent, whereas the former
//      need not be persistent. so probably what we want is a postgres instance for storing all non-private info,
//      and an in-memory / custom solution to performing the PIR (when memory usage becomes a problem, split the PIR
//      into multiple shards, and perform the queries on each shard)
//    -

// TODO: look into AsyncService; might be useful for performance
class MessengerImpl final : public Messenger::Service
{
  // TODO: add a thread safety argument (because the methods may be called from different threads)
  // TODO: add representation invariant

  Status Register(ServerContext *context, const messenger::RegisterInfo *registerInfo,
                  messenger::RegisterResponse *registerResponse) override
  {
    std::cout << "world" << std::endl;

    // return empty Status
    return Status::OK;
  }

  Status SendMessage(ServerContext *context, const messenger::SendMessageInfo *sendMessageInfo,
                     messenger::SendMessageResponse *sendMessageResponse) override
  {
      // check that the authentication token corresponds to the index
      // check that the message is not too long
    std::cout << "world" << std::endl;

    // return empty Status
    return Status::OK;
  }

  Status ReceiveMessage(ServerContext *context, const messenger::ReceiveMessageInfo *receiveMessageInfo,
                        messenger::ReceiveMessageResponse *receiveMessageResponse) override
  {
    std::cout << "world" << std::endl;

    // return empty Status
    return Status::OK;
  }
};

int main(int argc, char **argv)
{
  std::string server_address("0.0.0.0:50051");
  if (argc > 1)
  {
    server_address = argv[1];
  }
  MessengerImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());

  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}
