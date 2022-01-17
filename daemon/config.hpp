#pragma once

#include "asphr/asphr.hpp"
#include "asphr/pir/fast_pir/fast_pir_client.hpp"

struct Friend {
  string name;
  int write_index;
  int read_index;
  string write_key;
  string read_key;
  bool enabled;

  auto to_json() -> asphr::json;
  static auto from_json(const asphr::json& j) -> Friend;
};

struct RegistrationInfo {
  string name;
  string public_key;
  string private_key;
  string authentication_token;
  vector<int> allocation;

  auto to_json() -> asphr::json;
  static auto from_json(const asphr::json& j) -> RegistrationInfo;
};

class Config {
 public:
  Config(const string& config_file_address);
  Config(const asphr::json& config_json, const string& config_file_address);

  auto save() -> void;
  auto add_friend(const Friend& friend_info) -> void;
  auto remove_friend(const string& name) -> absl::Status;

  auto has_space_for_friends() -> bool;

  // TODO: transition to .md files stored with metadata, rather than the giant
  // .ndjson files.
  auto receive_file_address() -> std::filesystem::path;
  auto send_file_address() -> std::filesystem::path;

  bool has_registered;
  RegistrationInfo registrationInfo;

  // db_rows may possibly be overriden in the config, but should either way
  // always be constant
  const size_t db_rows;
  // store secret key and galois keys for pir
  string pir_secret_key;
  // TODO(sualeh, urgent): make this private
  std::unordered_map<string, Friend> friendTable;
  string pir_galois_keys;
  // make this a ptr because we want it to possibly be null
  std::unique_ptr<FastPIRClient> pir_client = nullptr;
  // the data dir! user-configurable
  std::filesystem::path data_dir;

 private:
  // it stores its own file address
  string saved_file_address;
};