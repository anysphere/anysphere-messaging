//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "crypto.hpp"

#include <gtest/gtest.h>

TEST(CryptoTest, EncryptDecrypt) {
  crypto::init();

  auto [pk1, sk1] = crypto::generate_keypair();
  auto [pk2, sk2] = crypto::generate_keypair();

  auto [r1, w1] = crypto::derive_read_write_keys(pk1, sk1, pk2);
  auto [r2, w2] = crypto::derive_read_write_keys(pk2, sk2, pk1);

  ASSERT_EQ(r2, w1);
  ASSERT_EQ(w2, r1);

  std::string plaintext = "hello world";
  asphrclient::Message message;
  message.set_msg(plaintext);

  auto ciphertext = crypto::encrypt_send(message, w1);
  EXPECT_TRUE(ciphertext.ok());

  auto decrypted = crypto::decrypt_receive(ciphertext.value(), r2);
  EXPECT_TRUE(decrypted.ok());

  EXPECT_EQ(decrypted->msg(), message.msg());
  EXPECT_EQ(decrypted->sequence_number(), message.sequence_number());
}

TEST(CryptoTest, EncryptDecryptMaxSize) {
  crypto::init();

  auto [pk1, sk1] = crypto::generate_keypair();
  auto [pk2, sk2] = crypto::generate_keypair();

  auto [r1, w1] = crypto::derive_read_write_keys(pk1, sk1, pk2);
  auto [r2, w2] = crypto::derive_read_write_keys(pk2, sk2, pk1);

  ASSERT_EQ(r2, w1);
  ASSERT_EQ(w2, r1);

  std::string plaintext;
  for (size_t i = 0; i < GUARANTEED_SINGLE_MESSAGE_SIZE; i++) {
    plaintext += 'a';
  }
  asphrclient::Message message;
  message.set_msg(plaintext);

  auto ciphertext = crypto::encrypt_send(message, w1);
  EXPECT_TRUE(ciphertext.ok());

  auto decrypted = crypto::decrypt_receive(ciphertext.value(), r2);
  EXPECT_TRUE(decrypted.ok());

  EXPECT_EQ(decrypted->msg(), message.msg());
  EXPECT_EQ(decrypted->sequence_number(), message.sequence_number());
}

TEST(CryptoTest, EncryptDecryptBiggerThanMaxSize) {
  crypto::init();

  auto [pk1, sk1] = crypto::generate_keypair();
  auto [pk2, sk2] = crypto::generate_keypair();

  auto [r1, w1] = crypto::derive_read_write_keys(pk1, sk1, pk2);
  auto [r2, w2] = crypto::derive_read_write_keys(pk2, sk2, pk1);

  ASSERT_EQ(r2, w1);
  ASSERT_EQ(w2, r1);

  std::string plaintext;
  for (size_t i = 0; i < GUARANTEED_SINGLE_MESSAGE_SIZE + 1; i++) {
    plaintext += 'a';
  }
  asphrclient::Message message;
  message.set_msg(plaintext);

  auto ciphertext = crypto::encrypt_send(message, w1);
  EXPECT_TRUE(ciphertext.ok());

  auto decrypted = crypto::decrypt_receive(ciphertext.value(), r2);
  EXPECT_TRUE(decrypted.ok());

  // the decrypted message should be a truncated version of the real message
  EXPECT_NE(decrypted->msg(), message.msg());
}

TEST(CryptoTest, EncryptDecryptAcks) {
  crypto::init();

  auto [pk1, sk1] = crypto::generate_keypair();
  auto [pk2, sk2] = crypto::generate_keypair();

  auto [r1, w1] = crypto::derive_read_write_keys(pk1, sk1, pk2);
  auto [r2, w2] = crypto::derive_read_write_keys(pk2, sk2, pk1);

  ASSERT_EQ(r2, w1);
  ASSERT_EQ(w2, r1);

  vector<uint32_t> acks = {0, 1, 10, 0xFFFFFFFF};

  for (auto ack : acks) {
    auto ciphertext = crypto::encrypt_ack(ack, w1);
    EXPECT_TRUE(ciphertext.ok());

    auto decrypted = crypto::decrypt_ack(ciphertext.value(), r2);
    EXPECT_TRUE(decrypted.ok());

    EXPECT_EQ(decrypted.value(), ack);
  }
}