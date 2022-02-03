#include "transmitter.hpp"

#include "schema/server.grpc.pb.h"

// TODO: make an actual directory structure here! do not just do giant .ndjson
// files
Transmitter::Transmitter(const Crypto& crypto, Config& config,
                         unique_ptr<asphrserver::Server::Stub>& stub)
    : crypto(crypto),
      config(config),
      stub(stub),
      inbox(config.data_dir / "inbox.json"),
      outbox(config.data_dir / "outbox.json") {
  check_rep();
}

auto Transmitter::retrieve_messages() -> void {
  check_rep();
  if (!config.has_registered) {
    cout << "hasn't registered yet, so cannot retrieve messages" << endl;
    return;
  }

  auto& client = *config.pir_client;
  asphrserver::ReceiveMessageInfo request;

  // choose a friend to receive from!!
  // priority 1: the friend that we just sent a message to
  // priority 2: the friend that we successfully received a message from the
  // previous round priority 3: random!
  Friend friend_info;
  bool dummy = false;
  if (config.friendTable.contains(just_sent_friend)) {
    friend_info = config.friendTable.at(just_sent_friend);
  } else if (config.friendTable.contains(previous_success_receive_friend)) {
    friend_info = config.friendTable.at(previous_success_receive_friend);
  } else if (config.num_enabled_friends() > 0) {
    // note: we do not need cryptographic randomness here. randomness is only
    // for liveness
    friend_info = config.friendTable.at(config.random_enabled_friend());
  } else {
    friend_info = config.dummyMe;
    dummy = true;
  }

  auto query = client.query(friend_info.read_index, config.db_rows);

  auto serialized_query = query.serialize_to_string();

  request.set_pir_query(serialized_query);

  asphrserver::ReceiveMessageResponse reply;
  grpc::ClientContext context;

  grpc::Status status = stub->ReceiveMessage(&context, request, &reply);

  previous_success_receive_friend = "";

  check_rep();

  // if dummy, we do not actually care about the answer.
  // we still do the rpc to not leak information.
  if (dummy) {
    return;
  }

  if (status.ok()) {
    cout << "received message!!!" << endl;

    Friend& friend_info_mut = config.friendTable.at(friend_info.name);
    auto message_opt =
        inbox.receive_message(client, reply, friend_info_mut, crypto,
                              previous_success_receive_friend);
    if (message_opt.has_value()) {
      auto message = message_opt.value();
      cout << "actual message: " << message.message << endl;
      auto file =
          std::ofstream(config.receive_file_address(), std::ios_base::app);

      auto time = absl::FormatTime(absl::Now(), absl::UTCTimeZone());
      json jmsg = {{"from", message.friend_name},
                   {"timestamp", time},
                   {"id", message.id},
                   {"message", message.message},
                   {"type", "MESSAGE_RECEIVED"}};
      if (file.is_open()) {
        file << std::setw(4) << jmsg.dump() << std::endl;
        cout << jmsg.dump() << endl;
        file.close();
      }
    } else {
      cout << "no message received" << endl;
    }
  } else {
    cout << status.error_code() << ": " << status.error_message() << endl;
  }
  check_rep();
}

auto Transmitter::send_messages() -> void {
  check_rep();
  if (!config.has_registered) {
    cout << "hasn't registered yet, so don't send a message" << endl;
    return;
  }

  auto new_entries =
      get_new_entries(config.send_file_address(), last_ui_timestamp);
  last_ui_timestamp = absl::Now();

  auto authentication_token = config.registrationInfo.authentication_token;

  std::unordered_map<string, string> friend_to_message;

  for (auto& entry : new_entries) {
    auto to = entry["to"].get<string>();
    if (!config.friendTable.contains(to)) {
      std::cerr << "FriendHashTable does not contain " << to
                << "; ignoring message" << endl;
      continue;
    }
    auto friend_info = config.friendTable.at(to);
    outbox.add(entry["message"].get<string>(), friend_info);
  }

  auto messageToSend =
      outbox.message_to_send(config.friendTable, config.dummyMe);
  // always send to the 0th index. currently we only support one index per
  // person!! in the future, this may change, so please don't assert that the
  // length is 1.
  auto index = config.registrationInfo.allocation.at(0);

  just_sent_friend = messageToSend.to.name;

  auto pir_value_message_status =
      crypto.encrypt_send(messageToSend.to_proto(), messageToSend.to);
  if (!pir_value_message_status.ok()) {
    cout << "encryption failed; not doing anything with message"
         << pir_value_message_status.status() << endl;
    return;
  }
  auto pir_value_message = pir_value_message_status.value();

  auto pir_encrypted_acks_status =
      inbox.get_encrypted_acks(config.friendTable, crypto, config.dummyMe);
  if (!pir_encrypted_acks_status.ok()) {
    cout << "encryption failed; not doing anything with message"
         << pir_encrypted_acks_status.status() << endl;
    return;
  }
  auto pir_encrypted_acks = pir_encrypted_acks_status.value();

  cout << "Sending message to server: " << endl;
  cout << "index: " << index << endl;
  cout << "authentication_token: " << authentication_token << endl;
  check_rep();

  // call register rpc to send the register request
  asphrserver::SendMessageInfo request;

  request.set_index(index);
  request.set_authentication_token(authentication_token);

  string padded_msg_str = "";
  for (auto& c : pir_value_message) {
    padded_msg_str += c;
  }
  request.set_message(padded_msg_str);

  string padded_acks_str = "";
  for (auto& c : pir_encrypted_acks) {
    padded_acks_str += c;
  }
  request.set_acks(padded_acks_str);

  asphrserver::SendMessageResponse reply;

  grpc::ClientContext context;

  grpc::Status status = stub->SendMessage(&context, request, &reply);

  if (status.ok()) {
    std::cout << "Message sent to server!" << std::endl;
  } else {
    std::cerr << status.error_code() << ": " << status.error_message()
              << " details:" << status.error_details() << std::endl;
  }
  check_rep();
}

auto Transmitter::check_rep() const noexcept -> void {
  // TODO: figure out how to check for dangling references
  // assert(&crypto != nullptr);
  // assert(&config != nullptr);
  // assert(&stub != nullptr);
  assert(config.has_registered || !config.has_registered);
}