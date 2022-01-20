#include "inbox.hpp"

#include "constants.hpp"

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
    -> std::optional<asphrclient::Message> {
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

  return message;
}