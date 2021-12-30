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
using messenger::RegisterInfo;
using messenger::RegisterResponse;

std::string get_greet(const std::string& who) { return "Hello " + who; }

void print_localtime() {
  std::time_t result = std::time(nullptr);
  std::cout << std::asctime(std::localtime(&result));
}

class MessengerServiceImpl final : public Messenger::Service {
  Status Register(ServerContext* context, const RegisterInfo* registerInfo,
                  RegisterResponse* registerResponse) override {
    std::cout << get_greet("world") << std::endl;

    // return empty Status
    return Status::OK;
  }
};

int main(int argc, char** argv) {
  std::string server_address("0.0.0.0:50051");
  if (argc > 1) {
    server_address = argv[1];
  }
  MessengerServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());

  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}
