//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "base58.hpp"

#include <gtest/gtest.h>

const vector<string> v = {"helloworld",
                          "00hi0",
                          "11111111",
                          "1",
                          "0",
                          string(10, '\0'),
                          string(1, '\0'),
                          string(1, '\1') + "1" + string(1, '\0'),
                          string(1, '\0') + "1" + string(100, '\1'),
                          ""};

TEST(Base58Test, Simple) {
  for (auto s : v) {
    auto b = base58::Encode(s);
    for (auto c : b) {
      ASSERT_NE(
          string("123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz")
              .find(c),
          string::npos);
    }
    auto r = base58::Decode(b);
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(s, r.value());
  }
}
