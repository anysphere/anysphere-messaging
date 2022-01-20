#include "outbox.hpp"

#include "client/client_lib/client_lib.hpp"

auto read_outbox_json(const string& file_address) -> asphr::json {
  if (!std::filesystem::exists(file_address) ||
      std::filesystem::file_size(file_address) == 0) {
    auto dir_path =
        std::filesystem::path(file_address).parent_path().u8string();
    std::filesystem::create_directories(dir_path);
    cout << "creating new outbox asphr::json!" << endl;
    asphr::json j = {};
    std::ofstream o(file_address);
    o << std::setw(4) << j.dump(4) << std::endl;
  }
  auto config_json = asphr::json::parse(std::ifstream(file_address));
  return config_json;
}

Outbox::Outbox(const string& file_address)
    : Outbox(read_outbox_json(file_address), file_address) {}

Outbox::Outbox(const asphr::json& serialized_json, const string& file_address) {
  // TODO: parse this
}

auto Outbox::save() noexcept(false) -> void {
  // TODO: save this
}

auto Outbox::add(const string& message, Friend& friend_info) noexcept -> void {
  // chunk! and add.
  vector<string> chunked_message;
  for (size_t i = 0; i < message.size(); i += GUARANTEED_SINGLE_MESSAGE_SIZE) {
    chunked_message.push_back(
        message.substr(i, GUARANTEED_SINGLE_MESSAGE_SIZE));
  }
  uint32_t last_send_id = friend_info.latest_ack_id;
  if (outbox.contains(friend_info.name)) {
    last_send_id = outbox.at(friend_info.name).back().id;
  }
  uint32_t chunks_start_id;
  for (size_t i = 0; i < chunked_message.size(); i++) {
    auto id = last_send_id + 1;
    last_send_id = id;
    if (i == 0) {
      chunks_start_id = id;
    }

    asphrclient::Message m;
    m.set_id(id);
    m.set_msg(chunked_message[i]);
    if (chunked_message.size() > 1) {
      m.set_num_chunks(chunked_message.size());
      m.set_chunks_start_id(chunks_start_id);
    }
    auto msgToSend = MessageToSend{friend_info.name, m, id};

    if (outbox.contains(friend_info.name)) {
      outbox[friend_info.name].push_back(msgToSend);
    } else {
      outbox[friend_info.name] = {msgToSend};
    }
  }
}

auto Outbox::message_to_send(
    const std::unordered_map<string, Friend>& friendTable,
    const Friend& dummyMe) -> MessageToSend {
  // first remove ACKed messages
  vector<string> recently_acked_friends;
  for (auto& [friend_name, messages] : outbox) {
    auto remove_num = 0;
    for (size_t i = 0; i < messages.size(); i++) {
      if (messages[i].id <= friendTable.at(friend_name).latest_ack_id) {
        remove_num++;
        recently_acked_friends.push_back(friend_name);
      } else {
        break;
      }
    }
    messages.erase(messages.begin(), messages.begin() + remove_num);
    if (messages.size() == 0) {
      outbox.erase(friend_name);
    }
  }

  // now choose a message to send!!
  // first try with the recently acked friends
  for (auto& friend_name : recently_acked_friends) {
    if (outbox.contains(friend_name)) {
      auto& messages = outbox[friend_name];
      return messages.at(0);
    }
  }

  // if not returned yet, choose a random friend from the outbox
  auto random_friend = std::next(std::begin(outbox), rand() % outbox.size());
  auto& messages = (*random_friend).second;
  return messages.at(0);
}