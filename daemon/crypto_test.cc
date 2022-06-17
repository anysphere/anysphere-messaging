//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "crypto.hpp"

#include <gtest/gtest.h>

#include "client_lib/client_lib.hpp"

TEST(CryptoTest, EncryptDecrypt) {
  Crypto crypto;

  auto [pk1, sk1] = crypto.generate_keypair();
  auto [pk2, sk2] = crypto.generate_keypair();

  auto [r1, w1] = crypto.derive_read_write_keys(pk1, sk1, pk2);
  auto [r2, w2] = crypto.derive_read_write_keys(pk2, sk2, pk1);

  auto friend2_from_perspective_of_friend1 = Friend("friend2", {}, "add2");
  friend2_from_perspective_of_friend1.write_key = w1;
  friend2_from_perspective_of_friend1.read_key = r1;
  auto friend1_from_perspective_of_friend2 = Friend("friend1", {}, "add1");
  friend1_from_perspective_of_friend2.write_key = w2;
  friend1_from_perspective_of_friend2.read_key = r2;

  ASSERT_EQ(r2, w1);
  ASSERT_EQ(w2, r1);

  std::string plaintext = "hello world";
  asphrclient::Message message;
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

  auto friend2_from_perspective_of_friend1 = Friend("friend2", {}, "add2");
  friend2_from_perspective_of_friend1.write_key = w1;
  friend2_from_perspective_of_friend1.read_key = r1;
  auto friend1_from_perspective_of_friend2 = Friend("friend1", {}, "add1");
  friend1_from_perspective_of_friend2.write_key = w2;
  friend1_from_perspective_of_friend2.read_key = r2;

  ASSERT_EQ(r2, w1);
  ASSERT_EQ(w2, r1);

  std::string plaintext;
  for (size_t i = 0; i < GUARANTEED_SINGLE_MESSAGE_SIZE; i++) {
    plaintext += 'a';
  }
  asphrclient::Message message;
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

  auto friend2_from_perspective_of_friend1 = Friend("friend2", {}, "add2");
  friend2_from_perspective_of_friend1.write_key = w1;
  friend2_from_perspective_of_friend1.read_key = r1;
  auto friend1_from_perspective_of_friend2 = Friend("friend1", {}, "add1");
  friend1_from_perspective_of_friend2.write_key = w2;
  friend1_from_perspective_of_friend2.read_key = r2;

  ASSERT_EQ(r2, w1);
  ASSERT_EQ(w2, r1);

  std::string plaintext;
  for (size_t i = 0; i < GUARANTEED_SINGLE_MESSAGE_SIZE + 1; i++) {
    plaintext += 'a';
  }
  asphrclient::Message message;
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

TEST(CryptoTest, EncryptDecryptAcks) {
  Crypto crypto;

  auto [pk1, sk1] = crypto.generate_keypair();
  auto [pk2, sk2] = crypto.generate_keypair();

  auto [r1, w1] = crypto.derive_read_write_keys(pk1, sk1, pk2);
  auto [r2, w2] = crypto.derive_read_write_keys(pk2, sk2, pk1);

  auto friend2_from_perspective_of_friend1 = Friend("friend2", {}, "add2");
  friend2_from_perspective_of_friend1.write_key = w1;
  friend2_from_perspective_of_friend1.read_key = r1;
  auto friend1_from_perspective_of_friend2 = Friend("friend1", {}, "add1");
  friend1_from_perspective_of_friend2.write_key = w2;
  friend1_from_perspective_of_friend2.read_key = r2;

  ASSERT_EQ(r2, w1);
  ASSERT_EQ(w2, r1);

  vector<uint32_t> acks = {0, 1, 10, 0xFFFFFFFF};

  for (auto ack : acks) {
    auto ciphertext =
        crypto.encrypt_ack(ack, friend2_from_perspective_of_friend1);
    EXPECT_TRUE(ciphertext.ok());

    auto decrypted = crypto.decrypt_ack(ciphertext.value(),
                                        friend1_from_perspective_of_friend2);
    EXPECT_TRUE(decrypted.ok());

    EXPECT_EQ(decrypted.value(), ack);
  }
}

TEST(CryptoTest, EncryptDecryptAsyncFriendRequest) {
  Crypto crypto;
  for (auto round = 0; round < 10; round++) {
    auto [pk1, sk1] = crypto.generate_keypair();
    auto [f_pk1, f_sk1] = crypto.generate_friend_request_keypair();
    auto [f_pk2, f_sk2] = crypto.generate_friend_request_keypair();

    int allocation = rand() % 10000;
    RegistrationInfo reg_info1 = {.name = "friend_test_1",
                                  .public_key = pk1,
                                  .private_key = sk1,
                                  .friend_request_public_key = f_pk1,
                                  .friend_request_private_key = f_sk1,
                                  .allocation = std::vector({allocation})};

    RegistrationInfo reg_info2 = {.name = "friend_test_2",
                                  .public_key = pk1,
                                  .private_key = sk1,
                                  .friend_request_public_key = f_pk2,
                                  .friend_request_private_key = f_sk2};
    // user 1 sends friend request to user 2
    auto friend_request_ =
        crypto.encrypt_async_friend_request(reg_info1, f_pk2);
    if (!friend_request_.ok()) {
      FAIL() << "Failed to encrypt friend request";
    }
    auto friend_request = friend_request_.value();
    // user 2 decrypts friend request
    auto decrypted_friend_request_ =
        crypto.decrypt_async_friend_request(reg_info2, f_pk1, friend_request);
    if (!decrypted_friend_request_.ok()) {
      FAIL() << "Failed to decrypt friend request";
    }
    auto decrypted_friend_request = decrypted_friend_request_.value();
    EXPECT_EQ(std::get<0>(decrypted_friend_request), reg_info1.name);
    EXPECT_EQ(std::get<1>(decrypted_friend_request), allocation);
    EXPECT_EQ(std::get<2>(decrypted_friend_request), reg_info1.public_key);
  }
}