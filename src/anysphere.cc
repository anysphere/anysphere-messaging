#include <grpcpp/grpcpp.h>

#include <ctime>
#include <iostream>
#include <string>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using messenger::Messenger;
using messenger::RegisterInfo;
using messenger::RegisterResponse;

std::string get_greet(const std::string& who) { return "Hello " + who; }

void print_localtime() {
  std::time_t result = std::time(nullptr);
  std::cout << std::asctime(std::localtime(&result));
}

void test(std::shared_ptr<Channel> channel,
          std::unique_ptr<Greeter::Stub> stub) {
  // Data we are sending to the server.
  RegisterInfo request;
  request.set_publicKey("hi_i_am_public_key");

  // Container for the data we expect from the server.
  RegisterResponse reply;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  // The actual RPC.
  Status status = stub->Register(&context, request, &reply);

  // Act upon its status.
  if (status.ok()) {
    return reply.message();
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return "RPC failed";
  }
}

int main(int argc, char** argv) {
  std::string server_address("0.0.0.0:50051");
  if (argc > 1) {
    server_address = argv[1];
  }
  std::cout << "Client querying server address: " << server_address
            << std::endl;

  std::shared_ptr<Channel> channel =
      grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());

  std::unique_ptr<Greeter::Stub> stub = Greeter::NewStub(channel);

  test(channel, stub);

  return 0;
}
