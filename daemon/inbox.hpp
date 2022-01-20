#pragma once

#include "asphr/asphr.hpp"
#include "crypto.hpp"
#include "schema/server.grpc.pb.h"

class Inbox {
 public:
  auto get_encrypted_acks(const std::unordered_map<string, Friend>& friendTable,
                          const Crypto& crypto, const Friend& dummyMe)
      -> asphr::StatusOr<pir_value_t>;

  // modifies friend_info to store the latest ack id!
  auto update_ack_from_friend(pir_value_t& pir_acks, Friend& friend_info,
                              const Crypto& crypto) -> bool;

  // receives a message! returns a message if all of its chunks have been
  // received, and it's a real message.
  auto receive_message(FastPIRClient& client,
                       const asphrserver::ReceiveMessageResponse& reply,
                       Friend& friend_info, const Crypto& crypto,
                       string& previous_success_receive_friend)
      -> std::optional<asphrclient::Message>;
};