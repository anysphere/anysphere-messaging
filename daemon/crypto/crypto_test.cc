//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "crypto.hpp"

#include <gtest/gtest.h>

TEST(CryptoTest, EncryptDecrypt) {
  crypto::init();

  auto [pk1, sk1] = crypto::generate_kx_keypair();
  auto [pk2, sk2] = crypto::generate_kx_keypair();

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

  auto [pk1, sk1] = crypto::generate_kx_keypair();
  auto [pk2, sk2] = crypto::generate_kx_keypair();

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

  auto [pk1, sk1] = crypto::generate_kx_keypair();
  auto [pk2, sk2] = crypto::generate_kx_keypair();

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

  auto [pk1, sk1] = crypto::generate_kx_keypair();
  auto [pk2, sk2] = crypto::generate_kx_keypair();

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

TEST(CryptoTest, EncodeDecodeId) {
  for (auto round = 0; round < 10; round++) {
    string user = "user_asdf_" + to_string(round);
    auto [kx_pk, kx_sk] = crypto::generate_kx_keypair();
    auto [f_pk, f_sk] = crypto::generate_friend_request_keypair();
    int allocation = rand() % 10000;
    auto id_ = crypto::generate_user_id(user, allocation, kx_pk, f_pk);
    EXPECT_TRUE(id_.ok());
    auto dec = crypto::decode_user_id(id_.value());
    EXPECT_TRUE(dec.ok());
    auto [user_, allocation_, kx_pk_, f_pk_] = dec.value();
    EXPECT_EQ(user, user_);
    EXPECT_EQ(allocation, allocation_);
    EXPECT_EQ(kx_pk, kx_pk_);
    EXPECT_EQ(f_pk, f_pk_);
  }
}

TEST(CryptoTest, EncryptDecryptAsyncFriendRequest) {
  for (auto round = 0; round < 10; round++) {
    string user1 = "user1_" + to_string(round);
    string user2 = "user2_" + to_string(round);
    auto [kx_pk1, kx_sk1] = crypto::generate_kx_keypair();
    auto [kx_pk2, kx_sk2] = crypto::generate_kx_keypair();
    auto [f_pk1, f_sk1] = crypto::generate_friend_request_keypair();
    auto [f_pk2, f_sk2] = crypto::generate_friend_request_keypair();

    cout << "Keypair generated" << endl;

    int allocation1 = rand() % 10000;
    int allocation2 = rand() % 10000;
    auto id1_ = crypto::generate_user_id(user1, allocation1, kx_pk1, f_pk1);
    auto id2_ = crypto::generate_user_id(user2, allocation2, kx_pk2, f_pk2);
    if (!id1_.ok() || !id2_.ok()) {
      FAIL() << "Failed to generate user IDs";
    }
    auto id1 = id1_.value();
    auto id2 = id2_.value();

    cout << "Id generated: " << id1 << endl;

    string message = "hello from 1 to 2 on round " + to_string(round);
    // user 1 sends friend request to user 2
    auto friend_request_ =
        crypto::encrypt_async_friend_request(id1, f_sk1, id2, message);
    if (!friend_request_.ok()) {
      FAIL() << "Failed to encrypt friend request";
    }
    auto friend_request = friend_request_.value();

    cout << "Friend request encrypted" << endl;
    // user 2 decrypts friend request
    auto decrypted_friend_request_ =
        crypto::decrypt_async_friend_request(id2, f_sk2, id1, friend_request);
    if (!decrypted_friend_request_.ok()) {
      cout << decrypted_friend_request_.status() << endl;
      FAIL() << "Failed to decrypt friend request";
    }
    auto decrypted_friend_request = decrypted_friend_request_.value();
    EXPECT_EQ(decrypted_friend_request.first, id1);
    EXPECT_EQ(decrypted_friend_request.second, message);
  }
}