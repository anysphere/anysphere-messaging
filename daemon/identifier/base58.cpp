//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

// From https://github.com/bitcoin/bitcoin/blob/master/src/base58.h
// Modified to not use Spans
// See COPYING.bitcoin for license.

// Copyright (c) 2014-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.hpp"

namespace base58 {

/** All alphanumeric characters except for "0", "I", "O", and "l" */
constexpr string_view pszBase58 =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
constexpr int8_t mapBase58[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,
    8,  -1, -1, -1, -1, -1, -1, -1, 9,  10, 11, 12, 13, 14, 15, 16, -1, 17, 18,
    19, 20, 21, -1, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1,
    -1, -1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1,
};
static_assert(std::size(mapBase58) == 256,
              "mapBase58.size() should be 256");  // guarantee not out of range
static_assert(pszBase58.size() == 58, "pszBase58.size() should be 58");

auto Encode(std::string_view input) -> string {
  // Skip & count leading zeroes.
  int zeroes = 0;
  int length = 0;
  while (input.size() > 0 && input.at(0) == 0) {
    input = input.substr(1);
    zeroes++;
  }
  // Allocate enough space in big-endian base58 representation.
  int size = input.size() * 138 / 100 + 1;  // log(256) / log(58), rounded up.
  std::vector<unsigned char> b58(size);
  // Process the bytes.
  while (input.size() > 0) {
    int carry = input.at(0);
    int i = 0;
    // Apply "b58 = b58 * 256 + ch".
    for (std::vector<unsigned char>::reverse_iterator it = b58.rbegin();
         (carry != 0 || i < length) && (it != b58.rend()); it++, i++) {
      carry += 256 * (*it);
      *it = carry % 58;
      carry /= 58;
    }

    assert(carry == 0);
    length = i;
    input = input.substr(1);
  }
  // Skip leading zeroes in base58 result.
  std::vector<unsigned char>::iterator it = b58.begin() + (size - length);
  while (it != b58.end() && *it == 0) it++;
  // Translate the result into a string.
  std::string str;
  str.reserve(zeroes + (b58.end() - it));
  str.assign(zeroes, '1');
  while (it != b58.end()) str += pszBase58.at(*(it++));
  return str;
}

auto Decode(std::string_view str) -> asphr::StatusOr<string> {
  // Skip and count leading '1's.
  int zeroes = 0;
  int length = 0;
  while (str.size() > 0 && str.at(0) == '1') {
    zeroes++;
    str = str.substr(1);
  }
  // Allocate enough space in big-endian base256 representation.
  int size = str.size() * 733 / 1000 + 1;  // log(58) / log(256), rounded up.
  std::vector<unsigned char> b256(size);
  // Process the characters.
  while (str.size() > 0) {
    // Decode base58 character
    int carry = mapBase58[(uint8_t)str.at(0)];
    if (carry == -1)  // Invalid b58 character
      return absl::InvalidArgumentError("Invalid b58 character");
    int i = 0;
    for (std::vector<unsigned char>::reverse_iterator it = b256.rbegin();
         (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i) {
      carry += 58 * (*it);
      *it = carry % 256;
      carry /= 256;
    }
    assert(carry == 0);
    length = i;
    str = str.substr(1);
  }
  // Skip leading zeroes in b256.
  std::vector<unsigned char>::iterator it = b256.begin() + (size - length);
  // Copy result into output vector.
  string out;
  out.reserve(zeroes + (b256.end() - it));
  out.assign(zeroes, '\0');
  while (it != b256.end()) out.push_back(*(it++));
  return out;
}
}  // namespace base58