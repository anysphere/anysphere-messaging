//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

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

// MAC bytes
extern const size_t CRYPTO_ABYTES;
// nonce bytes
extern const size_t CRYPTO_NPUBBYTES;
// the maximum size of a message such that it can be sent in a single message
extern const size_t GUARANTEED_SINGLE_MESSAGE_SIZE;

auto get_last_line(string filename);

auto get_last_lines(string filename, int n);

auto get_new_entries(const string& file_address, const Time& last_timestamp)
    -> vector<json>;

auto get_base_config_dir() noexcept(false) -> std::filesystem::path;
// TODO(arvid): file permissions should be ONLY daemon
auto get_daemon_config_dir() noexcept(false) -> std::filesystem::path;
// TODO(arvid): file permissions should be ONLY cli
auto get_cli_config_dir() noexcept(false) -> std::filesystem::path;
// TODO(arvid): file permissions should be ONLY gui
auto get_gui_config_dir() noexcept(false) -> std::filesystem::path;
// TODO(arvid): file permissions should be ONLY daemon, cli, gui (not any
// other users)
auto get_runtime_dir() noexcept(false) -> std::filesystem::path;
auto get_socket_path() noexcept(false) -> std::filesystem::path;
// TODO(arvid): file permissions should be daemon, cli, gui, AND the user.
auto get_default_data_dir() noexcept(false) -> std::filesystem::path;