#include "common.h"

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
