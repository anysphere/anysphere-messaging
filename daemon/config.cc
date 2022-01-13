
#include "config.hpp"

auto Friend::to_json() -> asphr::json {
  return asphr::json{{"name", name},
                     {"write_index", write_index},
                     {"read_index", read_index},
                     {"write_key", write_key},
                     {"read_key", read_key},
                     {"enabled", enabled}};
}

auto Friend::from_json(const asphr::json& j) -> Friend {
  Friend f;
  f.name = j.at("name").get<string>();
  f.write_index = j.at("write_index").get<int>();
  f.read_index = j.at("read_index").get<int>();
  f.write_key = j.at("write_key").get<string>();
  f.read_key = j.at("read_key").get<string>();
  f.enabled = j.at("enabled").get<bool>();
  return f;
}

// store the config in config_file_address
auto RegistrationInfo::to_json() -> asphr::json {
  asphr::json reg_json = {{"name", name},
                          {"public_key", Base64::Encode(public_key)},
                          {"private_key", Base64::Encode(private_key)},
                          {"authentication_token", authentication_token},
                          {"allocation", allocation}};
  return reg_json;
}

auto RegistrationInfo::from_json(const asphr::json& j) -> RegistrationInfo {
  RegistrationInfo reg_info;
  reg_info.name = j.at("name").get<string>();
  Base64::Decode(j.at("public_key").get<string>(), reg_info.public_key);
  Base64::Decode(j.at("private_key").get<string>(), reg_info.private_key);
  reg_info.authentication_token = j.at("authentication_token").get<string>();
  reg_info.allocation = j.at("allocation").get<vector<int>>();
  return reg_info;
}

auto read_json_file(const string& config_file_address) -> asphr::json {
  if (!std::filesystem::exists(config_file_address) ||
      std::filesystem::file_size(config_file_address) == 0) {
    auto dir_path =
        std::filesystem::path(config_file_address).parent_path().u8string();
    std::filesystem::create_directory(dir_path);
    cout << "creating new config asphr::json!" << endl;
    asphr::json j = R"({
      "has_registered": false,
      "friends": {}
    }
    )"_json;
    std::ofstream o(config_file_address);
    o << std::setw(4) << j.dump(4) << std::endl;
  }
  auto config_json = asphr::json::parse(std::ifstream(config_file_address));
  return config_json;
}

Config::Config(const string& config_file_address)
    : Config(read_json_file(config_file_address)) {}

Config::Config(const asphr::json& config_json) : db_rows(CLIENT_DB_ROWS) {
  if (!config_json.contains("has_registered")) {
    cout << "config file does not contain has_registered" << endl;
    return;
  }
  if (!config_json.contains("friends")) {
    cout << "config file does not contain friends" << endl;
    return;
  }
  if (!config_json["has_registered"].get<bool>()) {
    has_registered = false;
  } else {
    has_registered = true;
    registrationInfo =
        RegistrationInfo::from_json(config_json["registration_info"]);
    Base64::Decode(config_json["pir_secret_key"].get<string>(), pir_secret_key);
    Base64::Decode(config_json["pir_galois_keys"].get<string>(),
                   pir_galois_keys);
  }

  for (auto& friend_json : config_json["friends"]) {
    Friend f = Friend::from_json(friend_json);
    friendTable[f.name] = f;
  }
}

auto Config::save(const string& config_file_address) -> void {
  saved_file_address = config_file_address;
  save();
}

auto Config::save() -> void {
  asphr::json config_json;
  config_json["has_registered"] = has_registered;
  if (has_registered) {
    config_json["registration_info"] = registrationInfo.to_json();
    config_json["pir_secret_key"] = Base64::Encode(pir_secret_key);
    config_json["pir_galois_keys"] = Base64::Encode(pir_galois_keys);
  }
  for (auto& friend_pair : friendTable) {
    config_json["friends"].push_back(friend_pair.second.to_json());
  }
  std::ofstream o(saved_file_address);
  o << std::setw(4) << config_json.dump(4) << std::endl;
}

auto Config::has_space_for_friends() -> bool {
  return registrationInfo.allocation.size() > friendTable.size();
}

auto Config::add_friend(const Friend& f) -> void {
  friendTable[f.name] = f;
  Config::save();
}

auto Config::remove_friend(const string& name) -> absl::Status {
  if (!friendTable.contains(name)) {
    return absl::Status(absl::StatusCode::kInvalidArgument,
                        "friend does not exist");
  }
  friendTable.erase(name);
  Config::save();
  return absl::OkStatus();
}