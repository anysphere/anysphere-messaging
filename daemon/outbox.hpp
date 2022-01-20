
// Outbox is ONLY concerned with outgoing messages.
class Outbox {
  Outbox(const string& file_address);
  Outbox(const asphr::json& serialized_json, const string& file_address);

  auto save() noexcept(false) -> void;

  // the message here can be any size! Outbox takes care of splitting it into
  // chunks.
  auto add(const string& message, const string& friend) noexcept -> void;

  auto ack(const string& friend, const int ack_id) noexcept -> void;

  auto
}