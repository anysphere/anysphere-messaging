
#include "msgstore.hpp"

auto Msgstore::get_all_incoming_messages_sorted() -> vector<IncomingMessage> {
  //   // sort messages
  //   std::sort(messages.begin(), messages.end(), [](const json& a, const json&
  //   b) {
  //     auto timestamp_str = a.at("timestamp").get<string>();
  //     Time a_time;
  //     string err;
  //     absl::ParseTime(absl::RFC3339_full, timestamp_str, &a_time, &err);

  //     auto timestamp_str2 = b.at("timestamp").get<string>();
  //     Time b_time;
  //     absl::ParseTime(absl::RFC3339_full, timestamp_str2, &b_time, &err);

  //     // convert timestamp into time_t
  //     return a_time > b_time;
  //   });
  return {};
}

auto Msgstore::message_id(bool from, const string& person, int sequence_number)
    -> string {
  const auto fromOrTo = from ? "from" : "to";
  return asphr::StrCat(fromOrTo, ":", person, ":", sequence_number);
}