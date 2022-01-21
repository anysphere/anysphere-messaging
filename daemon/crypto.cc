#include "crypto.hpp"

#include "constants.hpp"

auto Crypto::generate_keypair() const -> std::pair<string, string> {
  unsigned char public_key[crypto_kx_PUBLICKEYBYTES];
  unsigned char secret_key[crypto_kx_SECRETKEYBYTES];
  crypto_kx_keypair(public_key, secret_key);
  return {
      string(reinterpret_cast<char*>(public_key), crypto_kx_PUBLICKEYBYTES),
      string(reinterpret_cast<char*>(secret_key), crypto_kx_SECRETKEYBYTES)};
}

auto Crypto::generate_friend_key(const string& my_public_key, int index) const
    -> string {
  string public_key_b64;
  public_key_b64.resize(sodium_base64_ENCODED_LEN(
      my_public_key.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING));
  sodium_bin2base64(
      public_key_b64.data(), public_key_b64.size(),
      reinterpret_cast<const unsigned char*>(my_public_key.data()),
      my_public_key.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING);

  // TODO: figure out a better way to encode both index and public key
  return asphr::StrCat(index, "a", public_key_b64);
}

auto Crypto::decode_friend_key(const string& friend_key) const
    -> asphr::StatusOr<std::pair<int, string>> {
  string index_str;
  string public_key_b64;
  for (size_t i = 0; i < friend_key.size(); ++i) {
    if (friend_key[i] == 'a') {
      public_key_b64 = friend_key.substr(i + 1);
      break;
    }
    index_str += friend_key[i];
  }
  int index;
  auto success = absl::SimpleAtoi(index_str, &index);
  if (!success) {
    return asphr::InvalidArgumentError("friend key index must be an integer");
  }

  string public_key;
  public_key.resize(public_key_b64.size());
  size_t public_key_len;
  const char* b64_end;
  sodium_base642bin(reinterpret_cast<unsigned char*>(public_key.data()),
                    public_key.size(), public_key_b64.data(),
                    public_key_b64.size(), "", &public_key_len, &b64_end,
                    sodium_base64_VARIANT_URLSAFE_NO_PADDING);
  public_key.resize(public_key_len);

  return make_pair(index, public_key);
}

