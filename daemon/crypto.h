#include <sodium.h>

#include <stdexcept>
#include <string>

#include "anysphere.h"
#include "schema/message.pb.h"

using client::Message;

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

  auto gen_keypair() -> std::pair<string, string> {
    unsigned char public_key[crypto_kx_PUBLICKEYBYTES];
    unsigned char secret_key[crypto_kx_SECRETKEYBYTES];
    crypto_kx_keypair(public_key, secret_key);
    return {
        string(reinterpret_cast<char*>(public_key), crypto_kx_PUBLICKEYBYTES),
        string(reinterpret_cast<char*>(secret_key), crypto_kx_SECRETKEYBYTES)};
  }

  auto derive_read_write_keys(string my_public_key, string my_private_key,
                              string friend_public_key)
      -> std::pair<string, string> {
    if (my_public_key.size() != crypto_kx_PUBLICKEYBYTES) {
      throw std::runtime_error("my_public_key is not the correct size");
    }
    if (my_private_key.size() != crypto_kx_SECRETKEYBYTES) {
      throw std::runtime_error("my_private_key is not the correct size");
    }
    if (friend_public_key.size() != crypto_kx_PUBLICKEYBYTES) {
      throw std::runtime_error("friend_public_key is not the correct size");
    }
    string read_key;
    read_key.resize(crypto_kx_SESSIONKEYBYTES);
    string write_key;
    write_key.resize(crypto_kx_SESSIONKEYBYTES);
    if (my_public_key < friend_public_key) {
      // assume role of client
      if (crypto_kx_client_session_keys(
              reinterpret_cast<unsigned char*>(read_key.data()),
              reinterpret_cast<unsigned char*>(write_key.data()),
              reinterpret_cast<const unsigned char*>(my_public_key.data()),
              reinterpret_cast<const unsigned char*>(my_private_key.data()),
              reinterpret_cast<const unsigned char*>(
                  friend_public_key.data())) != 0) {
        throw std::runtime_error("crypto_kx_client_session_keys failed");
      }
    } else {
      // assume role of server
      if (crypto_kx_server_session_keys(
              reinterpret_cast<unsigned char*>(read_key.data()),
              reinterpret_cast<unsigned char*>(write_key.data()),
              reinterpret_cast<const unsigned char*>(my_public_key.data()),
              reinterpret_cast<const unsigned char*>(my_private_key.data()),
              reinterpret_cast<const unsigned char*>(
                  friend_public_key.data())) != 0) {
        throw std::runtime_error("crypto_kx_client_session_keys failed");
      }
    }
    static_assert(crypto_aead_xchacha20poly1305_ietf_KEYBYTES <=
                      crypto_kx_SESSIONKEYBYTES,
                  "crypto_aead_xchacha20poly1305_ietf_KEYBYTES is too large");
    read_key.resize(crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    write_key.resize(crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    return std::make_pair(read_key, write_key);
  }

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
  auto encrypt_send(const Message& message_in, const Friend& friend_info)
      -> std::pair<pir_value_t, int> {
    auto message = message_in;
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
    assert(plaintext.size() <=
           MESSAGE_SIZE - crypto_aead_xchacha20poly1305_ietf_ABYTES);

    plaintext.resize(MESSAGE_SIZE - crypto_aead_xchacha20poly1305_ietf_ABYTES);

    size_t padded_plaintext_len;
    if (sodium_pad(&padded_plaintext_len,
                   reinterpret_cast<unsigned char*>(plaintext.data()),
                   plaintext.size(),
                   MESSAGE_SIZE - crypto_aead_xchacha20poly1305_ietf_ABYTES,
                   MESSAGE_SIZE - crypto_aead_xchacha20poly1305_ietf_ABYTES) !=
        0) {
      std::cerr << "sodium_pad failed" << std::endl;
      return std::make_pair(pir_value_t{}, -1);
    }

    assert(padded_plaintext_len ==
           MESSAGE_SIZE - crypto_aead_xchacha20poly1305_ietf_ABYTES);
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
      -> std::pair<Message, int> {
    string ciphertext_str = "";
    for (auto c : ciphertext) {
      ciphertext_str += c;
    }

    if (friend_info.read_key.size() !=
        crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
      return std::make_pair(Message(), -1);
    }

    std::string plaintext;
    plaintext.resize(MESSAGE_SIZE - crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long plaintext_len;
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    if (crypto_aead_xchacha20poly1305_ietf_decrypt(
            reinterpret_cast<unsigned char*>(plaintext.data()), &plaintext_len,
            nullptr,
            reinterpret_cast<const unsigned char*>(ciphertext_str.data()),
            ciphertext_str.size(), nullptr, 0, nonce,
            reinterpret_cast<const unsigned char*>(
                friend_info.read_key.data())) != 0) {
      return std::make_pair(Message(), -1);
    }

    assert(plaintext_len ==
           MESSAGE_SIZE - crypto_aead_xchacha20poly1305_ietf_ABYTES);
    size_t unpadded_plaintext_len;
    if (sodium_unpad(
            &unpadded_plaintext_len,
            reinterpret_cast<const unsigned char*>(plaintext.data()),
            plaintext.size(),
            MESSAGE_SIZE - crypto_aead_xchacha20poly1305_ietf_ABYTES) != 0) {
      return std::make_pair(Message(), -1);
    }

    assert(unpadded_plaintext_len <= plaintext_len);

    plaintext.resize(unpadded_plaintext_len);

    Message message;
    if (!message.ParseFromString(plaintext)) {
      return std::make_pair(Message(), -1);
    }

    return std::make_pair(message, 0);
  }
};