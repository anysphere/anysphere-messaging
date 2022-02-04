
#include "config.hpp"

#include "client/client_lib/client_lib.hpp"
#include "crypto.hpp"

auto Friend::to_json() -> asphr::json {
  check_rep();
  return asphr::json{{"name", name},
                     {"read_index", read_index},
                     {"write_key", asphr::Base64Escape(write_key)},
                     {"read_key", asphr::Base64Escape(read_key)},
                     {"ack_index", ack_index},
                     {"enabled", enabled},
                     {"latest_ack_id", latest_ack_id},
                     {"last_receive_id", last_receive_id}};
}

auto Friend::from_json(const asphr::json& j) -> Friend {
  Friend f;
  f.name = j.at("name").get<string>();
  f.read_index = j.at("read_index").get<int>();
  asphr::Base64Unescape(j.at("write_key").get<string>(), &f.write_key);
  asphr::Base64Unescape(j.at("read_key").get<string>(), &f.read_key);
  f.ack_index = j.at("ack_index").get<int>();
  f.enabled = j.at("enabled").get<bool>();
  f.latest_ack_id = j.at("latest_ack_id").get<uint32_t>();
  f.last_receive_id = j.at("last_receive_id").get<uint32_t>();
  f.check_rep();
  return f;
}

auto Friend::check_rep() const -> void {
  assert(!name.empty());
  assert(ack_index >= 0);
  assert(static_cast<size_t>(ack_index) < MAX_FRIENDS);
  if (enabled) {
    assert(read_index >= 0);
    assert(read_key.size() == crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    assert(write_key.size() == crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
  } else {
    // read_index must be valid! and index 0 will ALWAYS be a valid index.
    assert(read_index == 0);
  }
}

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
    cout << "creating new config asphr::json!" << endl;
    asphr::json j = new_config_json();
    std::ofstream o(config_file_address);
    o << std::setw(4) << j.dump(4) << std::endl;
  }
  auto config_json = asphr::json::parse(std::ifstream(config_file_address));
  return config_json;
}

auto Config::initialize_dummy_me() -> void {
  auto crypto = Crypto();

  auto dummy_friend_keypair = crypto.generate_keypair();
  auto dummy_read_write_keys = crypto.derive_read_write_keys(
      registrationInfo.public_key, registrationInfo.private_key,
      dummy_friend_keypair.first);

  dummyMe = Friend("dummyMe", 0, dummy_read_write_keys.first,
                   dummy_read_write_keys.second, 0, false, 0, 0);
}

Config::Config(const string& config_file_address)
    : Config(read_json_file(config_file_address), config_file_address) {}

Config::Config(const asphr::json& config_json_input,
               const string& config_file_address)
    : db_rows(CLIENT_DB_ROWS), saved_file_address(config_file_address) {
  auto config_json = config_json_input;
  if (!config_json.contains("has_registered")) {
    cout << "WARNING (invalid config file): config file does not contain "
            "has_registered"
         << endl;
    cout << "creating a new config file" << endl;
    config_json = new_config_json();
  }
  if (!config_json.contains("friends")) {
    cout
        << "WARNING (invalid config file): config file does not contain friends"
        << endl;
    cout << "creating a new config file" << endl;
    config_json = new_config_json();
  }
  if (!config_json.contains("data_dir")) {
    cout << "WARNING (invalid config file): config file does not contain "
            "data_dir"
         << endl;
    cout << "creating a new config file" << endl;
    config_json = new_config_json();
  }
  if (!config_json.contains("server_address")) {
    cout << "WARNING (invalid config file): config file does not contain "
            "server_address"
         << endl;
    cout << "creating a new config file" << endl;
    config_json = new_config_json();
  }
  if (!config_json.at("has_registered").get<bool>()) {
    has_registered = false;
  } else {
    has_registered = true;
    registrationInfo =
        RegistrationInfo::from_json(config_json.at("registration_info"));
    asphr::Base64Unescape(config_json.at("pir_secret_key").get<string>(),
                          &pir_secret_key);
    asphr::Base64Unescape(config_json.at("pir_galois_keys").get<string>(),
                          &pir_galois_keys);
    // create a pir_client !
    pir_client =
        std::make_unique<FastPIRClient>(pir_secret_key, pir_galois_keys);
    // initialize the dummyMe
    initialize_dummy_me();
  }

  for (auto& friend_json : config_json.at("friends")) {
    Friend f = Friend::from_json(friend_json);
    friendTable[f.name] = f;
  }

  data_dir = config_json.at("data_dir").get<string>();
  server_address = config_json.at("server_address").get<string>();

  check_rep();
}

auto Config::save() -> void {
  asphr::json config_json;
  config_json["has_registered"] = has_registered;
  config_json["data_dir"] = data_dir;
  config_json["server_address"] = server_address;
  if (has_registered) {
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

  check_rep();
}

auto Config::has_space_for_friends() -> bool {
  check_rep();
  return friendTable.size() < MAX_FRIENDS;
}

auto Config::num_enabled_friends() -> int {
  auto num_enabled_friends = 0;
  for (auto& friend_pair : friendTable) {
    if (friend_pair.second.enabled) {
      num_enabled_friends++;
    }
  }
  return num_enabled_friends;
}

auto Config::random_enabled_friend() -> string {
  assert(num_enabled_friends() > 0);
  vector<string> enabled_friends;
  for (auto& friend_pair : friendTable) {
    if (friend_pair.second.enabled) {
      enabled_friends.push_back(friend_pair.first);
    }
  }
  auto random_index = rand() % enabled_friends.size();
  return enabled_friends.at(random_index);
}

auto Config::add_friend(const Friend& f) -> void {
  check_rep();
  friendTable[f.name] = f;
  Config::save();
  check_rep();
}

auto Config::remove_friend(const string& name) -> absl::Status {
  check_rep();
  if (!friendTable.contains(name)) {
    return absl::Status(absl::StatusCode::kInvalidArgument,
                        "friend does not exist");
  }
  friendTable.erase(name);
  Config::save();
  check_rep();
  return absl::OkStatus();
}

auto Config::receive_file_address() -> std::filesystem::path {
  return data_dir / "receive.ndjson";
}
auto Config::send_file_address() -> std::filesystem::path {
  return data_dir / "send.ndjson";
}

auto Config::check_rep() const -> void {
  assert(saved_file_address != "");
  assert(data_dir != "");
  assert(db_rows > 0);

  assert(friendTable.size() <= MAX_FRIENDS);

  if (has_registered) {
    assert(registrationInfo.name != "");
    assert(registrationInfo.public_key.size() == crypto_kx_PUBLICKEYBYTES);
    assert(registrationInfo.private_key.size() == crypto_kx_SECRETKEYBYTES);
    assert(registrationInfo.authentication_token.size() > 0);
    assert(registrationInfo.allocation.size() > 0);

    assert(dummyMe.name == "dummyMe");
    assert(dummyMe.write_key.size() ==
           crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    assert(dummyMe.read_key.size() ==
           crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    assert(dummyMe.enabled == false);

    assert(pir_secret_key != "");
    assert(pir_galois_keys != "");
    assert(pir_client != nullptr);
  } else {
    assert(dummyMe.write_key.size() == 0);
    assert(dummyMe.read_key.size() == 0);
  }
}