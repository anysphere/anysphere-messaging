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