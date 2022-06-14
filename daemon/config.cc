//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "config.hpp"

#include <utility>

#include "client_lib/client_lib.hpp"
#include "crypto.hpp"

// store the config in config_file_address
auto RegistrationInfo::to_json() -> asphr::json {
  asphr::json reg_json = {{"name", name},
                          {"public_key", asphr::Base64Escape(public_key)},
                          {"private_key", asphr::Base64Escape(private_key)},
                          {"authentication_token", authentication_token},
                          {"allocation", allocation}};
  return reg_json;
}

auto RegistrationInfo::from_json(const asphr::json& j) -> RegistrationInfo {
  RegistrationInfo reg_info;
  reg_info.name = j.at("name").get<string>();
  asphr::Base64Unescape(j.at("public_key").get<string>(), &reg_info.public_key);
  asphr::Base64Unescape(j.at("private_key").get<string>(),
                        &reg_info.private_key);
  reg_info.authentication_token = j.at("authentication_token").get<string>();
  reg_info.allocation = j.at("allocation").get<vector<int>>();
  return reg_info;
}

auto new_config_json() -> asphr::json {
  vector<asphr::json> friends;
  asphr::json j = {{"has_registered", false},
                   {"friends", friends},
                   {"data_dir", get_default_data_dir()},
                   {"server_address", DEFAULT_SERVER_ADDRESS}};
  return j;
}

auto read_json_file(const string& config_file_address) -> asphr::json {
  if (!std::filesystem::exists(config_file_address) ||
      std::filesystem::file_size(config_file_address) == 0) {
    auto dir_path =
        std::filesystem::path(config_file_address).parent_path().u8string();
    std::filesystem::create_directories(dir_path);
    ASPHR_LOG_INFO("Creating new config asphr::json.");
    asphr::json j = new_config_json();
    std::ofstream o(config_file_address);
    o << std::setw(4) << j.dump(4) << std::endl;
  }
  auto config_json = asphr::json::parse(std::ifstream(config_file_address));
  return config_json;
}

auto Config::server_address() -> std::string {
  check_rep();
  return server_address_;
}

Config::Config(const string& config_file_address)
    : Config(read_json_file(config_file_address), config_file_address) {}

Config::Config(const asphr::json& config_json_input, string config_file_address)
    : saved_file_address(std::move(config_file_address)),
      db_rows_(CLIENT_DB_ROWS),
      dummyMe("dummyMe", 0, "add_key", "", "", 0, false, 0, 0, 0, true),
      latency_(DEFAULT_ROUND_DELAY_SECONDS) {
  auto config_json = config_json_input;
  if (!config_json.contains("has_registered")) {
    ASPHR_LOG_WARN(
        "Invalid config file: Config file does not contain has_registered.");
    ASPHR_LOG_INFO("Creating new config file.");
    config_json = new_config_json();
  }
  if (!config_json.contains("friends")) {
    ASPHR_LOG_WARN(
        "Invalid config file: Config file does not contain friends.");
    ASPHR_LOG_INFO("Creating new config file.");
    config_json = new_config_json();
  }
  if (!config_json.contains("data_dir")) {
    ASPHR_LOG_WARN(
        "Invalid config file: Config file does not contain data_dir.");
    ASPHR_LOG_INFO("Creating new config file.");
    config_json = new_config_json();
  }
  if (!config_json.contains("server_address")) {
    ASPHR_LOG_WARN(
        "Invalid config file: Config file does not contain server_address.");
    ASPHR_LOG_INFO("Creating new config file.");
    config_json = new_config_json();
  }
  if (config_json.contains("latency")) {
    latency_ = config_json.at("latency").get<int>();
  }
  if (!config_json.at("has_registered").get<bool>()) {
    has_registered_ = false;
  } else {
    has_registered_ = true;
    registrationInfo =
        RegistrationInfo::from_json(config_json.at("registration_info"));
    asphr::Base64Unescape(config_json.at("pir_secret_key").get<string>(),
                          &pir_secret_key);
    asphr::Base64Unescape(config_json.at("pir_galois_keys").get<string>(),
                          &pir_galois_keys);
    // create a pir_client !
    pir_client_ =
        std::make_unique<FastPIRClient>(pir_secret_key, pir_galois_keys);
    // initialize the dummyMe
    initialize_dummy_me();
  }

  for (auto& friend_json : config_json.at("friends")) {
    Friend f = Friend::from_json(friend_json);
    friendTable.try_emplace(f.name, f);
  }

  data_dir = config_json.at("data_dir").get<string>();
  server_address_ = config_json.at("server_address").get<string>();

  save();

  check_rep();
}

