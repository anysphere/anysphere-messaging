#include "client_lib.hpp"

#include <sodium.h>

extern constexpr size_t CRYPTO_ABYTES =
    crypto_aead_xchacha20poly1305_ietf_ABYTES;
extern constexpr size_t CRYPTO_NPUBBYTES =
    crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
extern constexpr size_t GUARANTEED_SINGLE_MESSAGE_SIZE =
    MESSAGE_SIZE - (1 + 5) -
    (1 +
     CEIL_DIV((sizeof MESSAGE_SIZE) * 8 - std::countl_zero(MESSAGE_SIZE), 8)) -
    (1 + 1 + 5) - CRYPTO_ABYTES - 1 - CRYPTO_NPUBBYTES;

const auto cwd = string("/Users/arvid/code/anysphere/");
extern const string DEFAULT_SOCKET_ADDRESS =
    StrCat("unix://", cwd, "anysphere.sock");

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