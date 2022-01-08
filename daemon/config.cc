
#include "config.hpp"

auto Friend::to_json() -> json {
  return json{{"name", name},
              {"write_index", write_index},
              {"read_index", read_index},
              {"write_key", write_key},
              {"read_key", read_key}};
}

auto Friend::from_json(const json& j) -> Friend {
  Friend f;
  f.name = j.at("name").get<string>();
  f.write_index = j.at("write_index").get<int>();
  f.read_index = j.at("read_index").get<int>();
  f.write_key = j.at("write_key").get<string>();
  f.read_key = j.at("read_key").get<string>();
  return f;
}

// store the config in config_file_address
auto RegistrationInfo::to_json() -> json {
  json reg_json = {{"name", name},
                   {"public_key", public_key},
                   {"private_key", private_key},
                   {"authentication_token", authentication_token},
                   {"allocation", allocation}};
  return reg_json;
}

auto RegistrationInfo::from_json(const json& j) -> RegistrationInfo {
  RegistrationInfo reg_info;
  reg_info.name = j.at("name").get<string>();
  reg_info.public_key = j.at("public_key").get<string>();
  reg_info.private_key = j.at("private_key").get<string>();
  reg_info.authentication_token = j.at("authentication_token").get<string>();
  reg_info.allocation = j.at("allocation").get<vector<int>>();
  return reg_info;
}

auto Config::Config(const string& config_file_address) {
  if (!std::filesystem::exists(config_file_address) ||
      std::filesystem::file_size(config_file_address) == 0) {
    cout << "creating new config json!" << endl;
    json j = R"({
      "has_registered": false,
      "friends": {}
    }
    )"_json;
    std::ofstream o(config_file_address);
    o << std::setw(4) << j.dump(4) << std::endl;
  }
  auto config_json = json::parse(std::ifstream(config_file_address));
  Config(config_json);
}

auto Config::Config(const json& config_json) {
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
    db_rows = config_json["db_rows"].get<int>();
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
  json config_json;
  config_json["has_registered"] = has_registered;
  if (has_registered) {
    config_json["registration_info"] = registrationInfo.to_json();
    config_json["db_rows"] = db_rows;
    config_json["pir_secret_key"] =
        Base64::Encode(pir_secret_key.data(), pir_secret_key.size());
    config_json["pir_galois_keys"] =
        Base64::Encode(pir_galois_keys.data(), pir_galois_keys.size());
  }
  for (auto& friend_pair : friendTable) {
    config_json["friends"].push_back(friend_pair.second.to_json());
  }
  std::ofstream o(config_file_address);
  o << std::setw(4) << config_json.dump(4) << std::endl;
}

auto Config::has_space_for_friends(const vector<int>& allocation) -> bool {
  return allocation.size() > friendTable.size();
}