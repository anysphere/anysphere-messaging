//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "crypto.hpp"

#include <gtest/gtest.h>

#include "daemon/identifier/identifier.hpp"

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

TEST(CryptoTest, EncryptDecryptAsyncFriendRequest) {
  for (auto round = 0; round < 10; round++) {
    string user1 = "user1_" + to_string(round);
    string user2 = "user2_" + to_string(round);
    auto [kx_pk1, kx_sk1] = crypto::generate_kx_keypair();
    auto [kx_pk2, kx_sk2] = crypto::generate_kx_keypair();
    auto [f_pk1, f_sk1] = crypto::generate_invitation_keypair();
    auto [f_pk2, f_sk2] = crypto::generate_invitation_keypair();

    cout << "Keypair generated" << endl;

    int allocation1 = rand() % 10000;
    // Create public ids
    auto id1 = PublicIdentifier(allocation1, kx_pk1, f_pk1).to_public_id();

    cout << "Id generated: " << id1 << endl;

    string message = "hello from 1 to 2 on round " + to_string(round);
    // user 1 sends friend request to user 2
    // note: This API have been changed in arvid's update.
    auto invitation_ =
        crypto::encrypt_async_invitation(id1, f_sk1, f_pk2, message);
    if (!invitation_.ok()) {
      FAIL() << "Failed to encrypt friend request";
    }
    auto invitation = invitation_.value();

    cout << "Friend request encrypted" << endl;
    // user 2 decrypts friend request
    // Note: This API have been changed in arvid's update.
    auto decrypted_invitation_ =
        crypto::decrypt_async_invitation(f_sk2, f_pk1, invitation);
    if (!decrypted_invitation_.ok()) {
      cout << decrypted_invitation_.status() << endl;
      FAIL() << "Failed to decrypt friend request";
    }
    auto decrypted_invitation = decrypted_invitation_.value();
    EXPECT_EQ(decrypted_invitation.first, id1);
    EXPECT_EQ(decrypted_invitation.second, message);
    // user 2 then decrypts the id to get the user1's kx key.
    auto identifier_ =
        PublicIdentifier::from_public_id(decrypted_invitation.first);
    assert(identifier_.ok());
    auto identifier = identifier_.value();
    assert(identifier.index == allocation1);
    assert(identifier.kx_public_key == kx_pk1);
    assert(identifier.invitation_public_key == f_pk1);
  }
}