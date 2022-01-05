#include "anysphere.h"

using messenger::ReceiveMessageInfo;
using messenger::ReceiveMessageResponse;
using messenger::SendMessageInfo;
using messenger::SendMessageResponse;

void retrieve_messages(const string& output_file_address,
                       std::unique_ptr<Messenger::Stub>& stub) {
  if (!RegistrationInfo.has_registered) {
    cout << "hasn't registered yet, so cannot retrieve messages" << endl;
    return;
  }
  if (RegistrationInfo.pir_client == nullptr) {
    RegistrationInfo.pir_client = std::make_unique<FastPIRClient>(
        RegistrationInfo.pir_secret_key, RegistrationInfo.pir_galois_keys);
  }

  auto& client = *RegistrationInfo.pir_client;
  for (auto& [friend_name, friend_info] : FriendTable) {
    ReceiveMessageInfo request;

    auto query = client.query(friend_info.read_index, RegistrationInfo.db_rows);

    auto serialized_query = query.serialize_to_string();

    request.set_pir_query(serialized_query);

    ReceiveMessageResponse reply;
    ClientContext context;

    Status status = stub->ReceiveMessage(&context, request, &reply);

    if (status.ok()) {
      cout << "received message!!!" << endl;
      auto answer = reply.pir_answer();
      auto answer_obj = client.answer_from_string(answer);
      auto decoded_value = client.decode(answer_obj, friend_info.read_index);

      if (decoded_value[0] == 0) {
        cout << "empty message for security" << endl;
        continue;
      }

      string decoded_string = "";
      for (auto& c : decoded_value) {
        if (c == 0) break;
        decoded_string += c;
      }

      cout << "actual message: " << decoded_string << endl;
      {
        auto file = std::ofstream(output_file_address, std::ios_base::app);

        auto time = absl::FormatTime(absl::Now(), utc);
        json jmsg = {{"from", friend_name},
                     {"timestamp", time},
                     {"message", decoded_string},
                     {"type", "MESSAGE_RECEIVED"}};
        if (file.is_open()) {
          file << std::setw(4) << jmsg.dump() << std::endl;
          cout << jmsg.dump() << endl;
          file.close();
        }
      }
    } else {
      cout << status.error_code() << ": " << status.error_message() << endl;
    }
  }
}

void process_ui_file(const string& ui_file_address,
                     const Time& last_ui_timestamp,
                     std::unique_ptr<Messenger::Stub>& stub) {
  if (!RegistrationInfo.has_registered) {
    cout << "hasn't registered yet, so don't send a message" << endl;
    return;
  }

  auto new_entries = get_new_entries(ui_file_address, last_ui_timestamp);

  auto authentication_token = RegistrationInfo.authentication_token;

  std::unordered_map<string, string> friend_to_message;

  for (auto& entry : new_entries) {
    auto to = entry["to"].get<string>();
    if (!FriendTable.contains(to)) {
      std::cerr << "FriendHashTable does not contain " << to
                << "; ignoring message" << endl;
      continue;
    }
    friend_to_message.insert({to, entry["message"].get<string>()});
  }

  for (auto& [friend_name, friend_info] : FriendTable) {
    pir_value_t padded_msg;
    padded_msg.fill(0);
    if (friend_to_message.count(friend_name) != 0) {
      auto message = friend_to_message.at(friend_name);
      std::copy(message.begin(), message.end(), padded_msg.begin());
    }

    auto index = friend_info.write_index;
    auto key = friend_info.shared_key;

    cout << "Sending message to server: " << endl;
    cout << "index: " << index << endl;
    cout << "authentication_token: " << authentication_token << endl;

    // static auto last_type = type;
    // static auto last_timestamp = timestamp;
    // static auto last_message = entry["message"];

    // call register rpc to send the register request
    SendMessageInfo request;

    request.set_index(index);
    request.set_authentication_token(authentication_token);
    // TODO: encrypt message here, pad it to the right length
    string padded_msg_str = "";
    for (auto& c : padded_msg) {
      padded_msg_str += c;
    }
    request.set_message(padded_msg_str);

    SendMessageResponse reply;

    ClientContext context;

    Status status = stub->SendMessage(&context, request, &reply);

    if (status.ok()) {
      auto db_rows = reply.db_rows();
      cout << "db_rows: " << db_rows << endl;
      RegistrationInfo.db_rows = db_rows;
      std::cout << "Message sent to server: " << request.message() << std::endl;
    } else {
      std::cerr << status.error_code() << ": " << status.error_message()
                << " details:" << status.error_details() << std::endl;
    }
  }
}
