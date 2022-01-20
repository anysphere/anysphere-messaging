#pragma once

#include <sodium.h>

#include <stdexcept>
#include <string>

#include "asphr/asphr.hpp"
#include "client/client_lib/client_lib.hpp"
#include "config.hpp"
#include "schema/message.pb.h"

using std::string;

/* Crypto implements an IND-CCA2 secure scheme.

Currently, we do not support forward secrecy. Note that this really only becomes
a problem if we want to be able to delete messages, because otherwise an
attacker that compromises the keys will also be able to read messages.
*/
class Crypto {
 public:
  Crypto() {
    if (sodium_init() < 0) {
      throw std::runtime_error("sodium_init failed");
    }
  }
  // Generates a new keypair in the format <public_key, private_key>.
  auto generate_keypair() const -> std::pair<string, string>;

  auto generate_friend_key(const string& my_public_key, int index) const
      -> string;
  auto decode_friend_key(const string& friend_key) const
      -> asphr::StatusOr<std::pair<int, string>>;

  auto derive_read_write_keys(string my_public_key, string my_private_key,
                              string friend_public_key) const
      -> std::pair<string, string>;

  // note: it is CRUCIAL that we use a new random nonce EVERY SINGLE TIME.
  // for message integrity and indistinguishability this is not a problem,
  // but for privacy this is very very important — because if we use a nonce
  // that is non-random, an attacker can observe when the nonce follows the
  // pattern and when it does not, and then figure out when a user switches
  // from talking to one friend to talking to another.
  //
  // this highlights that we need something stronger than IND-CCA2,
  // actually. we also need a ciphertext from one key to be
  // indistinguishable from a ciphertext from another key. in other words,
  // we need key privacy. Block-cipher based symmetric-key encryption
  // schemes do support this out of the box, but it is important to keep in
  // mind because it is a somewhat nonstandard requirement.
  auto encrypt_send(const asphrclient::Message& message_in,
                    const Friend& friend_info) const
      -> asphr::StatusOr<pir_value_t>;

  auto decrypt_receive(const pir_value_t& ciphertext,
                       const Friend& friend_info) const
      -> asphr::StatusOr<asphrclient::Message>;

  // encrypt_ack encrypts the ack_id to the friend
  auto encrypt_ack(uint32_t ack_id, const Friend& friend_info) const
      -> asphr::StatusOr<string>;
  // decrypt_ack undoes encrypt_ack
  auto decrypt_ack(const string& ciphertext, const Friend& friend_info) const
      -> asphr::StatusOr<uint32_t>;
};