#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <chrono>

#include <ctime>
#include <thread>
#include <vector>

#include <fstream>

#include "absl/time/time.h"
#include "absl/strings/str_cat.h"

#include "nlohmann_json.h"

// #include <grpcpp/grpcpp.h>
// #include "schema/messenger.grpc.pb.h"

using std::string;
using std::chrono::system_clock;
using time_point = system_clock::time_point;
using std::cout;
using std::endl;

using std::vector;

using json = nlohmann::json;

using absl::StrCat;
using absl::Time;

auto get_new_entries(string file_address, time_point last_timestamp) -> vector<json> {
  cout << "here" << endl;
  auto last_timestamp_s = last_timestamp.time_since_epoch().count();
  auto test = json::array();
  auto new_entries = vector<json>();
  auto file = std::ifstream(file_address);

  auto j = json({});
  if (file.is_open()) {
    string line;

    file >> j;
    cout << j << endl;
    file >> j;
    cout << j << endl;
    file >> j;
    cout << j << endl;

    // while (true) {
    //   std::getline(file, line);
    //   cout << line << endl;
    //   auto j = json::parse(line);
    //   auto timestamp = j["timestamp"];
    //   if (timestamp > last_timestamp_s) {
    //     new_entries.push_back(line);
    //   }
    // }
    // file.close();
  }
  return new_entries;
}

auto write_msg_to_file(string file_address, string msg) -> void {
  auto file = std::ofstream(file_address, std::ios_base::app);

  json jmsg = {
    {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()},
    {"type", "register"},
    {"msg", msg}
  };
  if (file.is_open()) {
    file << std::setw(4) << jmsg.dump() << std::endl;
    file.close();
  }
}
