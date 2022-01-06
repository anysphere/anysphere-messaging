#include "crypto.h"

#include <gtest/gtest.h>

TEST(CryptoTest, EncryptDecrypt) {
  Crypto crypto;
  std::string key = "0123456789abcdef0123456789abcdef";
  std::string plaintext = "hello world";
  std::string ciphertext = crypto.encrypt(plaintext, key);
  std::string decrypted = crypto.decrypt(ciphertext, key);
  EXPECT_EQ(plaintext, decrypted);
}