#pragma once

#include "asphr/asphr.hpp"
#include "config.hpp"

struct BaseMessage {
  // id is a unique identifier among all messages on this device
  string id;
  string message;
};
struct IncomingMessage : BaseMessage {
  string from;
  // received_timestamp is the time at which the message was received, by us. it
  // is NOT when the message was sent.
  absl::Time received_timestamp;
  bool seen;

  auto to_json() const -> asphr::json;
  static auto from_json(const asphr::json& j) -> IncomingMessage;
};
struct OutgoingMessage : BaseMessage {
  string to;
  // written_timestamp is the time at which the message was written, NOT when it
  // was sent or delivered or read.
  absl::Time written_timestamp;
  bool delivered;

  auto to_json() const -> asphr::json;
  static auto from_json(const asphr::json& j) -> OutgoingMessage;
};

/**
 * @brief Msgstore is the threadsafe source of truth for all messages.
 *
 * If a message is in the Msgstore, it MUST NOT be in the Inbox. It MUST NOT be
 * in the Outbox if .delivered is true, and it MAY be in the Outbox if
 * .delivered is false.
 */
class Msgstore {
 public:
  Msgstore(const string& file_address, shared_ptr<Config> config);
  Msgstore(const asphr::json& serialized_json, const string& file_address,
           shared_ptr<Config> config);

  // add_outgoing_message adds a message to the Msgstore, to be sent.
  // returns a status that is not ok if, for example, the friend doesn't exist
  auto add_outgoing_message(const string& to, const string& message)
      -> asphr::Status;
  // deliver_outgoing_message will mark the message as delivered, which means
  // that it will never be sent again. only call this if ACKs for all of this
  // message's chunks have been received.
  auto deliver_outgoing_message(const string& id) -> asphr::Status;

  // add_incoming_message adds a message that was received to the Msgstore.
  // call this only in a transaction with removing all of the message chunks
  // from the Inbox.
  auto add_incoming_message(int sequence_number, const string& from,
                            const string& message) -> void;
  auto mark_message_as_seen(const string& id) -> asphr::Status;

  // TODO: we want some kind of message querying interface here..... maybe we
  // should just expose the raw SQL??? otherwise we kinda need to invent our own
  // querying system that we then parse into SQL which seems a little
  // over-engineered. get_incoming_message returns all messages that have been
  // received
  auto get_all_incoming_messages_sorted() -> vector<IncomingMessage>;
  auto get_new_incoming_messages_sorted() -> vector<IncomingMessage>;

  auto get_undelivered_outgoing_messages_sorted() -> vector<OutgoingMessage>;
  auto get_delivered_outgoing_messages_sorted() -> vector<OutgoingMessage>;

 private:
  // TODO: remove this mutex! eventually, all data should only ever be read and
  // written to the database on every call, and never stored in memory.
  mutable std::mutex msgstore_mtx;

  const string saved_file_address;

  shared_ptr<Config> config;

  // TODO: don't store any data here! everything should be stored just in the
  // database and never in memory.
  vector<IncomingMessage> incoming;
  vector<OutgoingMessage> outgoing;

  auto message_id(bool from, const string& person, int sequence_number)
      -> string;

  auto check_rep() const -> void;
  auto save() -> void;
};