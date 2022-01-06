#include <sodium.h>

#include <stdexcept>
#include <string>

/* Crypto implements an IND-CCA2 secure scheme.

*/
class Crypto {
 public:
  Crypto() {
    if (sodium_init() < 0) {
      throw std::runtime_error("sodium_init failed");
    }
  }

  // TODO: the message should be a protobuf? yes definitely.
  auto encrypt_send(const std::string& message, const Friend& friend_info)
      -> pir_value_t {
    // if (key.size() != crypto_secretbox_KEYBYTES) {
    //   throw std::runtime_error("key size is not 32 bytes");
    // }

    // std::string ciphertext;
    // ciphertext.resize(plaintext.size() + crypto_secretbox_MACBYTES);

    // if (crypto_secretbox_easy(
    //         reinterpret_cast<unsigned char*>(&ciphertext[0]),
    //         reinterpret_cast<const unsigned char*>(&plaintext[0]),
    //         plaintext.size(),
    //         reinterpret_cast<const unsigned char*>(&key[0]),
    //         reinterpret_cast<const unsigned char*>(&nonce_[0])) != 0) {
    //   throw std::runtime_error("encrypt failed");
    // }

    return plaintext;
  }

  auto decrypt_receive(const pir_value_t& ciphertext, const Friend& friend_info)
      -> std::string {
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