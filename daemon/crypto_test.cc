#include "crypto.hpp"

#include <gtest/gtest.h>

#include "client/client_lib/client_lib.h"

TEST(CryptoTest, EncryptDecrypt) {
  Crypto crypto;

  auto [pk1, sk1] = crypto.generate_keypair();
  auto [pk2, sk2] = crypto.generate_keypair();

  auto [r1, w1] = crypto.derive_read_write_keys(pk1, sk1, pk2);
  auto [r2, w2] = crypto.derive_read_write_keys(pk2, sk2, pk1);

  Friend friend2_from_perspective_of_friend1;
  friend2_from_perspective_of_friend1.write_key = w1;
  friend2_from_perspective_of_friend1.read_key = r1;
  Friend friend1_from_perspective_of_friend2;
  friend1_from_perspective_of_friend2.write_key = w2;
  friend1_from_perspective_of_friend2.read_key = r2;

  ASSERT_EQ(r2, w1);
  ASSERT_EQ(w2, r1);

  std::string plaintext = "hello world";
  Message message;
  message.set_msg(plaintext);

  auto ciphertext =
      crypto.encrypt_send(message, friend2_from_perspective_of_friend1);
  EXPECT_TRUE(ciphertext.ok());

  auto decrypted = crypto.decrypt_receive(ciphertext.value(),
                                          friend1_from_perspective_of_friend2);
  EXPECT_TRUE(decrypted.ok());

  EXPECT_EQ(decrypted->msg(), message.msg());
  EXPECT_EQ(decrypted->id(), message.id());
  EXPECT_EQ(decrypted->acks_size(), message.acks_size());
}

TEST(CryptoTest, EncryptDecryptMaxSize) {
  Crypto crypto;

  auto [pk1, sk1] = crypto.generate_keypair();
  auto [pk2, sk2] = crypto.generate_keypair();

  auto [r1, w1] = crypto.derive_read_write_keys(pk1, sk1, pk2);
  auto [r2, w2] = crypto.derive_read_write_keys(pk2, sk2, pk1);

  Friend friend2_from_perspective_of_friend1;
  friend2_from_perspective_of_friend1.write_key = w1;
  friend2_from_perspective_of_friend1.read_key = r1;
  Friend friend1_from_perspective_of_friend2;
  friend1_from_perspective_of_friend2.write_key = w2;
  friend1_from_perspective_of_friend2.read_key = r2;

  ASSERT_EQ(r2, w1);
  ASSERT_EQ(w2, r1);

  std::string plaintext;
  for (size_t i = 0; i < GUARANTEED_SINGLE_MESSAGE_SIZE; i++) {
    plaintext += 'a';
  }
  Message message;
  message.set_msg(plaintext);

  auto ciphertext =
      crypto.encrypt_send(message, friend2_from_perspective_of_friend1);
  EXPECT_TRUE(ciphertext.ok());

  auto decrypted = crypto.decrypt_receive(ciphertext.value(),
                                          friend1_from_perspective_of_friend2);
  EXPECT_TRUE(decrypted.ok());

  EXPECT_EQ(decrypted->msg(), message.msg());
  EXPECT_EQ(decrypted->id(), message.id());
  EXPECT_EQ(decrypted->acks_size(), message.acks_size());
}

TEST(CryptoTest, EncryptDecryptBiggerThanMaxSize) {
  Crypto crypto;

  auto [pk1, sk1] = crypto.generate_keypair();
  auto [pk2, sk2] = crypto.generate_keypair();

  auto [r1, w1] = crypto.derive_read_write_keys(pk1, sk1, pk2);
  auto [r2, w2] = crypto.derive_read_write_keys(pk2, sk2, pk1);

  Friend friend2_from_perspective_of_friend1;
  friend2_from_perspective_of_friend1.write_key = w1;
  friend2_from_perspective_of_friend1.read_key = r1;
  Friend friend1_from_perspective_of_friend2;
  friend1_from_perspective_of_friend2.write_key = w2;
  friend1_from_perspective_of_friend2.read_key = r2;

  ASSERT_EQ(r2, w1);
  ASSERT_EQ(w2, r1);

  std::string plaintext;
  for (size_t i = 0; i < GUARANTEED_SINGLE_MESSAGE_SIZE + 1; i++) {
    plaintext += 'a';
  }
  Message message;
  message.set_msg(plaintext);

  auto ciphertext =
      crypto.encrypt_send(message, friend2_from_perspective_of_friend1);
  EXPECT_TRUE(ciphertext.ok());

  auto decrypted = crypto.decrypt_receive(ciphertext.value(),
                                          friend1_from_perspective_of_friend2);
  EXPECT_TRUE(decrypted.ok());

  // the decrypted message should be a truncated version of the real message
  EXPECT_NE(decrypted->msg(), message.msg());
}