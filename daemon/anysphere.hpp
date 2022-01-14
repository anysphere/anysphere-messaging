#pragma once
#include <grpcpp/grpcpp.h>

#include "asphr/asphr.hpp"
#include "client/client_lib/client_lib.hpp"
#include "crypto.hpp"
#include "daemon_rpc.hpp"
#include "schema/server.grpc.pb.h"

const auto cwd = string("/workspace/anysphere/");
const auto SEND_FILE = StrCat(cwd, "client/logs/send.ndjson");
const auto RECEIVE_FILE = StrCat(cwd, "client/logs/receive.ndjson");
const auto CONFIG_FILE = StrCat(cwd, "client/logs/config.json");