auto Config::has_space_for_friends() -> bool {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();
  return friendTable.size() < MAX_FRIENDS;
}

auto Config::has_registered() -> bool {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();
  return has_registered_;
}

auto Config::do_register(const string& name, const string& public_key,
                         const string& private_key,
                         const string& authentication_token,
                         const vector<int>& allocation) -> void {
  check_rep();

  const std::lock_guard<std::mutex> l(config_mtx);

  registrationInfo = RegistrationInfo{
      .name = name,
      .public_key = public_key,
      .private_key = private_key,
      .authentication_token = authentication_token,
      .allocation = allocation,
  };

  auto [secret_key, galois_keys] = generate_keys();
  pir_secret_key = secret_key;
  pir_galois_keys = galois_keys;

  assert(pir_client_ == nullptr);

  pir_client_ =
      std::make_unique<FastPIRClient>(pir_secret_key, pir_galois_keys);
  initialize_dummy_me();

  has_registered_ = true;

  save();

  check_rep();
}

auto Config::registration_info() -> RegistrationInfo {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();
  return registrationInfo;
}

auto Config::num_enabled_friends() -> int {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();

  auto num_enabled_friends = 0;
  for (auto& friend_pair : friendTable) {
    if (friend_pair.second.enabled) {
      num_enabled_friends++;
    }
  }

  check_rep();

  return num_enabled_friends;
}

auto Config::random_enabled_friend(const std::unordered_set<string>& excluded)
    -> asphr::StatusOr<Friend> {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();

  vector<Friend> enabled_friends;
  for (auto& friend_pair : friendTable) {
    if (friend_pair.second.enabled && !excluded.contains(friend_pair.first)) {
      enabled_friends.push_back(friend_pair.second);
    }
  }

  if (enabled_friends.empty()) {
    return absl::NotFoundError("No enabled friends");
  }

  auto random_index = absl::Uniform(rand_bitgen_, 0u, enabled_friends.size());

  check_rep();

  return enabled_friends.at(random_index);
}

auto Config::dummy_me() -> Friend {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();

  return dummyMe;
}

auto Config::add_friend(const Friend& f) -> void {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();

  assert(!friendTable.contains(f.name));
  friendTable.try_emplace(f.name, f);

  save();
  check_rep();
}

auto Config::remove_friend(const string& name) -> absl::Status {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();

  if (!friendTable.contains(name)) {
    return {absl::StatusCode::kInvalidArgument, "friend does not exist"};
  }
  friendTable.erase(name);

  save();
  check_rep();

  return absl::OkStatus();
}

auto Config::friends() const -> vector<Friend> {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();

  vector<Friend> friends;
  for (const auto& [_, f] : friendTable) {
    friends.push_back(f);
  }

  check_rep();
  return friends;
}

auto Config::get_friend(const string& name) const -> absl::StatusOr<Friend> {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();

  if (!friendTable.contains(name)) {
    return absl::InvalidArgumentError("friend does not exist");
  }

  check_rep();
  return friendTable.at(name);
}

auto Config::update_friend(const string& name, const FriendUpdate& fs) -> void {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();

  assert(friendTable.contains(name));

  auto old_friend = friendTable.at(name);

  auto new_friend =
      Friend(name, fs.read_index.value_or(old_friend.read_index),
             fs.add_key.value_or(old_friend.add_key),
             fs.read_key.value_or(old_friend.read_key),
             fs.write_key.value_or(old_friend.write_key),
             fs.ack_index.value_or(old_friend.ack_index),
             fs.enabled.value_or(old_friend.enabled),
             fs.latest_ack_id.value_or(old_friend.latest_ack_id),
             fs.latest_send_id.value_or(old_friend.latest_send_id),
             fs.last_receive_id.value_or(old_friend.last_receive_id),
             fs.dummy.value_or(old_friend.dummy));

  friendTable.insert_or_assign(name, new_friend);

  save();

  check_rep();
}

