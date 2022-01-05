#pragma once
#include <grpcpp/grpcpp.h>

#include "asphr/asphr.h"
#include "client/client_lib/client_lib.h"
#include "schema/messenger.grpc.pb.h"
#include "server/pir/fast_pir/fast_pir_client.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using messenger::Messenger;
using messenger::RegisterInfo;
using messenger::RegisterResponse;

struct Friend {
  Friend() = default;
  Friend(string name, int write_index, int read_index, string shared_key)
      : name(name),
        write_index(write_index),
        read_index(read_index),
        shared_key(shared_key){};

  string name;
  int write_index;
  int read_index;
  string shared_key;
};

struct RegisterationInfo {
  RegisterationInfo() = default;
  RegisterationInfo(string name, string public_key, string private_key)
      : name(name), public_key(public_key), private_key(private_key){};

  bool has_registered;
  string name;
  string public_key;
  string private_key;
  string authentication_token;
  vector<int> allocation;
  // the client needs to know roughly how many rows are in the database.
  // here we cache the latest number of db_rows we received
  // TODO: move this out of registrationinfo?
  int db_rows;
  // store secret key and galois keys for pir
  string pir_secret_key;
  string pir_galois_keys;
  // make this a ptr because we want it to possibly be null
  std::unique_ptr<FastPIRClient> pir_client = nullptr;

  // store the config in config_file_address
  auto store(const string& config_file_address) -> void {
    json reg_json = {{"name", name},
                     {"public_key", public_key},
                     {"private_key", private_key},
                     {"authentication_token", authentication_token},
                     {"allocation", allocation},
                     {"db_rows", db_rows},
                     {"pir_secret_key", Base64::Encode(pir_secret_key)},
                     {"pir_galois_keys", Base64::Encode(pir_galois_keys)}};
    json config_json = json::parse(std::ifstream(config_file_address));
    config_json["registration_info"] = reg_json;
    config_json["has_registered"] = true;
    std::ofstream out(config_file_address);
    out << std::setw(4) << config_json.dump(4) << std::endl;
  }
};

static auto RegistrationInfo = RegisterationInfo();
static auto FriendTable = std::unordered_map<string, Friend>();

auto store_friend_table(const string& config_file_address) -> void {
  json config_json = json::parse(std::ifstream(config_file_address));
  json friend_table_json = json::array();
  for (auto& friend_pair : FriendTable) {
    json friend_json = {{"name", friend_pair.second.name},
                        {"write_index", friend_pair.second.write_index},
                        {"read_index", friend_pair.second.read_index},
                        {"shared_key", friend_pair.second.shared_key}};
    friend_table_json.push_back(friend_json);
  }
  config_json["friends"] = friend_table_json;
  std::ofstream out(config_file_address);
  out << std::setw(4) << config_json.dump(4) << std::endl;
}

auto read_config(const string& config_file_address) -> void {
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
  if (!config_json.contains("has_registered")) {
    cout << "config file does not contain has_registered" << endl;
    return;
  }
  if (!config_json.contains("friends")) {
    cout << "config file does not contain friends" << endl;
    return;
  }
  if (!config_json["has_registered"].get<bool>()) {
    RegistrationInfo.has_registered = false;
  } else {
    RegistrationInfo.has_registered = true;
    RegistrationInfo.name =
        config_json["registration_info"]["name"].get<string>();
    RegistrationInfo.public_key =
        config_json["registration_info"]["public_key"].get<string>();
    RegistrationInfo.private_key =
        config_json["registration_info"]["private_key"].get<string>();
    RegistrationInfo.authentication_token =
        config_json["registration_info"]["authentication_token"].get<string>();
    RegistrationInfo.allocation =
        config_json["registration_info"]["allocation"].get<vector<int>>();
    RegistrationInfo.db_rows =
        config_json["registration_info"]["db_rows"].get<int>();
    Base64::Decode(
        config_json["registration_info"]["pir_secret_key"].get<string>(),
        RegistrationInfo.pir_secret_key);
    Base64::Decode(
        config_json["registration_info"]["pir_galois_keys"].get<string>(),
        RegistrationInfo.pir_galois_keys);
  }

  for (auto& friend_json : config_json["friends"]) {
    FriendTable[friend_json["name"].get<string>()] =
        Friend(friend_json["name"].get<string>(),
               friend_json["write_index"].get<int>(),
               friend_json["read_index"].get<int>(),
               friend_json["shared_key"].get<string>());
  }
}
