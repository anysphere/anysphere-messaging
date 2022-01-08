#pragma once
#include <grpcpp/grpcpp.h>

#include "asphr/asphr.h"
#include "client/client_lib/client_lib.h"
#include "schema/messenger.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using messenger::Messenger;
using messenger::RegisterInfo;
using messenger::RegisterResponse;

constexpr auto SEND_FILE = "/workspace/anysphere/client/logs/send.ndjson";
constexpr auto RECEIVE_FILE = "/workspace/anysphere/client/logs/receive.ndjson";
constexpr auto CONFIG_FILE = "/workspace/anysphere/client/logs/config.json";

auto write_msg_to_file(string file_address, string msg, string type, string to)
    -> asphr::Status {
  auto file = std::ofstream(file_address, std::ios_base::app);

  auto time = absl::FormatTime(absl::Now(), utc);
  json jmsg = {
      {"timestamp", time}, {"type", type}, {"message", msg}, {"to", to}};
  if (file.is_open()) {
    file << std::setw(4) << jmsg.dump() << std::endl;
    cout << "write msg to file: " << jmsg.dump() << endl;
    file.close();
    return asphr::Status::OK;
  } else {
    return absl::UnknownError("file is not open");
  }
}