auto Config::msgstore_address() -> std::filesystem::path {
  return data_dir / "msgstore.json";
}
auto Config::data_dir_address() -> std::filesystem::path { return data_dir; }

auto Config::pir_client() -> FastPIRClient& {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();
  return *pir_client_;
}

auto Config::db_rows() -> size_t {
  const std::lock_guard<std::mutex> l(config_mtx);

  return db_rows_;
}

auto Config::get_latency_seconds() -> int {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();
  return latency_;
}

auto Config::set_latency(int latency) -> asphr::Status {
  const std::lock_guard<std::mutex> l(config_mtx);

  check_rep();
  latency_ = latency;
  save();
  check_rep();

  return absl::OkStatus();
}

auto Config::kill() -> void {
  const std::lock_guard<std::mutex> l(config_mtx);

  {
    const std::lock_guard<std::mutex> kill_l(kill_mtx);
    kill_ = true;
  }

  kill_cv.notify_all();
}

auto Config::wait_until_killed_or_seconds(int seconds) -> bool {
  std::unique_lock<std::mutex> kill_l(kill_mtx);

  if (kill_) {
    return true;
  }

  kill_cv.wait_for(kill_l, std::chrono::seconds(seconds),
                   [this] { return kill_; });

  kill_l.unlock();

  // kill only ever changes false -> true, so we are fine returning this here
  // even after unlocking
  return kill_;
}

// private method; hence no check_rep, no lock
auto Config::check_rep() const -> void {
  assert(!saved_file_address.empty());
  ASPHR_ASSERT_NEQ(data_dir, "");
  assert(db_rows_ > 0);

  assert(latency_ >= 1);

  assert(friendTable.size() <= MAX_FRIENDS);

  if (has_registered_) {
    assert(!registrationInfo.name.empty());
    assert(registrationInfo.public_key.size() == crypto_kx_PUBLICKEYBYTES);
    assert(registrationInfo.private_key.size() == crypto_kx_SECRETKEYBYTES);
    assert(!registrationInfo.authentication_token.empty());
    assert(!registrationInfo.allocation.empty());

    assert(dummyMe.name == "dummyMe");
    assert(dummyMe.dummy);
    assert(dummyMe.write_key.size() ==
           crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    assert(dummyMe.read_key.size() ==
           crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    assert(dummyMe.enabled == false);

    assert(!pir_secret_key.empty());
    assert(!pir_galois_keys.empty());
    assert(pir_client_ != nullptr);
  } else {
    assert(dummyMe.write_key.empty());
    assert(dummyMe.read_key.empty());
  }
}

// private method; hence, no check_rep, no lock
auto Config::save() -> void {
  // only save if rep is ok!
  check_rep();

  asphr::json config_json;
  config_json["has_registered"] = has_registered_;
  config_json["data_dir"] = data_dir;
  config_json["server_address"] = server_address_;
  config_json["latency"] = latency_;

  if (has_registered_) {
    config_json["registration_info"] = registrationInfo.to_json();
    config_json["pir_secret_key"] = asphr::Base64Escape(pir_secret_key);
    config_json["pir_galois_keys"] = asphr::Base64Escape(pir_galois_keys);
  }
  vector<asphr::json> friends;
  for (auto& friend_pair : friendTable) {
    friends.push_back(friend_pair.second.to_json());
  }
  config_json["friends"] = friends;
  std::ofstream o(saved_file_address);
  o << std::setw(4) << config_json.dump(4) << std::endl;
}

// private method; hence, no check_rep, no lock
auto Config::initialize_dummy_me() -> void {
  auto crypto = Crypto();

  auto dummy_friend_keypair = crypto.generate_keypair();
  auto dummy_read_write_keys = crypto.derive_read_write_keys(
      registrationInfo.public_key, registrationInfo.private_key,
      dummy_friend_keypair.first);

  dummyMe = Friend("dummyMe", 0, "add_key", dummy_read_write_keys.first,
                   dummy_read_write_keys.second, 0, false, 0, 0, 0, true);

  // don't save at the end because initialize_dummy_me is called before other
  // things are initialized, so if we saved here we would save an invalid
  // config. that's why we also don't have a check-rep here: check-rep is not
  // guaranteed to pass!
}
