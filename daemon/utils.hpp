//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include "asphr/asphr.hpp"

static auto write_msg_to_file(string file_address, string msg, string type,
                              string to) -> asphr::Status {
  auto file = std::ofstream(file_address, std::ios_base::app);

  auto time = absl::FormatTime(absl::Now(), absl::UTCTimeZone());
  json jmsg = {
      {"timestamp", time}, {"type", type}, {"message", msg}, {"to", to}};
  if (file.is_open()) {
    file << std::setw(4) << jmsg.dump() << std::endl;
    cout << "write msg to file: " << jmsg.dump() << endl;
    file.close();
    return absl::OkStatus();
  } else {
    return absl::UnknownError("file is not open");
  }
}

static auto get_entries(const string& file_address) -> vector<json> {
  auto test = json::array();
  auto entries = vector<json>();

  auto file = std::ifstream(file_address);
  string line;
  while (std::getline(file, line)) {
    auto j = json::parse(line);
    entries.push_back(j);
  }
  file.close();

  return entries;
}