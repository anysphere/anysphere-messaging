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

auto get_base_config_dir() noexcept(false) -> std::filesystem::path;
auto get_daemon_config_dir() noexcept(false) -> std::filesystem::path;
auto get_cli_config_dir() noexcept(false) -> std::filesystem::path;
auto get_gui_config_dir() noexcept(false) -> std::filesystem::path;
auto get_runtime_dir() noexcept(false) -> std::filesystem::path;
auto get_default_data_dir() noexcept(false) -> std::filesystem::path;

// Config directories.
// TODO(arvid): file permissions should be ONLY daemon
inline const auto DAEMON_CONFIG_DIR = get_daemon_config_dir();
// TODO(arvid): file permissions should be ONLY cli
inline const auto CLI_CONFIG_DIR = get_cli_config_dir();
// TODO(arvid): file permissions should be ONLY gui
inline const auto GUI_CONFIG_DIR = get_gui_config_dir();
// TODO(arvid): file permissions should be ONLY daemon, cli, gui (not any
// other users)
inline const auto RUNTIME_DIR = get_runtime_dir();
// TODO(arvid): file permissions should be daemon, cli, gui, AND the user.
inline const auto DEFAULT_DATA_DIR = get_default_data_dir();

inline const auto SOCKET_PATH = RUNTIME_DIR / "anysphere.sock";