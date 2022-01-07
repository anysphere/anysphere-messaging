#include "crypto.h"

#include <gtest/gtest.h>

TEST(CryptoTest, EncryptDecrypt) {
  Crypto crypto;

  auto [pk1, sk1] = crypto.gen_keypair();
  auto [pk2, sk2] = crypto.gen_keypair();

  auto [r1, w1] = crypto.derive_read_write_keys(pk1, sk1, pk2);
  auto [r2, w2] = crypto.derive_read_write_keys(pk2, sk2, pk1);

  Friend friend2_from_perspective_of_friend1;
  friend2_from_perspective_of_friend1.write_key = w1;
  friend2_from_perspective_of_friend1.read_key = r1;
  Friend friend1_from_perspective_of_friend2;
  friend1_from_perspective_of_friend2.write_key = w2;
  friend1_from_perspective_of_friend2.read_key = r2;

  std::string plaintext = "hello world";
  Message message;
  message.set_msg(plaintext);

  auto [ciphertext, valid1] =
      crypto.encrypt_send(message, friend2_from_perspective_of_friend1);
  EXPECT_EQ(valid1, 0);

  auto [decrypted, valid2] =
      crypto.decrypt_receive(ciphertext, friend1_from_perspective_of_friend2);
  EXPECT_EQ(valid2, 0);

  EXPECT_EQ(decrypted.msg(), message.msg());
  EXPECT_EQ(decrypted.id(), message.id());
  EXPECT_EQ(decrypted.acks_size(), message.acks_size());
}