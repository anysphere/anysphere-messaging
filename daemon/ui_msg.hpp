#pragma once

#include "crypto.hpp"
#include "schema/message.pb.h"

using client::Message;

void retrieve_messages(const string& output_file_address,
                       unique_ptr<asphrserver::Server::Stub>& stub,
                       const Crypto& crypto, const Config& config) {
  if (!config.has_registered) {
    cout << "hasn't registered yet, so cannot retrieve messages" << endl;
    return;
  }

  auto& client = *config.pir_client;
  for (auto& [friend_name, friend_info] : config.friendTable) {
    asphrserver::ReceiveMessageInfo request;

    auto query = client.query(friend_info.read_index, config.db_rows);

    auto serialized_query = query.serialize_to_string();

    request.set_pir_query(serialized_query);

    asphrserver::ReceiveMessageResponse reply;
    grpc::ClientContext context;

    grpc::Status status = stub->ReceiveMessage(&context, request, &reply);

    if (status.ok()) {
      cout << "received message!!!" << endl;
      auto answer = reply.pir_answer();
      auto answer_obj = client.answer_from_string(answer);
      auto decoded_value = client.decode(answer_obj, friend_info.read_index);

      auto decrypted = crypto.decrypt_receive(decoded_value, friend_info);
      if (!decrypted.ok()) {
        cout << "decryption failed: " << decrypted.status() << endl;
        continue;
      }
      auto& message = decrypted.value();

      if (message.id() == 0) {
        cout << "empty message for security" << endl;
        continue;
      }

      cout << "actual message: " << message.msg() << endl;
      {
        auto file = std::ofstream(output_file_address, std::ios_base::app);

        auto time = absl::FormatTime(absl::Now(), utc);
        json jmsg = {{"from", friend_name},
                     {"timestamp", time},
                     {"message", message.msg()},
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
                     unique_ptr<asphrserver::Server::Stub>& stub,
                     Crypto& crypto, Config& config) {
  if (!config.has_registered) {
    cout << "hasn't registered yet, so don't send a message" << endl;
    return;
  }

  auto new_entries = get_new_entries(ui_file_address, last_ui_timestamp);

  auto authentication_token = config.registrationInfo.authentication_token;

  std::unordered_map<string, string> friend_to_message;

  for (auto& entry : new_entries) {
    auto to = entry["to"].get<string>();
    if (!config.friendTable.contains(to)) {
      std::cerr << "FriendHashTable does not contain " << to
                << "; ignoring message" << endl;
      continue;
    }
    friend_to_message.insert({to, entry["message"].get<string>()});
  }

  for (auto& [friend_name, friend_info] : config.friendTable) {
    Message message;
    message.set_id(0);
    if (friend_to_message.count(friend_name) != 0) {
      auto message_s = friend_to_message.at(friend_name);
      message.set_id(1);  // TODO: actually store a counter for every friend
                          // with the ID of last sent thing
      message.set_msg(message_s);
    }

    auto index = friend_info.write_index;

    auto pir_value_message_status = crypto.encrypt_send(message, friend_info);
    if (!pir_value_message_status.ok()) {
      cout << "encryption failed; not doing anything with message"
           << pir_value_message_status.status() << endl;
      continue;
    }
    auto pir_value_message = pir_value_message_status.value();

    cout << "Sending message to server: " << endl;
    cout << "index: " << index << endl;
    cout << "authentication_token: " << authentication_token << endl;

    // static auto last_type = type;
    // static auto last_timestamp = timestamp;
    // static auto last_message = entry["message"];

    // call register rpc to send the register request
    asphrserver::SendMessageInfo request;

    request.set_index(index);
    request.set_authentication_token(authentication_token);
    // TODO: encrypt message here, pad it to the right length
    string padded_msg_str = "";
    for (auto& c : pir_value_message) {
      padded_msg_str += c;
    }
    request.set_message(padded_msg_str);

    asphrserver::SendMessageResponse reply;

    grpc::ClientContext context;

    grpc::Status status = stub->SendMessage(&context, request, &reply);

    if (status.ok()) {
      auto db_rows = reply.db_rows();
      cout << "db_rows: " << db_rows << endl;
      config.db_rows = db_rows;
      std::cout << "Message sent to server: " << request.message() << std::endl;
    } else {
      std::cerr << status.error_code() << ": " << status.error_message()
                << " details:" << status.error_details() << std::endl;
    }
  }
}
