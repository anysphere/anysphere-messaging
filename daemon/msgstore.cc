
#include "msgstore.hpp"

auto IncomingMessage::to_json() const -> asphr::json {
  return asphr::json{
      {"id", id},
      {"message", message},
      {"from", from},
      {"received_timestamp", absl::FormatTime(received_timestamp)},
      {"mono_index", mono_index},
      {"seen", seen},
  };
}

auto IncomingMessage::from_json(const asphr::json& j) -> IncomingMessage {
  IncomingMessage msg;
  msg.id = j.at("id").get<string>();
  msg.message = j.at("message").get<string>();
  msg.from = j.at("from").get<string>();
  string err;
  absl::ParseTime(absl::RFC3339_full, j.at("received_timestamp").get<string>(),
                  &msg.received_timestamp, &err);
  if (!err.empty()) {
    throw std::runtime_error("error parsing time: " + err);
  }
  msg.seen = j.at("seen").get<bool>();
  msg.mono_index = j.at("mono_index").get<int>();
  return msg;
}

auto OutgoingMessage::to_json() const -> asphr::json {
  return asphr::json{
      {"id", id},
      {"message", message},
      {"to", to},
      {"written_timestamp", absl::FormatTime(written_timestamp)},
      {"delivered", delivered},
  };
}

auto OutgoingMessage::from_json(const asphr::json& j) -> OutgoingMessage {
  OutgoingMessage msg;
  msg.id = j.at("id").get<string>();
  msg.message = j.at("message").get<string>();
  msg.to = j.at("to").get<string>();
  string err;
  absl::ParseTime(absl::RFC3339_full, j.at("written_timestamp").get<string>(),
                  &msg.written_timestamp, &err);
  if (!err.empty()) {
    throw std::runtime_error("error parsing time: " + err);
  }
  msg.delivered = j.at("delivered").get<bool>();
  return msg;
}

auto read_msgstore_json(const string& file_address) -> asphr::json {
  if (!std::filesystem::exists(file_address) ||
      std::filesystem::file_size(file_address) == 0) {
    auto dir_path =
        std::filesystem::path(file_address).parent_path().u8string();
    std::filesystem::create_directories(dir_path);
    cout << "creating new msgstore asphr::json!" << endl;
    asphr::json j = {{"incoming", asphr::json::array()},
                     {"outgoing", asphr::json::array()},
                     {"last_mono_index", 0}};
    std::ofstream o(file_address);
    o << std::setw(4) << j.dump(4) << std::endl;
  }
  auto json = asphr::json::parse(std::ifstream(file_address));
  return json;
}

Msgstore::Msgstore(const string& file_address, shared_ptr<Config> config)
    : Msgstore(read_msgstore_json(file_address), file_address, config) {}

Msgstore::Msgstore(const asphr::json& serialized_json,
                   const string& file_address, shared_ptr<Config> config)
    : last_mono_index(0), saved_file_address(file_address), config(config) {
  for (auto& messageJson : serialized_json.at("incoming")) {
    auto message = IncomingMessage::from_json(messageJson);
    incoming.push_back(message);
  }
  for (auto& messageJson : serialized_json.at("outgoing")) {
    auto message = OutgoingMessage::from_json(messageJson);
    outgoing.push_back(message);
  }
  if (serialized_json.contains("last_mono_index")) {
    last_mono_index = serialized_json.at("last_mono_index").get<int>();
  }

  check_rep();
}

auto Msgstore::save() noexcept(false) -> void {
  check_rep();
  asphr::json j = {{"incoming", {}}, {"outgoing", {}}};
  for (auto& m : incoming) {
    j.at("incoming").push_back(m.to_json());
  }
  for (auto& m : outgoing) {
    j.at("outgoing").push_back(m.to_json());
  }
  std::ofstream o(saved_file_address);
  o << std::setw(4) << j.dump(4) << std::endl;
  check_rep();
}

auto Msgstore::add_outgoing_message(const string& to, const string& message)
    -> asphr::Status {
  const std::lock_guard<std::mutex> l(msgstore_mtx);

  check_rep();

  // generate a unique id for this message
  // we could store a count per user for this, and we probably should...

  auto f_status = config->get_friend(to);
  if (!f_status.ok()) {
    std::cerr << "FriendHashTable does not contain " << to
              << "; ignoring message" << endl;
    return f_status.status();
  }

  const auto id = message_id(false, to, f_status.value().latest_send_id);

  auto new_friend = f_status.value();
  new_friend.latest_send_id++;
  config->update_friend(new_friend);

  OutgoingMessage outgoing_message{{id, message}, to, absl::Now(), false};
  outgoing.push_back(outgoing_message);

  save();
  check_rep();

  return absl::OkStatus();
}
// deliver_outgoing_message will mark the message as delivered, which means
// that it will never be sent again. only call this if ACKs for all of this
// message's chunks have been received.
auto Msgstore::deliver_outgoing_message(const string& id) -> asphr::Status {
  const std::lock_guard<std::mutex> l(msgstore_mtx);

  check_rep();

  for (auto& m : outgoing) {
    if (m.id == id) {
      if (m.delivered) {
        std::cerr << "message " << id << " already delivered" << endl;
        return absl::UnknownError("message already delivered");
      }
      m.delivered = true;
      save();
      check_rep();
      return absl::OkStatus();
    }
  }

  return absl::NotFoundError("id not found");
}

