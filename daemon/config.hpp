#pragma once

#include "asphr/asphr.hpp"
#include "asphr/pir/fast_pir/fast_pir_client.hpp"
#include "constants.hpp"
#include "friend.hpp"

struct RegistrationInfo {
  string name;
  string public_key;
  string private_key;
  string authentication_token;
  // NOTE: OUR CURRENT CODE ONLY SUPPORTS ALLOCATION.SIZE() == 1
  // if we want to support bigger allocations in the future, it is very
  // important to carefully think about the code!
  vector<int> allocation;

  auto to_json() -> asphr::json;
  static auto from_json(const asphr::json& j) -> RegistrationInfo;
};

/**
 * @brief The Config class
 *
 * Note: almost all accesses to this class are guarded by config_mutex
 * std::lock_guard<std::mutex> l(config_mtx);
 */
class Config {
 public:
  Config(const string& config_file_address);
  Config(const asphr::json& config_json, const string& config_file_address);

  // precondition: friend_info.name is not in friendTable
  auto add_friend(const Friend& friend_info) -> void;
  auto remove_friend(const string& name) -> asphr::Status;
  auto friends() const -> vector<Friend>;
  auto get_friend(const string& name) const -> asphr::StatusOr<Friend>;
  // precondition: f.name is in friendTable
  auto update_friend(const Friend& f) -> void;

  auto has_space_for_friends() -> bool;
  auto num_enabled_friends() -> int;
  auto random_enabled_friend() -> asphr::StatusOr<Friend>;
  auto dummy_me() -> Friend;

  auto has_registered() -> bool;
  auto do_register(const string& name, const string& public_key,
                   const string& private_key,
                   const string& authentication_token,
                   const vector<int>& allocation) -> void;
  auto registration_info() -> RegistrationInfo;

  // TODO: transition to .md files stored with metadata, rather than the giant
  // .ndjson files.
  auto receive_file_address() -> std::filesystem::path;
  auto send_file_address() -> std::filesystem::path;
  auto seen_file_address() -> std::filesystem::path;
  auto data_dir_address() -> std::filesystem::path;
  auto server_address() -> std::string;

  auto pir_client() -> FastPIRClient&;
  auto db_rows() -> size_t;

 private:
  // it stores its own file address
  string saved_file_address;
  bool has_registered_;
  RegistrationInfo registrationInfo;

  // db_rows may possibly be overriden in the config, but should either way
  // always be constant
  const size_t db_rows_;
  // store secret key and galois keys for pir
  string pir_secret_key;
  // store server address
  string server_address_;
  // friendTable DOES NOT CONTAIN dummyMe
  std::unordered_map<string, Friend> friendTable;
  string pir_galois_keys;
  // make this a ptr because we want it to possibly be null
  std::unique_ptr<FastPIRClient> pir_client_ = nullptr;
  // the data dir! user-configurable
  std::filesystem::path data_dir;
  // me is used whenever we need to encrypt dummy data!
  Friend dummyMe;
  mutable std::mutex config_mtx;

  auto check_rep() const -> void;
  auto initialize_dummy_me() -> void;

  auto save() -> void;
};