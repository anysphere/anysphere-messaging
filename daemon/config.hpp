#pragma once

#include "asphr/asphr.hpp"
#include "asphr/pir/fast_pir/fast_pir_client.hpp"
#include "constants.hpp"

struct Friend {
  using FriendTable = std::unordered_map<string, Friend>;
  Friend() = default;
  Friend(const string& name, const FriendTable& ft)
      : name(name), enabled(false) {
    ack_index = 0;

    auto rng = std::default_random_engine{};

    auto all_ack_indexes_not_used = asphr::unordered_set<int>{};
    for (size_t i = 0; i < MAX_FRIENDS; i++) {
      all_ack_indexes_not_used.insert(i);
    }

    // get all the ack_indexes from the FriendTable
    for (const auto& f : ft) {
      all_ack_indexes_not_used.erase(f.second.ack_index);
    }

    // get a random ack_index that is not used
    std::uniform_int_distribution<int> dist(
        0, all_ack_indexes_not_used.size() - 1);
    auto random_ack_index = dist(rng);
    // TODO: do this a better way
    ack_index = *std::next(all_ack_indexes_not_used.begin(), random_ack_index);
  }
  Friend(const string& name, const int read_index, const string& read_key,
         const string& write_key, const int ack_index, const bool enabled,
         const uint32_t latest_ack_id, const uint32_t last_receive_id)
      : name(name),
        read_index(read_index),
        read_key(read_key),
        write_key(write_key),
        ack_index(ack_index),
        enabled(enabled),
        latest_ack_id(latest_ack_id),
        last_receive_id(last_receive_id) {}

  Friend(const Friend& f) = default;
  Friend& operator=(const Friend& f) = default;

  string name;
  int read_index;
  string read_key;
  string write_key;
  // ack_index is the index into the acking data for this friend
  // this NEEDS to be unique for every friend!!
  // This needs to be between 0 <= ack_index < MAX_FRIENDS
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
  auto initialize_dummy_me() -> void;

  bool has_registered;
  RegistrationInfo registrationInfo;

  // db_rows may possibly be overriden in the config, but should either way
  // always be constant
  const size_t db_rows;
  // store secret key and galois keys for pir
  string pir_secret_key;
  // TODO(sualeh, urgent): make this private
  // friendTable DOES NOT CONTAIN dummyMe
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