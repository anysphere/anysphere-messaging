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

// copied from bitcoin!
// https://github.com/bitcoin/bitcoin/blob/master/src/test/data/base58_encode_decode.json
// and
// https://github.com/bitcoin/bitcoin/blob/2598720d6c1ef15288045599de7e65f8707d09bc/src/test/base58_tests.cpp#L24
const vector<pair<string, string>> bitcoin_base58_pairs = {
    {"", ""},
    {"61", "2g"},
    {"626262", "a3gV"},
    {"636363", "aPEr"},
    {"73696d706c792061206c6f6e6720737472696e67",
     "2cFupjhnEsSn59qHXstmK2ffpLv2"},
    {"00eb15231dfceb60925886b67d065299925915aeb172c06647",
     "1NS17iag9jJgTHD1VXjvLCEnZuQ3rJDE9L"},
    {"516b6fcd0f", "ABnLTmg"},
    {"bf4f89001e670274dd", "3SEo3LWLoPntC"},
    {"572e4794", "3EFU7m"},
    {"ecac89cad93923c02321", "EJDM8drfXA6uyA"},
    {"10c8511e", "Rt5zm"},
    {"00000000000000000000", "1111111111"},
    {"000111d38e5fc9071ffcd20b4a763cc9ae4f252bb4e48fd66a835e252ada93ff480d6dd4"
     "3dc62a641155a5",
     "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"},
    {"000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20212223"
     "2425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f4041424344454647"
     "48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768696a6b"
     "6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f"
     "909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3"
     "b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7"
     "d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafb"
     "fcfdfeff",
     "1cWB5HCBdLjAuqGGReWE3R3CguuwSjw6RHn39s2yuDRTS5NsBgNiFpWgAnEx6VQi8csexkgY"
     "w3mdYrMHr8x9i7aEwP8kZ7vccXWqKDvGv3u1GxFKPuAkn8JCPPGDMf3vMMnbzm6Nh9zh1gcN"
     "sMvH3ZNLmP5fSG6DGbbi2tuwMWPthr4boWwCxf7ewSgNQeacyozhKDDQQ1qL5fQFUW52QKUZ"
     "DZ5fw3KXNQJMcNTcaB723LchjeKun7MuGW5qyCBZYzA1KjofN1gYBV3NqyhQJ3Ns746GNuf9"
     "N2pQPmHz4xpnSrrfCvy6TVVz5d4PdrjeshsWQwpZsZGzvbdAdN8MKV5QsBDY"}};

TEST(Base58Test, BitcoinEncode) {
  for (auto p : bitcoin_base58_pairs) {
    vector<absl::uint128> raw_bytes(p.first.size() / 2);
    for (int i = 0; i < std::ssize(p.first) / 2; i++) {
      bool success =
          absl::SimpleHexAtoi(p.first.substr(2 * i, 2), &raw_bytes.at(i));
      EXPECT_TRUE(success) << "Failed to parse hex: "
                           << p.first.substr(2 * i, 2);
    }
    // convert raw_bytes to string
    string raw_bytes_string;
    for (auto b : raw_bytes) {
      raw_bytes_string += static_cast<unsigned char>(b);
    }
    auto b = base58::Encode(raw_bytes_string);
    EXPECT_EQ(p.second, b);
  }
}