#pragma once

#include "asphr/asphr.hpp"

using Msg = string;
using PublicKey = string;
using PrivateKey = string;
using Name = string;

using std::cout;
using std::endl;
using std::string;
using std::to_string;

using std::vector;

using json = nlohmann::json;

using absl::StrCat;
using absl::Time;

// if the message is this size or shorter, it is guaranteed to be sent in a
// single round. 1+5 is for the uint32 ID, 1+MESSAGE_SIZE is for the header of
// the string, and 1 + 1 + 5 is for the repeated acks containing at least one
// element. -1 at the end is for the padding which reserves one byte.
extern const size_t CRYPTO_ABYTES;
extern const size_t CRYPTO_NPUBBYTES;
extern const size_t GUARANTEED_SINGLE_MESSAGE_SIZE;

auto get_last_line(string filename);

auto get_last_lines(string filename, int n);

auto get_new_entries(const string& file_address, const Time& last_timestamp)
    -> vector<json>;

// Config directories.
extern const std::filesystem::path DAEMON_CONFIG_DIR;
extern const std::filesystem::path CLI_CONFIG_DIR;
extern const std::filesystem::path GUI_CONFIG_DIR;
extern const std::filesystem::path RUNTIME_DIR;
extern const std::filesystem::path DEFAULT_DATA_DIR;
extern const std::filesystem::path SOCKET_PATH;