#pragma once

#include "asphr/asphr.h"
#include "asphr/pir/fast_pir/fast_pir_client.h"

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
  // the client needs to know roughly how many rows are in the database.
  // here we cache the latest number of db_rows we received
  auto to_json() -> asphr::json;
  static auto from_json(const asphr::json& j) -> RegistrationInfo;
};

class Config {
 public:
  Config(const string& config_file_address);
  Config(const asphr::json& config_json);

  auto save(const string& config_file_address) -> void;

  auto has_space_for_friends() -> bool;

  bool has_registered;
  RegistrationInfo registrationInfo;
  std::unordered_map<string, Friend> friendTable;
  // TODO: move this out of registrationinfo?
  int db_rows;
  // store secret key and galois keys for pir
  string pir_secret_key;
  string pir_galois_keys;
  // make this a ptr because we want it to possibly be null
  std::unique_ptr<FastPIRClient> pir_client = nullptr;
};

struct EphemeralConfig {
  string config_file_address;
  string send_messages_file_address;
  string received_messages_file_address;
};