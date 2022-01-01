#include <grpcpp/grpcpp.h>

#include <ctime>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <thread>

#include "schema/messenger.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using messenger::Messenger;
using messenger::RegisterInfo;
using messenger::RegisterResponse;

// needs to store local data that is persisted between rounds, like:
// - whether registered or not
// - authentication token and allocation indices
// - which friends are assigned to which indices
// can we store this in sqlite?

int test(std::shared_ptr<Channel> channel,
         std::unique_ptr<Messenger::Stub> stub) {
  // Data we are sending to the server.
  RegisterInfo request;
  // request.set_publickey("hi_i_am_public_key");

  // Container for the data we expect from the server.
  RegisterResponse reply;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  // The actual RPC.
  Status status = stub->Register(&context, request, &reply);

  // Act upon its status.
  if (status.ok()) {
    return 1;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return 0;
  }
}

int main(int argc, char **argv) {
  std::string server_address("0.0.0.0:50051");
  if (argc > 1) {
    server_address = argv[1];
  }
  std::cout << "Client querying server address: " << server_address
            << std::endl;

  // std::shared_ptr<Channel> channel =
  //     grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());

  // std::unique_ptr<Messenger::Stub> stub = Messenger::NewStub(channel);

  // test(channel, std::move(stub));
  
  // get the local time using the system clock
  auto t = std::chrono::system_clock().now();

  // get a duration using chrono
  auto duration = std::chrono::milliseconds(500);

  while (true) {
    // get the time difference from t
    auto now = std::chrono::system_clock().now();
    auto diff = now - t;

    std::cout << "I have been running for " << diff.count()/1000 << "ms" << std::endl;
    // sleep for 500 milliseconds
    std::this_thread::sleep_for(duration);
  }

  return 0;
}
