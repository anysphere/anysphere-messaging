#pragma once

#include <sodium.h>

#include <stdexcept>
#include <string>

#include "anysphere.h"
#include "schema/message.pb.h"

using client::Message;

using std::string;

// TODO: for some weird reason, in normal builds, all sodium functions are in
// the sodium namespace (GREAT), but in tests, they are not... hence why we use
// this to be able to do both. ideally we want tests to have the namespace
// though...
using namespace sodium;

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

  auto gen_keypair() -> std::pair<string, string>;

  auto generate_friend_key(const string& my_public_key, int index) -> string;
  auto decode_friend_key(const string& friend_key) -> std::pair<int, string>;

  auto derive_read_write_keys(string my_public_key, string my_private_key,
                              string friend_public_key)
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
  auto Crypto::encrypt_send(const Message& message_in,
                            const Friend& friend_info)
      -> std::pair<pir_value_t, int>;

  auto Crypto::decrypt_receive(const pir_value_t& ciphertext,
                               const Friend& friend_info)
      -> std::pair<Message, int>;
};