// add_incoming_message adds a message that was received to the Msgstore.
// call this only in a transaction with removing all of the message chunks
// from the Inbox.
auto Msgstore::add_incoming_message(int sequence_number, const string& from,
                                    const string& message) -> void {
  const std::lock_guard<std::mutex> l(msgstore_mtx);

  check_rep();

  auto mono_index = last_mono_index + 1;

  const auto id = message_id(true, from, sequence_number);

  IncomingMessage incoming_message{{id, message}, from, absl::Now(), false, mono_index};
  incoming.push_back(incoming_message);

  save();
  check_rep();

  // only after everything is committed (i.e. saved), we can notify the add_cv
  {
    std::lock_guard<std::mutex> l(add_cv_mtx);
    last_mono_index = mono_index;
    add_cv.notify_all();
  }
}

auto Msgstore::mark_message_as_seen(const string& id) -> asphr::Status {
  const std::lock_guard<std::mutex> l(msgstore_mtx);

  check_rep();

  for (auto& m : incoming) {
    if (m.id == id) {
      if (m.seen) {
        std::cerr << "message " << id << " already seen" << endl;
        return absl::UnknownError("message already seen");
      }
      m.seen = true;
      save();
      check_rep();
      return absl::OkStatus();
    }
  }

  return absl::NotFoundError("id not found");
}

auto Msgstore::get_incoming_message_by_id(const string& id) -> asphr::StatusOr<IncomingMessage> {
  const std::lock_guard<std::mutex> l(msgstore_mtx);

  check_rep();

  for (auto& m : incoming) {
    if (m.id == id) {
      return m;
    }
  }

  return absl::NotFoundError("id not found");
}

// TODO: we want some kind of message querying interface here..... maybe we
// should just expose the raw SQL??? otherwise we kinda need to invent our own
// querying system that we then parse into SQL which seems a little
// over-engineered. get_incoming_message returns all messages that have been
// received
auto Msgstore::get_all_incoming_messages_sorted() -> vector<IncomingMessage> {
  const std::lock_guard<std::mutex> l(msgstore_mtx);

  check_rep();

  vector<IncomingMessage> sorted_incoming;
  sorted_incoming.reserve(incoming.size());
  for (auto& m : incoming) {
    sorted_incoming.push_back(m);
  }
  std::sort(sorted_incoming.begin(), sorted_incoming.end(),
            [](const IncomingMessage& a, const IncomingMessage& b) {
              return a.received_timestamp > b.received_timestamp;
            });
  return sorted_incoming;
}

auto Msgstore::get_incoming_messages_sorted_after(int after_mono_index)
    -> vector<IncomingMessage> {
  const std::lock_guard<std::mutex> l(msgstore_mtx);

  check_rep();

  vector<IncomingMessage> sorted_incoming;
  sorted_incoming.reserve(incoming.size());
  for (auto& m : incoming) {
    if (m.mono_index > after_mono_index) {
      sorted_incoming.push_back(m);
    }
  }
  std::sort(sorted_incoming.begin(), sorted_incoming.end(),
            [](const IncomingMessage& a, const IncomingMessage& b) {
              return a.received_timestamp > b.received_timestamp;
            });
  return sorted_incoming;
}

auto Msgstore::get_new_incoming_messages_sorted() -> vector<IncomingMessage> {
  const std::lock_guard<std::mutex> l(msgstore_mtx);

  check_rep();

  vector<IncomingMessage> sorted_incoming;
  sorted_incoming.reserve(incoming.size());
  for (auto& m : incoming) {
    if (!m.seen) {
      sorted_incoming.push_back(m);
    }
  }
  std::sort(sorted_incoming.begin(), sorted_incoming.end(),
            [](const IncomingMessage& a, const IncomingMessage& b) {
              return a.received_timestamp > b.received_timestamp;
            });
  return sorted_incoming;
}

auto Msgstore::get_undelivered_outgoing_messages_sorted()
    -> vector<OutgoingMessage> {
  const std::lock_guard<std::mutex> l(msgstore_mtx);

  check_rep();

  vector<OutgoingMessage> undelivered_outgoing;
  undelivered_outgoing.reserve(outgoing.size());
  for (auto& m : outgoing) {
    if (!m.delivered) {
      undelivered_outgoing.push_back(m);
    }
  }
  std::sort(undelivered_outgoing.begin(), undelivered_outgoing.end(),
            [](const OutgoingMessage& a, const OutgoingMessage& b) {
              return a.written_timestamp < b.written_timestamp;
            });
  return undelivered_outgoing;
}

auto Msgstore::get_delivered_outgoing_messages_sorted()
    -> vector<OutgoingMessage> {
  const std::lock_guard<std::mutex> l(msgstore_mtx);

  check_rep();

  vector<OutgoingMessage> delivered_outgoing;
  delivered_outgoing.reserve(outgoing.size());
  for (auto& m : outgoing) {
    if (m.delivered) {
      delivered_outgoing.push_back(m);
    }
  }
  std::sort(delivered_outgoing.begin(), delivered_outgoing.end(),
            [](const OutgoingMessage& a, const OutgoingMessage& b) {
              return a.written_timestamp < b.written_timestamp;
            });
  return delivered_outgoing;
}

auto Msgstore::message_id(bool from, const string& person, int sequence_number)
    -> string {
  const auto fromOrTo = from ? "from" : "to";
  return asphr::StrCat(fromOrTo, ":", person, ":", sequence_number);
}

auto Msgstore::check_rep() const -> void {
  assert(config != nullptr);
  assert(saved_file_address != "");

  // all message IDs need to be unique
  std::set<string> ids;
  for (const auto& m : incoming) {
    ids.insert(m.id);
  }
  for (const auto& m : outgoing) {
    ids.insert(m.id);
  }
  assert(ids.size() == incoming.size() + outgoing.size());
}