auto Crypto::derive_read_write_keys(string my_public_key, string my_private_key,
                                    string friend_public_key) const
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
            reinterpret_cast<const unsigned char*>(friend_public_key.data())) !=
        0) {
      throw std::runtime_error("crypto_kx_client_session_keys failed");
    }
  } else {
    // assume role of server
    if (crypto_kx_server_session_keys(
            reinterpret_cast<unsigned char*>(read_key.data()),
            reinterpret_cast<unsigned char*>(write_key.data()),
            reinterpret_cast<const unsigned char*>(my_public_key.data()),
            reinterpret_cast<const unsigned char*>(my_private_key.data()),
            reinterpret_cast<const unsigned char*>(friend_public_key.data())) !=
        0) {
      throw std::runtime_error("crypto_kx_client_session_keys failed");
    }
  }
  static_assert(
      crypto_aead_xchacha20poly1305_ietf_KEYBYTES <= crypto_kx_SESSIONKEYBYTES,
      "crypto_aead_xchacha20poly1305_ietf_KEYBYTES is too large");
  read_key.resize(crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
  write_key.resize(crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
  return std::make_pair(read_key, write_key);
}

auto Crypto::encrypt_send(const asphrclient::Message& message_in,
                          const Friend& friend_info) const
    -> asphr::StatusOr<pir_value_t> {
  auto message = message_in;
  if (friend_info.write_key.size() !=
      crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
    return asphr::InvalidArgumentError(
        "friend_info.write_key is not the correct size");
  }

  std::string ciphertext;
  ciphertext.resize(MESSAGE_SIZE);
  unsigned long long ciphertext_len;

  // truncate the message if it is too long
  // this should never happne, but just in case
  if (message.msg().size() > GUARANTEED_SINGLE_MESSAGE_SIZE) {
    message.mutable_msg()->resize(GUARANTEED_SINGLE_MESSAGE_SIZE);
  }

  std::string plaintext;
  if (!message.SerializeToString(&plaintext)) {
    return absl::UnknownError("failed to serialize message");
  }
  auto unpadded_plaintext_len = plaintext.size();
  auto plaintext_max_len = MESSAGE_SIZE -
                           crypto_aead_xchacha20poly1305_ietf_ABYTES -
                           crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
  assert(unpadded_plaintext_len < plaintext_max_len);

  plaintext.resize(plaintext_max_len);

  size_t padded_plaintext_len;
  if (sodium_pad(&padded_plaintext_len,
                 reinterpret_cast<unsigned char*>(plaintext.data()),
                 unpadded_plaintext_len, plaintext_max_len,
                 plaintext_max_len) != 0) {
    return absl::UnknownError("failed to pad plaintext");
  }

  assert(padded_plaintext_len == plaintext_max_len);
  assert(padded_plaintext_len == plaintext.size());

  // encrypt it!

  unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
  randombytes_buf(nonce, sizeof nonce);

  if (crypto_aead_xchacha20poly1305_ietf_encrypt(
          reinterpret_cast<unsigned char*>(ciphertext.data()), &ciphertext_len,
          reinterpret_cast<const unsigned char*>(plaintext.data()),
          plaintext.size(), nullptr, 0, nullptr, nonce,
          reinterpret_cast<const unsigned char*>(
              friend_info.write_key.data())) != 0) {
    return absl::UnknownError("failed to encrypt message");
  }

  assert(ciphertext_len ==
         MESSAGE_SIZE - crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);

  pir_value_t pir_ciphertext;
  std::copy(ciphertext.begin(), ciphertext.end(), pir_ciphertext.begin());
  for (size_t i = 0; i < crypto_aead_xchacha20poly1305_ietf_NPUBBYTES; i++) {
    pir_ciphertext[i + ciphertext_len] = nonce[i];
  }

  return pir_ciphertext;
}

auto Crypto::decrypt_receive(const pir_value_t& ciphertext,
                             const Friend& friend_info) const
    -> asphr::StatusOr<asphrclient::Message> {
  auto ciphertext_len =
      MESSAGE_SIZE - crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
  string ciphertext_str = "";
  for (size_t i = 0; i < ciphertext_len; i++) {
    ciphertext_str += ciphertext[i];
  }
  unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
  for (size_t i = 0; i < crypto_aead_xchacha20poly1305_ietf_NPUBBYTES; i++) {
    nonce[i] = ciphertext[i + ciphertext_len];
  }
  auto plaintext_max_len = MESSAGE_SIZE -
                           crypto_aead_xchacha20poly1305_ietf_ABYTES -
                           crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

  if (friend_info.read_key.size() !=
      crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
    return asphr::InvalidArgumentError(
        "friend_info.read_key is not the correct size");
  }

  std::string plaintext;
  plaintext.resize(plaintext_max_len);
  unsigned long long plaintext_len;
  if (crypto_aead_xchacha20poly1305_ietf_decrypt(
          reinterpret_cast<unsigned char*>(plaintext.data()), &plaintext_len,
          nullptr,
          reinterpret_cast<const unsigned char*>(ciphertext_str.data()),
          ciphertext_str.size(), nullptr, 0, nonce,
          reinterpret_cast<const unsigned char*>(
              friend_info.read_key.data())) != 0) {
    return absl::UnknownError("failed to decrypt message");
  }

  assert(plaintext_len == plaintext_max_len);
  size_t unpadded_plaintext_len;
  if (sodium_unpad(&unpadded_plaintext_len,
                   reinterpret_cast<const unsigned char*>(plaintext.data()),
                   plaintext.size(), plaintext_max_len) != 0) {
    return absl::UnknownError("failed to unpad plaintext");
  }

  assert(unpadded_plaintext_len <= plaintext_len);

  plaintext.resize(unpadded_plaintext_len);

  asphrclient::Message message;
  if (!message.ParseFromString(plaintext)) {
    return absl::UnknownError("failed to parse message");
  }

  return message;
}

// encrypt_ack encrypts the ack_id to the friend
auto Crypto::encrypt_ack(uint32_t ack_id, const Friend& friend_info) const
    -> asphr::StatusOr<string> {
  if (friend_info.write_key.size() !=
      crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
    return asphr::InvalidArgumentError(
        "friend_info.write_key is not the correct size");
  }
  std::string ciphertext;
  ciphertext.resize(ENCRYPTED_ACKING_BYTES);
  unsigned long long ciphertext_len;

  auto ack_id_str = reinterpret_cast<char*>(&ack_id);
  std::string plaintext(ack_id_str, 4);
  auto plaintext_len = plaintext.size();
  assert(plaintext_len == ACKING_BYTES);

  unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
  randombytes_buf(nonce, sizeof nonce);

  if (crypto_aead_xchacha20poly1305_ietf_encrypt(
          reinterpret_cast<unsigned char*>(ciphertext.data()), &ciphertext_len,
          reinterpret_cast<const unsigned char*>(plaintext.data()),
          plaintext.size(), nullptr, 0, nullptr, nonce,
          reinterpret_cast<const unsigned char*>(
              friend_info.write_key.data())) != 0) {
    return absl::UnknownError("failed to encrypt message");
  }

  assert(ciphertext_len ==
         ENCRYPTED_ACKING_BYTES - crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);

  for (size_t i = 0; i < crypto_aead_xchacha20poly1305_ietf_NPUBBYTES; i++) {
    ciphertext[i + ciphertext_len] += nonce[i];
  }

  return ciphertext;
}
// decrypt_ack undoes encrypt_ack
auto Crypto::decrypt_ack(const string& ciphertext,
                         const Friend& friend_info) const
    -> asphr::StatusOr<uint32_t> {
  if (friend_info.read_key.size() !=
      crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
    return asphr::InvalidArgumentError(
        "friend_info.read_key is not the correct size");
  }
  auto ciphertext_len =
      ENCRYPTED_ACKING_BYTES - crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
  string ciphertext_str = "";
  for (size_t i = 0; i < ciphertext_len; i++) {
    ciphertext_str += ciphertext[i];
  }
  unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
  for (size_t i = 0; i < crypto_aead_xchacha20poly1305_ietf_NPUBBYTES; i++) {
    nonce[i] = ciphertext[i + ciphertext_len];
  }
  std::string plaintext;
  plaintext.resize(ACKING_BYTES);
  unsigned long long plaintext_len;
  if (crypto_aead_xchacha20poly1305_ietf_decrypt(
          reinterpret_cast<unsigned char*>(plaintext.data()), &plaintext_len,
          nullptr,
          reinterpret_cast<const unsigned char*>(ciphertext_str.data()),
          ciphertext_str.size(), nullptr, 0, nonce,
          reinterpret_cast<const unsigned char*>(
              friend_info.read_key.data())) != 0) {
    // DEBUG PRINT
    return absl::UnknownError("failed to decrypt message");
  }

  assert(plaintext_len == ACKING_BYTES);

  uint32_t ack_id = reinterpret_cast<uint32_t*>(plaintext.data())[0];

  return ack_id;
}