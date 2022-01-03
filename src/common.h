#pragma once

#include <grpcpp/grpcpp.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "absl/hash/hash.h"
#include "absl/random/random.h"
#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "nlohmann_json.h"
#include "schema/messenger.grpc.pb.h"

using std::cout;
using std::endl;
using std::string;
using std::to_string;

using std::vector;

using json = nlohmann::json;

using absl::StrCat;
using absl::Time;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using messenger::Messenger;
using messenger::RegisterInfo;
using messenger::RegisterResponse;

absl::BitGen gen_;
absl::TimeZone utc = absl::UTCTimeZone();

constexpr auto UI_FILE = "/workspace/anysphere/logs/ui.ndjson";
constexpr auto UI_URGENT_FILE = "/workspace/anysphere/logs/ui_urgent.ndjson";
constexpr auto CLIENT_FILE = "/workspace/anysphere/logs/client.ndjson";
constexpr auto CONFIG_FILE = "/workspace/anysphere/logs/config.ndjson";

/**
 * @brief This function gets the last line of a file
 * @property: Nonconsuming: It does not consume the charachters, it only scans
 * them.
 */
auto get_last_line(string filename)
{
  std::ifstream fin;
  fin.open(filename);
  if (fin.is_open())
  {
    fin.seekg(-1, std::ios_base::end); // go to one spot before the EOF

    char ch = fin.get();
    // if we end with a newline, we go back one more
    while (ch == '\n')
    {
      fin.seekg(-2, std::ios_base::cur); // go to the second last char
      ch = fin.get();
    }

    bool keepLooping = true;
    while (keepLooping)
    {
      if ((int)fin.tellg() <= 1)
      {                      // If the data was at or before the 0th byte
        fin.seekg(0);        // The first line is the last line
        keepLooping = false; // So stop there
      }
      else if (ch == '\n')
      {                      // If the data was a newline
        keepLooping = false; // Stop at the current position.
      }
      else
      { // If the data was neither a newline nor at the 0 byte
        fin.seekg(-2,
                  std::ios_base::cur); // Move to the front of that data, then
                                       // to the front of the data before it
      }

      fin.get(ch); // update the current byte's data
    }

    fin.seekg(-1, std::ios_base::cur); // go to the last char
    string lastLine;
    getline(fin, lastLine); // Read the current line
    return lastLine;
  }
}

auto get_last_lines(string filename, int n)
{
  // TODO(sualeh): still a bit buggy.
  std::ifstream fin;
  fin.open(filename);
  if (fin.is_open())
  {
    fin.seekg(-1, std::ios_base::end); // go to one spot before the EOF

    char ch = fin.get();
    // if we end with a newline, we go back one more
    while (ch == '\n')
    {
      fin.seekg(-2, std::ios_base::cur); // go to the second last char
      ch = fin.get();
    }

    int lines_found = 0;
    bool early_exit = false;

    vector<string> lines;
    while (lines_found < n && !early_exit)
    {
      if ((int)fin.tellg() <= 1)
      {               // If the data was at or before the 0th byte
        fin.seekg(0); // The first line is the last line

        early_exit = true;
      }
      else if (ch == '\n')
      { // If the data was a newline
        lines_found++;
        fin.seekg(-2, std::ios_base::cur); // continue going back
      }
      else
      { // If the data was neither a newline nor at the 0 byte
        // Move to the front of that data, then
        // to the front of the data before it
        fin.seekg(-2, std::ios_base::cur);
      }

      fin.get(ch); // update the current byte's data
    }

    string line;
    while (getline(fin, line))
    {
      lines.push_back(line);
      cout << "here and found line: " << line << endl;
    }
    while (lines_found < n)
    {
      lines.push_back("");
      lines_found++;
    }

    for (auto &line : lines)
    {
      cout << line << endl;
    }
    return lines;

    fin.close();
  }
}

auto get_new_entries(const string &file_address, const Time &last_timestamp)
    -> vector<json>
{
  auto test = json::array();
  auto new_entries = vector<json>();

  auto file = std::ifstream(file_address);
  string line;
  while (std::getline(file, line))
  {
    auto j = json::parse(line);
    string jt = j["timestamp"].get<string>();
    Time jt_time = absl::FromUnixSeconds(std::stoull(jt));
    if (jt_time > last_timestamp)
    {
      new_entries.push_back(j);
    }
  }
  file.close();

  // cout << "finding a new entry:" << endl;
  // auto last_line = get_last_lines(file_address, 3);

  // for (auto& line : last_line) {
  //   cout << line << endl;

  //   // auto entry = json::parse(line);
  //   // new_entries.push_back(entry);
  //   return new_entries;
  // }

  // if (last_line.size() > 0) {
  //   auto j = json({});
  //   j = json::parse(last_line);

  //   cout << j << endl;

  //   auto last_timestamp_j = j["timestamp"];

  //   cout << last_timestamp_j << endl;
  //   auto last_timestamp_j_s = last_timestamp_j.get<long>();
  //   if (last_timestamp_j_s > last_timestamp_s) {
  //     new_entries.push_back(j);
  //   }
  // } else {
  //   cout << "no new entries" << endl;
  // }

  return new_entries;
}

auto get_entries(const string &file_address) -> vector<json>
{
  auto test = json::array();
  auto entries = vector<json>();

  auto file = std::ifstream(file_address);
  string line;
  while (std::getline(file, line))
  {
    auto j = json::parse(line);
    entries.push_back(j);
  }
  file.close();

  return entries;
}

auto write_msg_to_file(string file_address, string msg, string type, string to) -> void
{
  auto file = std::ofstream(file_address, std::ios_base::app);

  auto time = absl::FormatTime(absl::Now(), utc);
  json jmsg = {{"timestamp", time}, {"type", type}, {"message", msg}, {"to", to}};
  if (file.is_open())
  {
    file << std::setw(4) << jmsg.dump() << std::endl;
    cout << jmsg.dump() << endl;
    file.close();
  }
}

struct Friend
{
  Friend() = default;
  Friend(string name, int write_index, int read_index, string shared_key)
      : name(name),
        write_index(write_index),
        read_index(read_index),
        shared_key(shared_key){};

  string name;
  int write_index;
  int read_index;
  string shared_key;
};

struct RegisterationInfo
{
  RegisterationInfo() = default;
  RegisterationInfo(string name, string public_key, string private_key)
      : name(name), public_key(public_key), private_key(private_key){};

  string name;
  string public_key;
  string private_key;
  string authentication_token;
  vector<int> allocation;
};

static auto RegistrationInfo = RegisterationInfo();
static auto FriendTable = std::unordered_map<string, Friend>();
