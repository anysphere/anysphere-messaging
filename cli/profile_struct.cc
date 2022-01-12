#include "as_cli.hpp"

void Profile::add(unique_ptr<asphrdaemon::Daemon::Stub>& stub) {
  set_time();

  grpc::ClientContext context;
  asphrdaemon::RegisterUserRequest request;
  asphrdaemon::RegisterUserResponse reply;

  request.set_name(name_);

  grpc::Status status = stub->RegisterUser(&context, request, &reply);

  if (!status.ok()) {
    cout << "register user failed: " << status.error_message() << endl;
  } else if (!reply.success()) {
    cout << "register user failed from the daemon." << endl;
  } else {
    cout << "Registered as " << name_ << endl;
  }
}
