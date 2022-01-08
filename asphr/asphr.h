#pragma once

#include <stdlib.h>

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "absl/hash/hash.h"
#include "absl/random/random.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "base64.h"
#include "json/nlohmann_json.h"
#include "utils.h"

using std::array;
using std::cerr;
using std::cout;
using std::endl;
using std::make_shared;
using std::make_unique;
using std::shared_ptr;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;

namespace asphr {
using json = nlohmann::json;

using absl::InvalidArgumentError;
using absl::Status;
using absl::StatusOr;
using absl::StrCat;
using absl::Time;
}  // namespace asphr

#define CEIL_DIV(a, b) (((a) + (b)-1) / (b))

// MESSAGE_SIZE is the size of the message in bytes
constexpr size_t MESSAGE_SIZE = 1024;
constexpr size_t MESSAGE_SIZE_BITS = MESSAGE_SIZE * 8;

// pir_value is the type of the value stored at one index in the database
using pir_value_t = array<byte, MESSAGE_SIZE>;
// pir_index is the type of the index in the database. this should correspond to
// the index in the schema/messenger.proto definition
using pir_index_t = uint32_t;