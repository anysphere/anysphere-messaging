#include "asphr/asphr.h"

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
absl::BitGen gen_;

absl::TimeZone utc = absl::UTCTimeZone();

// TODO(sualeh): make this work
//  const auto WORKSPACE_DIR = string("/workspace/anysphere/client/logs/");
//  const auto UI_FILE = WORKSPACE_DIR + string("ui.ndjson");
//  const auto UI_URGENT_FILE = WORKSPACE_DIR + string("ui_urgent.ndjson");
//  const auto CLIENT_FILE = WORKSPACE_DIR + string("client.ndjson");
//  const auto CONFIG_FILE = WORKSPACE_DIR + string("config.json");

constexpr auto UI_FILE = "/workspace/anysphere/client/logs/ui.ndjson";
constexpr auto UI_URGENT_FILE =
    "/workspace/anysphere/client/logs/ui_urgent.ndjson";
constexpr auto CLIENT_FILE = "/workspace/anysphere/client/logs/client.ndjson";
constexpr auto CONFIG_FILE = "/workspace/anysphere/client/logs/config.ndjson";

/**
 * @brief This function gets the last line of a file
 * @property: Nonconsuming: It does not consume the charachters, it only scans
 * them.
 */
auto get_last_line(string filename) {
  std::ifstream fin;
  fin.open(filename);
  if (fin.is_open()) {
    fin.seekg(-1, std::ios_base::end);  // go to one spot before the EOF

    char ch = fin.get();
    // if we end with a newline, we go back one more
    while (ch == '\n') {
      fin.seekg(-2, std::ios_base::cur);  // go to the second last char
      ch = fin.get();
    }

    bool keepLooping = true;
    while (keepLooping) {
      if ((int)fin.tellg() <= 1) {  // If the data was at or before the 0th byte
        fin.seekg(0);               // The first line is the last line
        keepLooping = false;        // So stop there
      } else if (ch == '\n') {      // If the data was a newline
        keepLooping = false;        // Stop at the current position.
      } else {  // If the data was neither a newline nor at the 0 byte
        fin.seekg(-2,
                  std::ios_base::cur);  // Move to the front of that data, then
                                        // to the front of the data before it
      }

      fin.get(ch);  // update the current byte's data
    }

    fin.seekg(-1, std::ios_base::cur);  // go to the last char
    string lastLine;
    getline(fin, lastLine);  // Read the current line
    return lastLine;
  }
  return string();
}

auto get_last_lines(string filename, int n) {
  // TODO(sualeh): still a bit buggy.
  std::ifstream fin;
  fin.open(filename);
  vector<string> lines;
  if (fin.is_open()) {
    fin.seekg(-1, std::ios_base::end);  // go to one spot before the EOF

    char ch = fin.get();
    // if we end with a newline, we go back one more
    while (ch == '\n') {
      fin.seekg(-2, std::ios_base::cur);  // go to the second last char
      ch = fin.get();
    }

    int lines_found = 0;
    bool early_exit = false;

    while (lines_found < n && !early_exit) {
      if ((int)fin.tellg() <= 1) {  // If the data was at or before the 0th byte
        fin.seekg(0);               // The first line is the last line

        early_exit = true;
      } else if (ch == '\n') {  // If the data was a newline
        lines_found++;
        fin.seekg(-2, std::ios_base::cur);  // continue going back
      } else {  // If the data was neither a newline nor at the 0 byte
        // Move to the front of that data, then
        // to the front of the data before it
        fin.seekg(-2, std::ios_base::cur);
      }

      fin.get(ch);  // update the current byte's data
    }

    string line;
    while (getline(fin, line)) {
      lines.push_back(line);
      cout << "here and found line: " << line << endl;
    }
    while (lines_found < n) {
      lines.push_back("");
      lines_found++;
    }

    for (auto& line : lines) {
      cout << line << endl;
    }

    fin.close();
  }
  return lines;
}

