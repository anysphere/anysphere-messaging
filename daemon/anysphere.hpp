#pragma once
#include <grpcpp/grpcpp.h>

#include "asphr/asphr.hpp"
#include "client/client_lib/client_lib.hpp"
#include "crypto.hpp"
#include "schema/server.grpc.pb.h"

constexpr auto SEND_FILE = "/workspace/anysphere/client/logs/send.ndjson";
constexpr auto RECEIVE_FILE = "/workspace/anysphere/client/logs/receive.ndjson";
constexpr auto CONFIG_FILE = "/workspace/anysphere/client/logs/config.json";
