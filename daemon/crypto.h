#include <sodium.h>

#include <stdexcept>
#include <string>

#include "schema/message.pb.h"

using client::Message;

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

  // note: it is CRUCIAL that we use a new random nonce EVERY SINGLE TIME.
  // for message integrity and indistinguishability this is not a problem, but
  // for privacy this is very very important — because if we use a nonce that
  // is non-random, an attacker can observe when the nonce follows the pattern
  // and when it does not, and then figure out when a user switches from talking
  // to one friend to talking to another.
  //
  // this highlights that we need something stronger than IND-CCA2, actually. we
  // also need a ciphertext from one key to be indistinguishable from a
  // ciphertext from another key. in other words, we need key privacy.
  // Block-cipher based symmetric-key encryption schemes do support this out of
  // the box, but it is important to keep in mind because it is a somewhat
  // nonstandard requirement.
  auto encrypt_send(const Message& message, const Friend& friend_info)
      -> std::pair<pir_value_t, int> {
    if (friend_info.write_key.size() !=
        crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
      return std::make_pair(pir_value_t{}, -1);
    }

    std::string ciphertext;
    ciphertext.resize(MESSAGE_SIZE);
    unsigned long long ciphertext_len;

    // truncate the message if it is too long
    // TODO: split into messages
    if (message.msg().size() > GUARANTEED_SINGLE_MESSAGE_SIZE) {
      message.mutable_msg()->resize(GUARANTEED_SINGLE_MESSAGE_SIZE);
    }

    std::string plaintext;
    if (!message.SerializeToString(&plaintext)) {
      return std::make_pair(pir_value_t{}, -1);
    }
    assert(plaintext.size() <= MESSAGE_SIZE - crypto_secretbox_MACBYTES);

    plaintext.resize(MESSAGE_SIZE - crypto_secretbox_MACBYTES);

    size_t padded_plaintext_len;
    if (sodium_pad(&padded_plaintext_len, plaintext.data(), plaintext.size(),
                   MESSAGE_SIZE - crypto_secretbox_MACBYTES,
                   MESSAGE_SIZE - crypto_secretbox_MACBYTES) != 0) {
      return std::make_pair(pir_value_t{}, -1);
    }

    assert(padded_plaintext_len == MESSAGE_SIZE - crypto_secretbox_MACBYTES);
    assert(padded_plaintext_len == plaintext.size());

    // TODO: send acks.

    // encrypt it!

    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    randombytes_buf(nonce, sizeof nonce);

    if (crypto_aead_xchacha20poly1305_ietf_encrypt(
            reinterpret_cast<unsigned char*>(ciphertext.data()),
            &ciphertext_len,
            reinterpret_cast<const unsigned char*>(plaintext.data()),
            plaintext.size(), nullptr, 0, nullptr, nonce,
            reinterpret_cast<const unsigned char*>(
                friend_info.write_key.data())) != 0) {
      return std::make_pair(pir_value_t{}, -1);
    }

    assert(ciphertext_len == MESSAGE_SIZE);

    pir_value_t pir_ciphertext;
    std::copy(ciphertext.begin(), ciphertext.end(), pir_ciphertext.begin());

    return std::make_pair(pir_ciphertext, 0);
  }

  auto decrypt_receive(const pir_value_t& ciphertext, const Friend& friend_info)
      -> Message {
    // if (key.size() != crypto_secretbox_KEYBYTES) {
    //   throw std::runtime_error("key size is not 32 bytes");
    // }

    // std::string plaintext;
    // plaintext.resize(ciphertext.size() - crypto_secretbox_MACBYTES);

    // if (crypto_secretbox_open_easy(
    //         reinterpret_cast<unsigned char*>(&plaintext[0]),
    //         reinterpret_cast<const unsigned char*>(&ciphertext[0]),
    //         ciphertext.size(),
    //         reinterpret_cast<const unsigned char*>(&key[0]),
    //         reinterpret_cast<const unsigned char*>(&nonce_[0])) != 0) {
    //   throw std::runtime_error("decrypt failed");
    // }

    return ciphertext;
  }
};