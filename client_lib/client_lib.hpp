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