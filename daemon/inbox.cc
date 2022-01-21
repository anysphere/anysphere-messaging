#include "inbox.hpp"

#include "constants.hpp"

auto read_inbox_json(const string& file_address) -> asphr::json {
  if (!std::filesystem::exists(file_address) ||
      std::filesystem::file_size(file_address) == 0) {
    auto dir_path =
        std::filesystem::path(file_address).parent_path().u8string();
    std::filesystem::create_directories(dir_path);
    cout << "creating new inbox asphr::json!" << endl;
    asphr::json j = {{"inprogress", {}}};
    std::ofstream o(file_address);
    cout << "here 2: " << endl;
    o << std::setw(4) << j.dump(4) << std::endl;
  }
  auto i_json = asphr::json::parse(std::ifstream(file_address));
  return i_json;
}

Inbox::Inbox(const string& file_address)
    : Inbox(read_inbox_json(file_address), file_address) {}

Inbox::Inbox(const asphr::json& serialized_json, const string& file_address)
    : saved_file_address(file_address) {
  for (const auto& inbox_item : serialized_json["inprogress"]) {
    auto friend_name = inbox_item.at("friend_name").get<string>();
    auto chunk_start_id = inbox_item.at("chunk_start_id").get<uint32_t>();
    auto chunks = inbox_item.at("chunks").get<vector<string>>();
    inbox[make_pair(friend_name, chunk_start_id)] = chunks;
  }
}

auto Inbox::save() noexcept(false) -> void {
  asphr::json j = {"inprogress", {}};
  for (const auto& inbox_item : inbox) {
    auto friend_name = inbox_item.first.first;
    auto chunk_start_id = inbox_item.first.second;
    auto chunks = inbox_item.second;
    asphr::json j_item = {
        {"friend_name", friend_name},
        {"chunk_start_id", chunk_start_id},
        {"chunks", chunks},
    };
    j["inprogress"].push_back(j_item);
  }
  std::ofstream o(saved_file_address);
  // DEBUG PRINT
  cout << "here 1: " << endl;
  o << std::setw(4) << j.dump(4) << std::endl;
}

auto Inbox::get_encrypted_acks(
    const std::unordered_map<string, Friend>& friendTable, const Crypto& crypto,
    const Friend& dummyMe) -> asphr::StatusOr<pir_value_t> {
  assert(friendTable.size() <= MAX_FRIENDS);

  vector<string> encrypted_acks(MAX_FRIENDS);
  for (auto& [friend_name, friend_info] : friendTable) {
    if (!friend_info.enabled) {
      continue;
    }
    auto ack = crypto.encrypt_ack(friend_info.last_receive_id, friend_info);
    if (!ack.ok()) {
      cout << "encryption failed: " << ack.status() << endl;
      return absl::UnknownError("encryption failed");
    }
    // DEBUG
    cout << "encrypted ack: " << ack.value() << endl;
    cout << "friend_name: " << friend_name << endl;
    encrypted_acks[friend_info.ack_index] = ack.value();
  }
  for (size_t i = 0; i < MAX_FRIENDS; i++) {
    if (encrypted_acks[i].empty()) {
      auto ack = crypto.encrypt_ack(0, dummyMe);
      if (!ack.ok()) {
        cout << "encryption failed: " << ack.status() << endl;
        return absl::UnknownError("encryption failed");
      }
      encrypted_acks[i] = ack.value();
    }
  }
  pir_value_t pir_acks;
  for (size_t i = 0; i < MAX_FRIENDS; i++) {
    std::copy(encrypted_acks[i].begin(), encrypted_acks[i].end(),
              encrypted_acks.begin() + i * ENCRYPTED_ACKING_BYTES);
  }

  return pir_acks;
}

auto Inbox::update_ack_from_friend(pir_value_t& pir_acks, Friend& friend_info,
                                   const Crypto& crypto) -> bool {
  vector<string> encrypted_acks(MAX_FRIENDS);
  for (size_t i = 0; i < MAX_FRIENDS; i++) {
    encrypted_acks[i].resize(ENCRYPTED_ACKING_BYTES);
    std::copy(pir_acks.begin() + i * ENCRYPTED_ACKING_BYTES,
              pir_acks.begin() + (i + 1) * ENCRYPTED_ACKING_BYTES,
              encrypted_acks[i].begin());
  }

  // try decrypting each!
  for (size_t i = 0; i < MAX_FRIENDS; i++) {
    auto ack = crypto.decrypt_ack(encrypted_acks[i], friend_info);
    if (!ack.ok()) {
      cout << "decryption failed (this is expected!): " << ack.status() << endl;
      continue;
    }
    if (ack.value() > friend_info.last_receive_id) {
      friend_info.latest_ack_id = ack.value();
    } else {
      cout << "something weird is going on.... ACKing is older than latest ack "
              "id. look into this"
           << endl;
    }
    return true;
  }
  return false;
}

auto Inbox::receive_message(FastPIRClient& client,
                            const asphrserver::ReceiveMessageResponse& reply,
                            Friend& friend_info, const Crypto& crypto,
                            string& previous_success_receive_friend)
    -> std::optional<InboxMessage> {
  // first look at the ACKs
  auto ack_answer = reply.pir_answer_acks();
  auto ack_answer_obj = client.answer_from_string(ack_answer);
  auto decoded_acks = client.decode(ack_answer_obj, friend_info.read_index);
  // now check to see what we can do with this! update the friend ACK info
  auto success_acks = update_ack_from_friend(decoded_acks, friend_info, crypto);
  // we do not want to break if this wasn't successful. it is ok if it
  // isn't!!!
  cout << "acks successful or not (expect fail often): " << success_acks
       << endl;

  auto answer = reply.pir_answer();
  auto answer_obj = client.answer_from_string(answer);
  auto decoded_value = client.decode(answer_obj, friend_info.read_index);

  auto decrypted = crypto.decrypt_receive(decoded_value, friend_info);
  if (!decrypted.ok()) {
    cout << "decryption failed, message was probably not for us: "
         << decrypted.status() << endl;
    return std::nullopt;
  } else {
    previous_success_receive_friend = friend_info.name;
  }
  auto& message = decrypted.value();

  if (message.id() == 0) {
    cout << "empty message for security" << endl;
    return std::nullopt;
  }

  // only ACK this message if it is exactly the next ID we expect. otherwise, we
  // still need to await more messages
  if (message.id() == friend_info.last_receive_id + 1) {
    friend_info.last_receive_id = message.id();
  } else {
    cout << "message ID is not next expected ID. we need to wait for more "
            "messages. WARNING may be worth looking into."
         << endl;
  }

  if (message.num_chunks() == 0) {
    return InboxMessage{message.msg(), friend_info.name, message.id()};
  }

  // now we need to check chunks...
  auto chunk_key = make_pair(friend_info.name, message.chunks_start_id());
  if (!inbox.contains(chunk_key)) {
    inbox[chunk_key] = vector<string>(message.num_chunks());
  }
  auto& chunks = inbox.at(chunk_key);

  chunks[message.id() - message.chunks_start_id()] = message.msg();

  if (friend_info.last_receive_id ==
      message.chunks_start_id() + message.num_chunks() - 1) {
    // we have all the chunks!
    string msg;
    for (auto& chunk : chunks) {
      msg += chunk;
    }
    // remove the chunk
    inbox.erase(chunk_key);
    return InboxMessage{msg, friend_info.name, message.id()};
  }
}