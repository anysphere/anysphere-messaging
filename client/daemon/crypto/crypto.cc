//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "crypto.hpp"

namespace crypto {
auto generate_kx_keypair() -> std::pair<string, string> {
  unsigned char public_key[crypto_kx_PUBLICKEYBYTES];
  unsigned char secret_key[crypto_kx_SECRETKEYBYTES];
  crypto_kx_keypair(public_key, secret_key);
  return {
      string(reinterpret_cast<char*>(public_key), crypto_kx_PUBLICKEYBYTES),
      string(reinterpret_cast<char*>(secret_key), crypto_kx_SECRETKEYBYTES)};
}

auto generate_invitation_keypair() -> std::pair<string, string> {
  unsigned char public_key[crypto_box_PUBLICKEYBYTES];
  unsigned char secret_key[crypto_box_SECRETKEYBYTES];
  crypto_box_keypair(public_key, secret_key);
  return {
      string(reinterpret_cast<char*>(public_key), crypto_box_PUBLICKEYBYTES),
      string(reinterpret_cast<char*>(secret_key), crypto_box_SECRETKEYBYTES)};
}

auto generic_hash(string_view data) -> std::string {
  unsigned char hash[crypto_generichash_BYTES];
  crypto_generichash(hash, sizeof hash,
                     reinterpret_cast<const unsigned char*>(data.data()),
                     data.size(), nullptr, 0);
  return string(reinterpret_cast<char*>(hash), sizeof hash);
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

auto encrypt_send(const asphrclient::Chunk& message_in, const string& write_key)
    -> asphr::StatusOr<pir_value_t> {
  auto message = message_in;
  if (write_key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
    return asphr::InvalidArgumentError("write_key is not the correct size");
  }

  std::string ciphertext;
  ciphertext.resize(MESSAGE_SIZE);
  unsigned long long ciphertext_len;

  // truncate the message if it is too long
  // this should never happen, but just in case
  if (message.msg().size() > GUARANTEED_SINGLE_MESSAGE_SIZE) {
    message.mutable_msg()->resize(GUARANTEED_SINGLE_MESSAGE_SIZE);
  }
  // same thing with system message data
  if (message.system_message_data().size() > GUARANTEED_SINGLE_MESSAGE_SIZE) {
    message.mutable_system_message_data()->resize(
        GUARANTEED_SINGLE_MESSAGE_SIZE);
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
          reinterpret_cast<const unsigned char*>(write_key.data())) != 0) {
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

auto decrypt_receive(const pir_value_t& ciphertext, const string& read_key)
    -> asphr::StatusOr<asphrclient::Chunk> {
  auto ciphertext_len =
      MESSAGE_SIZE - crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
  string ciphertext_str = "";
  for (size_t i = 0; i < ciphertext_len; i++) {
    ciphertext_str += ciphertext.at(i);
  }
  unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
  for (size_t i = 0; i < crypto_aead_xchacha20poly1305_ietf_NPUBBYTES; i++) {
    nonce[i] = ciphertext[i + ciphertext_len];
  }
  auto plaintext_max_len = MESSAGE_SIZE -
                           crypto_aead_xchacha20poly1305_ietf_ABYTES -
                           crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

  if (read_key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
    return asphr::InvalidArgumentError("read_key is not the correct size");
  }

  std::string plaintext;
  plaintext.resize(plaintext_max_len);
  unsigned long long plaintext_len;
  if (crypto_aead_xchacha20poly1305_ietf_decrypt(
          reinterpret_cast<unsigned char*>(plaintext.data()), &plaintext_len,
          nullptr,
          reinterpret_cast<const unsigned char*>(ciphertext_str.data()),
          ciphertext_str.size(), nullptr, 0, nonce,
          reinterpret_cast<const unsigned char*>(read_key.data())) != 0) {
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

  asphrclient::Chunk message;
  if (!message.ParseFromString(plaintext)) {
    return absl::UnknownError("failed to parse message");
  }

  return message;
}

// encrypt_ack encrypts the ack_id to the friend
auto encrypt_ack(uint32_t ack_id, const string& write_key)
    -> asphr::StatusOr<string> {
  if (write_key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
    return asphr::InvalidArgumentError("write_key is not the correct size");
  }
  std::string ciphertext;
  ciphertext.resize(ENCRYPTED_ACKING_BYTES);
  unsigned long long ciphertext_len;

  auto ack_id_str = reinterpret_cast<char*>(&ack_id);
  std::string plaintext(ack_id_str, 4);
  auto plaintext_len = plaintext.size();
  (void)plaintext_len;
  assert(plaintext_len == ACKING_BYTES);

  unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
  randombytes_buf(nonce, sizeof nonce);

  if (crypto_aead_xchacha20poly1305_ietf_encrypt(
          reinterpret_cast<unsigned char*>(ciphertext.data()), &ciphertext_len,
          reinterpret_cast<const unsigned char*>(plaintext.data()),
          plaintext.size(), nullptr, 0, nullptr, nonce,
          reinterpret_cast<const unsigned char*>(write_key.data())) != 0) {
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
auto decrypt_ack(const string& ciphertext, const string& read_key)
    -> asphr::StatusOr<uint32_t> {
  if (read_key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
    return asphr::InvalidArgumentError("read_key is not the correct size");
  }
  auto ciphertext_len =
      ENCRYPTED_ACKING_BYTES - crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
  string ciphertext_str = "";
  for (size_t i = 0; i < ciphertext_len; i++) {
    ciphertext_str += ciphertext.at(i);
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
          reinterpret_cast<const unsigned char*>(read_key.data())) != 0) {
    return absl::UnknownError("failed to decrypt message");
  }

  assert(plaintext_len == ACKING_BYTES);

  uint32_t ack_id = reinterpret_cast<uint32_t*>(plaintext.data())[0];

  return ack_id;
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// |||                       Async Friending Methods |||
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

// encrypt an asynchronous friend request
// The async friend request from A->B is Enc(ID_B, ID_A || msg) =
// = Enc(f_sk_A, f_pk_B, ID_A || msg)
// Note: there's a lot of redundant information here
// TODO: optimize message length
auto encrypt_async_invitation(const string& self_id,
                              const string& self_invitation_private_key,
                              const string& friend_invitation_public_key,
                              const string& message)
    -> asphr::StatusOr<string> {
  // check integrity of the keys
  if (friend_invitation_public_key.size() != crypto_box_PUBLICKEYBYTES) {
    return asphr::InvalidArgumentError(
        "friend_public_key is not the correct size");
  }
  if (self_invitation_private_key.size() != crypto_box_SECRETKEYBYTES) {
    return asphr::InvalidArgumentError(
        "self_private_key is not the correct size");
  }
  // ensure that the message is not too long
  if (message.size() > INVITATION_MESSAGE_MAX_PLAINTEXT_SIZE) {
    return asphr::InvalidArgumentError(
        "message is too long, must be less than " +
        std::to_string(INVITATION_MESSAGE_MAX_PLAINTEXT_SIZE));
  }
  asphrclient::AsyncInvitation async_invitation;
  async_invitation.set_my_public_id(self_id);
  async_invitation.set_message(message);
  std::string async_invitation_str;
  if (!async_invitation.SerializeToString(&async_invitation_str)) {
    return absl::UnknownError("failed to serialize async invitation");
  }

  // We now encrypt the message, using a clone of the code above
  string plaintext = async_invitation_str;
  size_t unpadded_plaintext_size = plaintext.size();

  // Here we need to pad the plaintext to the max length
  // to prevent length attack
  // since the public key crypto protocol asserts that
  // "ciphertextlength = plaintextlength + const"
  if (unpadded_plaintext_size > ASYNC_INVITATION_SIZE) {
    return asphr::InvalidArgumentError(
        "message is too long, must be less than " +
        std::to_string(ASYNC_INVITATION_SIZE));
  }

  plaintext.resize(ASYNC_INVITATION_SIZE);

  // use libsodium to pad the plaintext
  size_t plaintext_size;
  if (sodium_pad(&plaintext_size,
                 reinterpret_cast<unsigned char*>(plaintext.data()),
                 unpadded_plaintext_size, ASYNC_INVITATION_SIZE,
                 ASYNC_INVITATION_SIZE) != 0) {
    return absl::UnknownError("failed to pad message");
  }

  // encrypt it!
  std::string ciphertext;
  unsigned long long ciphertext_size = plaintext_size + crypto_box_MACBYTES;
  ciphertext.resize(ciphertext_size + crypto_box_NONCEBYTES);

  unsigned char nonce[crypto_box_NONCEBYTES];
  randombytes_buf(nonce, sizeof nonce);

  if (crypto_box_easy(reinterpret_cast<unsigned char*>(ciphertext.data()),
                      reinterpret_cast<const unsigned char*>(plaintext.data()),
                      plaintext.size(), nonce,
                      reinterpret_cast<const unsigned char*>(
                          friend_invitation_public_key.data()),
                      reinterpret_cast<const unsigned char*>(
                          self_invitation_private_key.data())) != 0) {
    return absl::UnknownError("failed to encrypt message");
  }
  // append the nounce to the end of the ciphertext

  for (size_t i = 0; i < crypto_box_NONCEBYTES; i++) {
    ciphertext[i + ciphertext_size] += nonce[i];
  }

  // Sanity check the length of everything
  assert(ciphertext.size() == ciphertext_size + crypto_box_NONCEBYTES);
  assert(ciphertext_size == plaintext_size + crypto_box_MACBYTES);
  assert(plaintext.size() == plaintext_size);
  assert(plaintext_size == ASYNC_INVITATION_SIZE);

  return ciphertext;
}

// decrypt the asynchronous friend requests

auto decrypt_async_invitation(const string& self_invitation_private_key,
                              const string& friend_invitation_public_key,
                              const string& ciphertext)
    -> asphr::StatusOr<pair<string, string>> {
  // We now decrypt the message, using a clone of the code above
  if (friend_invitation_public_key.size() != crypto_box_PUBLICKEYBYTES) {
    return asphr::InvalidArgumentError(
        "friend_public_key is not the correct size");
  }
  if (self_invitation_private_key.size() != crypto_box_SECRETKEYBYTES) {
    return asphr::InvalidArgumentError(
        "self_private_key is not the correct size");
  }
  auto ciphertext_len = ciphertext.size() - crypto_box_NONCEBYTES;
  auto padded_plaintext_len = ciphertext_len - crypto_box_MACBYTES;

  // make sure the plaintext is of the correct length
  if (padded_plaintext_len != ASYNC_INVITATION_SIZE) {
    return asphr::InvalidArgumentError("ciphertext is not the correct size");
  }

  // Extract the nounce from the end of the ciphertext
  unsigned char nonce[crypto_box_NONCEBYTES];
  for (size_t i = 0; i < crypto_box_NONCEBYTES; i++) {
    nonce[i] = ciphertext[i + ciphertext_len];
  }
  // convert the ciphertext to the string form
  string ciphertext_str = "";
  for (size_t i = 0; i < ciphertext_len; i++) {
    ciphertext_str += ciphertext.at(i);
  }
  // decrypt it!!
  std::string plaintext;
  plaintext.resize(padded_plaintext_len);
  if (crypto_box_open_easy(
          reinterpret_cast<unsigned char*>(plaintext.data()),
          reinterpret_cast<const unsigned char*>(ciphertext_str.data()),
          ciphertext_str.size(), nonce,
          reinterpret_cast<const unsigned char*>(
              friend_invitation_public_key.data()),
          reinterpret_cast<const unsigned char*>(
              self_invitation_private_key.data())) != 0) {
    return absl::UnknownError("failed to decrypt friend request");
  }
  assert(padded_plaintext_len == plaintext.size());

  // remove padding
  size_t unpadded_plaintext_len = 0;
  plaintext.resize(padded_plaintext_len);
  if (sodium_unpad(&unpadded_plaintext_len,
                   reinterpret_cast<unsigned char*>(plaintext.data()),
                   padded_plaintext_len, ASYNC_INVITATION_SIZE) != 0) {
    return absl::UnknownError("failed to unpad message");
  }

  plaintext.resize(unpadded_plaintext_len);

  // deconstruct protobuf
  asphrclient::AsyncInvitation async_invitation;
  if (!async_invitation.ParseFromString(plaintext)) {
    return absl::UnknownError("failed to deserialize async invitation");
  }

  // TODO: specifically, we need to verify that the public_id in the body
  // corresponds to the public_id that the message was authenticated with
  // otherwise, someone might impersonate the real receiver
  return make_pair(async_invitation.my_public_id(), async_invitation.message());
}
}  // namespace crypto