auto get_new_entries(const string& file_address, const Time& last_timestamp)
    -> vector<json> {
  auto test = json::array();
  auto new_entries = vector<json>();

  auto file = std::ifstream(file_address);
  string line;
  while (std::getline(file, line)) {
    if (line == "") {
      continue;
    }
    auto j = json::parse(line);
    string jt = j["timestamp"].get<string>();
    Time jt_time;
    string err;
    absl::ParseTime(absl::RFC3339_full, jt, &jt_time, &err);
    if (err != "") {
      cout << "error parsing time: " << err << endl;
      continue;
    }
    if (jt_time > last_timestamp) {
      new_entries.push_back(j);
    }
  }
  file.close();

  return new_entries;
}

auto get_entries(const string& file_address) -> vector<json> {
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

auto write_msg_to_file(string file_address, string msg, string type) -> void {
  auto file = std::ofstream(file_address, std::ios_base::app);

  auto time = absl::FormatTime(absl::Now(), utc);
  json jmsg = {{"timestamp", time}, {"type", type}, {"message", msg}};
  if (file.is_open()) {
    file << std::setw(4) << jmsg.dump() << std::endl;
    cout << jmsg.dump() << endl;
    file.close();
  }
}

auto write_msg_to_file(string file_address, string msg, string type, string to)
    -> void {
  auto file = std::ofstream(file_address, std::ios_base::app);

  auto time = absl::FormatTime(absl::Now(), utc);
  json jmsg = {
      {"timestamp", time}, {"type", type}, {"message", msg}, {"to", to}};
  if (file.is_open()) {
    file << std::setw(4) << jmsg.dump() << std::endl;
    cout << "write msg to file: " << jmsg.dump() << endl;
    file.close();
  }
}

auto write_msg_to_file(string type, string file_address, Msg msg,
                       Name friend_name, Time time) -> void {
  auto file = std::ofstream(file_address, std::ios_base::app);

  auto send_time = absl::FormatTime(time, utc);

  json jmsg = {{"timestamp", send_time},
               {"type", type},
               {"message", msg},
               {"to", friend_name}};
  if (file.is_open()) {
    file << std::setw(4) << jmsg.dump() << std::endl;
    cout << jmsg.dump() << endl;
    file.close();
  }
}

auto write_friend_to_file(string file_address, const Name friend_name,
                          const int& write_index, const int& read_index,
                          const string& shared_key, Time time) -> void {
  auto file = std::ofstream(file_address, std::ios_base::app);

  auto send_time = absl::FormatTime(time, utc);
  json jmsg = {{"timestamp", send_time},   {"type", "FRIEND"},
               {"name", friend_name},      {"write_index", write_index},
               {"read_index", read_index}, {"shared_key", shared_key}};
  if (file.is_open()) {
    file << std::setw(4) << jmsg.dump() << std::endl;
    cout << jmsg.dump() << endl;
    file.close();
  }
}

auto register_profile_to_file(Name name, PublicKey key, PrivateKey private_key,
                              Time time) -> void {
  const auto file_address = UI_URGENT_FILE;
  auto file = std::ofstream(file_address, std::ios_base::app);

  auto send_time = absl::FormatTime(time, utc);
  json jmsg = {{"timestamp", send_time},
               {"type", "REGISTER"},
               {"name", name},
               {"public_key", key},
               {"private_key", private_key}};
  if (file.is_open()) {
    file << std::setw(4) << jmsg.dump() << std::endl;
    cout << jmsg.dump() << endl;
    file.close();
  }
}

auto read_friend_messages_from_file(Name friend_name, size_t number)
    -> vector<json> {
  const auto file_address = CLIENT_FILE;
  auto file = std::ifstream(file_address);
  vector<json> messages;
  string line;
  while (std::getline(file, line)) {
    auto j = json::parse(line);
    if (j["type"].get<string>() == "MESSAGE_RECEIVED" &&
        j["from"].get<string>() == friend_name) {
      messages.push_back(j);
    }
  }
  file.close();

  // cut off the last n messages
  if (messages.size() > number) {
    messages.erase(messages.begin(),
                   messages.begin() + messages.size() - number);
  }
  return messages;
}

auto read_friends_from_file() -> vector<json> {
  const auto file_address = CONFIG_FILE;
  auto file = std::ifstream(file_address);
  vector<json> friends;
  string line;
  while (std::getline(file, line)) {
    auto j = json::parse(line);
    if (j["type"].get<string>() == "FRIEND") {
      friends.push_back(j);
    }
  }
  file.close();
  return friends;
}
