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
                          "key\x90\xA1",
                          "y\x90",
                          "y",
                          "\x90",
                          "b\x90",
                          "0",
                          string(10, '\0'),
                          string(1, '\0'),
                          string(1, '\1') + "1" + string(1, '\0'),
                          string(1, '\0') + "1" + string(100, '\1'),
                          ""};

auto bytes_as_numbers(string_view bytes) -> vector<int> {
  vector<int> numbers;
  for (auto c : bytes) {
    numbers.push_back((uint8_t)c);
  }
  return numbers;
}
auto vector_as_string(vector<int> numbers) -> string {
  string s;
  for (auto n : numbers) {
    s = absl::StrCat(s, ",", n);
  }
  return s;
}

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
    EXPECT_EQ(s, r.value())
        << "base68 real: " << b
        << " bytes: " << vector_as_string(bytes_as_numbers(s))
        << " base58: " << vector_as_string(bytes_as_numbers(b))
        << " decoded: " << vector_as_string(bytes_as_numbers(r.value()));
  }
}
