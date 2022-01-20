#pragma once

#include "asphr/asphr.hpp"
#include "asphr/pir/fast_pir/fast_pir_client.hpp"

struct Friend {
  Friend() = default;
  Friend(const string& name) : name(name), enabled(false) {}
  Friend(const Friend& f) = default;
  Friend& operator=(const Friend& f) = default;

  string name;
  int read_index;
  string write_key;
  string read_key;
  // ack_index is the index into the acking data for this friend
  // this NEEDS to be unique for every friend!!
  int ack_index;
  bool enabled;
  // latest_ack is the latest ID that was ACKed by the friend
  // any message with ID > latest_ack_id MUST be retried
  uint32_t latest_ack_id;
  // last_receive_id is the value that should be ACKed. we acknowledge that we
  // have received all IDs up to and including this value.
  uint32_t last_receive_id;

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

// TODO: MUTEX GUARD THIS CONFIG!!!
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
  // friendTable contains dummyMe!!! dummyMe is always disabled, but be careful!
  std::unordered_map<string, Friend> friendTable;
  string pir_galois_keys;
  // make this a ptr because we want it to possibly be null
  std::unique_ptr<FastPIRClient> pir_client = nullptr;
  // the data dir! user-configurable
  std::filesystem::path data_dir;
  // me is used whenever we need to encrypt dummy data!
  Friend dummyMe;

 private:
  // it stores its own file address
  string saved_file_address;
};