#include "anysphere.h"

void process_ui_urgent_file(const string &ui_urgent_file_address,
                            const string &config_file_address,
                            const Time &last_ui_urgent_timestamp,
                            std::unique_ptr<Messenger::Stub> &stub) {
  auto new_entries =
      get_new_entries(ui_urgent_file_address, last_ui_urgent_timestamp);

  if (new_entries.empty()) {
    cout << "no new entries"
         << "last_ui_urgent_timestamp: " << last_ui_urgent_timestamp << endl;
    return;
  }
  for (auto &entry : new_entries) {
    auto type = entry["type"].get<string>();
    auto timestamp = entry["timestamp"].get<string>();
    if (type == "REGISTER") {
      if (RegistrationInfo.has_registered) {
        cout << "already registered" << endl;
        continue;
      }
      // call register rpc to send the register request
      RegisterInfo request;
      auto public_key = entry["public_key"].get<string>();
      auto private_key = entry["private_key"].get<string>();
      request.set_public_key(public_key);

      RegistrationInfo.name = entry["name"].get<string>();
      RegistrationInfo.public_key = public_key;
      RegistrationInfo.private_key = private_key;

      RegisterResponse reply;
      ClientContext context;

      Status status = stub->Register(&context, request, &reply);

      if (status.ok()) {
        // TODO(sualeh): tell the UI that the registration was successful
        cout << "register success" << endl;

        RegistrationInfo.has_registered = true;
        RegistrationInfo.authentication_token = reply.authentication_token();
        auto alloc_repeated = reply.allocation();
        RegistrationInfo.allocation =
            vector<int>(alloc_repeated.begin(), alloc_repeated.end());

        if (reply.authentication_token() == "") {
          cout << "authentication token is empty" << endl;
          continue;
        }
        if (reply.allocation().empty()) {
          cout << "allocation is empty" << endl;
          continue;
        }

        auto [secret_key, galois_keys] = generate_keys();
        RegistrationInfo.pir_secret_key = secret_key;
        RegistrationInfo.pir_galois_keys = galois_keys;

        RegistrationInfo.store(config_file_address);
      } else {
        cout << status.error_code() << ": " << status.error_message() << endl;
      }
    } else if (type == "FRIEND") {
      auto name = entry["name"].get<string>();
      if (!FriendTable.contains(name)) {
        auto write_index = entry["write_index"].get<int>();
        auto read_index = entry["read_index"].get<int>();
        auto shared_key = entry["shared_key"].get<string>();

        Friend friend_(name, write_index, read_index, shared_key);
        FriendTable.insert({name, friend_});
        cout << "added friend " << name << endl;
        store_friend_table(config_file_address);
      }
    } else {
      std::cerr
          << "Unknown type of message. Ensure that messages are correctly "
             "written to file. "
          << type << std::endl;
      exit(1);
    }
  }
}
