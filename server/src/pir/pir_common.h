#pragma once

#include <cstddef>
#include <array>
#include "anysphere/utils.h"

using std::array;

// MESSAGE_SIZE is the size of the message in bytes
constexpr size_t MESSAGE_SIZE = 1024;
constexpr size_t MESSAGE_SIZE_BITS = MESSAGE_SIZE * 8;

// pir_value is the type of the value stored at one index in the database
using pir_value_t = array<byte, MESSAGE_SIZE>;
// pir_index is the type of the index in the database. this should correspond to the index in the schema/messenger.proto definition
using pir_index_t = uint32_t;