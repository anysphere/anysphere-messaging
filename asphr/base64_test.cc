
#include "base64.hpp"

#include <gtest/gtest.h>

TEST(Base64, EmptyString) {
  auto out = Base64::Encode("");
  EXPECT_EQ(out, "");
}

TEST(Base64, EncodeArbitrary) {
  auto out = Base64::Encode("hi my name is arvid");
  EXPECT_EQ(out, "aGkgbXkgbmFtZSBpcyBhcnZpZA==");
}

TEST(Base64, DecodeArbitrary) {
  std::string out;
  Base64::Decode("aGkgbXkgbmFtZSBpcyBhcnZpZA==", out);
  EXPECT_EQ(out, "hi my name is arvid");
}

TEST(Base64, EncodeDecode) {
  auto out = Base64::Encode("こんにちは");
  std::string in;
  Base64::Decode(out, in);
  EXPECT_EQ(in, "こんにちは");
}