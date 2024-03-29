//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "identifier.hpp"

#include "asphr/schema/identifier.pb.h"
#include "base58.hpp"
#include "client/daemon/crypto/crypto.hpp"
#include "wordlist.hpp"

// append_checksum adds a checksum to the end of the string
// it is 2 bytes long, catching typos with probability >=99.99%
auto append_checksum(string data) -> string {
  constexpr int len = 2;
  auto hash = crypto::generic_hash(data);
  return string(data) + hash.substr(0, len);
}
// remove_checksum checks the checksum and removes it if all goes well
auto remove_checksum(string bytes) -> asphr::StatusOr<string> {
  constexpr int len = 2;
  if (bytes.size() < len) {
    return absl::InvalidArgumentError("bytes too short to contain checksum");
  }
  if (crypto::generic_hash(bytes.substr(0, bytes.size() - len))
          .substr(0, len) != bytes.substr(bytes.size() - len)) {
    return absl::InvalidArgumentError("checksum does not match");
  }
  return string(bytes.substr(0, bytes.size() - len));
}

PublicIdentifier::PublicIdentifier(int index, string kx_public_key,
                                   string invitation_public_key)
    : index(index),
      kx_public_key(kx_public_key),
      invitation_public_key(invitation_public_key) {}

auto PublicIdentifier::from_public_id(string public_id)
    -> asphr::StatusOr<PublicIdentifier> {
  auto raw_bytes = base58::Decode(public_id);
  if (!raw_bytes.ok()) {
    return raw_bytes.status();
  }
  auto bytes = remove_checksum(raw_bytes.value());
  if (!bytes.ok()) {
    return bytes.status();
  }

  asphrclient::PublicID proto_struct;
  if (!proto_struct.ParseFromString(bytes.value())) {
    return absl::UnknownError("failed to parse message");
  }

  return PublicIdentifier(proto_struct.index(), proto_struct.kx_public_key(),
                          proto_struct.invitation_public_key());
}

auto PublicIdentifier::to_public_id() const -> string {
  asphrclient::PublicID proto_struct;
  proto_struct.set_index(index);
  proto_struct.set_kx_public_key(kx_public_key);
  proto_struct.set_invitation_public_key(invitation_public_key);
  auto bytes = proto_struct.SerializeAsString();
  return base58::Encode(append_checksum(bytes));
}

SyncIdentifier::SyncIdentifier(int index, string kx_public_key)
    : index(index), kx_public_key(kx_public_key) {}

// may lose information about leading zeros
auto change_base(vector<int> values, std::function<int(int)> from_base,
                 std::function<int(int)> to_base) -> vector<int> {
  // stored in reverse, to be reversed at the end
  vector<int> result;
  for (size_t i = 0; i < values.size(); i++) {
    // multiply everything by current base!
    int carry = values.at(i);
    ASPHR_ASSERT(carry < from_base(std::ssize(values) - 1 - i));
    int j = 0;
    for (j = 0; j < std::ssize(result); j++) {
      int new_value =
          result.at(j) * from_base(std::ssize(values) - 1 - i) + carry;
      result.at(j) = new_value % to_base(j);
      carry = new_value / to_base(j);
    }
    while (carry > 0) {
      result.push_back(carry % to_base(j));
      carry /= to_base(j);
      j++;
    }
  }
  // reverse result
  return vector<int>(result.rbegin(), result.rend());
}

auto SyncIdentifier::from_story(string story)
    -> asphr::StatusOr<SyncIdentifier> {
  ASPHR_ASSERT(std::ssize(wordlist::nouns_vec) > 1000);
  ASPHR_ASSERT(std::ssize(wordlist::verbs_vec) > 500);
  ASPHR_ASSERT(std::ssize(wordlist::adjectives_vec) > 400);
  // create a vector of words
  std::vector<string> words;
  for (auto word : absl::StrSplit(story, ' ')) {
    // remove any trailing punctuation
    word = word.substr(0, word.find_last_not_of(".,;:!?\"'") + 1);
    if (word.empty()) {
      continue;
    }
    // make word lowercase
    auto lowercase = absl::AsciiStrToLower(word);
    words.push_back(string(lowercase));
  }
  // get indices of words in word list
  std::vector<int> indices;
  int j = 0;
  while (j < std::ssize(words)) {
    ASPHR_ASSERT_EQ(j % 4, 0);
    {
      auto word = words.at(j + 0);
      auto it = std::find(wordlist::adjectives_vec.begin(),
                          wordlist::adjectives_vec.end(), word);
      if (it == wordlist::adjectives_vec.end()) {
        return absl::InvalidArgumentError("unknown word: " + word);
      }
      indices.push_back(it - wordlist::adjectives_vec.begin());
    }
    if (j + 1 >= std::ssize(words)) {
      break;
    }
    {
      auto word = words.at(j + 1);
      auto it = std::find(wordlist::nouns_vec.begin(),
                          wordlist::nouns_vec.end(), word);
      if (it == wordlist::nouns_vec.end()) {
        return absl::InvalidArgumentError("unknown word: " + word);
      }
      indices.push_back(it - wordlist::nouns_vec.begin());
    }
    if (j + 2 >= std::ssize(words)) {
      break;
    }
    {
      auto word = words.at(j + 2);
      auto it = std::find(wordlist::verbs_vec.begin(),
                          wordlist::verbs_vec.end(), word);
      if (it == wordlist::verbs_vec.end()) {
        return absl::InvalidArgumentError("unknown word: " + word);
      }
      indices.push_back(it - wordlist::verbs_vec.begin());
    }
    if (j + 3 >= std::ssize(words)) {
      break;
    }
    {
      auto word = words.at(j + 3);
      auto it = std::find(wordlist::nouns_vec.begin(),
                          wordlist::nouns_vec.end(), word);
      if (it == wordlist::nouns_vec.end()) {
        return absl::InvalidArgumentError("unknown word: " + word);
      }
      indices.push_back(it - wordlist::nouns_vec.begin());
    }
    j += 4;
  }

  // reverse indices
  std::reverse(indices.begin(), indices.end());

  // change basis to 256 constant base
  std::vector<int> base256_indices = change_base(
      indices,
      [](int i) {
        switch (i % 4) {
          case 0:
            return wordlist::adjectives_vec.size();
          case 1:
            return wordlist::nouns_vec.size();
          case 2:
            return wordlist::verbs_vec.size();
          case 3:
            return wordlist::nouns_vec.size();
          default:
            ASPHR_LOG_ERR("unexpected case", i, i, i_mod_4, i % 4);
            ASPHR_ASSERT_MSG(false, "should never ever happen");
        }
      },
      [](int i) { return 257; });

  for (size_t i = 0; i < base256_indices.size(); i++) {
    ASPHR_ASSERT(base256_indices.at(i) < 257);
    ASPHR_ASSERT(base256_indices.at(i) >= 0);
    if (base256_indices.at(i) == 0) {
      // this should never happen for real stories that are generated by us.
      // it MAY happen if someone maliciously generates a story, though
      // return an error
      return absl::InvalidArgumentError(
          "story contains zero; not a valid story");
    }
    base256_indices.at(i) -= 1;
  }

  // convert to bytes
  string raw_bytes = "";
  for (int i = 0; i < std::ssize(base256_indices); i++) {
    raw_bytes += static_cast<unsigned char>(base256_indices.at(i));
  }

  auto bytes = remove_checksum(raw_bytes);
  if (!bytes.ok()) {
    return bytes.status();
  }

  asphrclient::PublicID proto_struct;
  if (!proto_struct.ParseFromString(bytes.value())) {
    return absl::UnknownError("failed to parse message");
  }

  return SyncIdentifier(proto_struct.index(), proto_struct.kx_public_key());
}

auto SyncIdentifier::to_story() const -> string {
  ASPHR_ASSERT(wordlist::nouns_vec.size() > 1000);
  ASPHR_ASSERT(wordlist::verbs_vec.size() > 500);
  ASPHR_ASSERT(wordlist::adjectives_vec.size() > 400);

  asphrclient::PublicID proto_struct;
  proto_struct.set_index(index);
  proto_struct.set_kx_public_key(kx_public_key);
  auto bytes = proto_struct.SerializeAsString();
  auto raw_bytes = append_checksum(bytes);

  // convert to base 256
  vector<int> base256_indices;
  for (int i = 0; i < std::ssize(raw_bytes); i++) {
    base256_indices.push_back((uint8_t)raw_bytes.at(i));
  }
  for (int i = 0; i < std::ssize(raw_bytes); i++) {
    base256_indices[i] += 1;
  }

  std::vector<int> wordbase_indices = change_base(
      base256_indices, [](int i) { return 257; },
      [](int i) {
        switch (i % 4) {
          case 0:
            return std::ssize(wordlist::adjectives_vec);
          case 1:
            return std::ssize(wordlist::nouns_vec);
          case 2:
            return std::ssize(wordlist::verbs_vec);
          case 3:
            return std::ssize(wordlist::nouns_vec);
          default:
            ASPHR_LOG_ERR("unexpected case", i, i, i_mod_4, i % 4);
            ASPHR_ASSERT_MSG(false, "should never ever happen");
        }
      });

  // reverse indices
  std::reverse(wordbase_indices.begin(), wordbase_indices.end());

  // get words in wordlist
  std::vector<string> words;
  for (int i = 0; i < std::ssize(wordbase_indices); i++) {
    ASPHR_ASSERT(wordbase_indices.at(i) >= 0);
    switch (i % 4) {
      case 0:
        ASPHR_ASSERT(static_cast<size_t>(wordbase_indices.at(i)) <=
                     wordlist::adjectives_vec.size());
        words.push_back(wordlist::adjectives_vec.at(wordbase_indices.at(i)));
        break;
      case 1:
        ASPHR_ASSERT(static_cast<size_t>(wordbase_indices.at(i)) <=
                     wordlist::nouns_vec.size());
        words.push_back(wordlist::nouns_vec.at(wordbase_indices.at(i)));
        break;
      case 2:
        ASPHR_ASSERT(static_cast<size_t>(wordbase_indices.at(i)) <=
                     wordlist::verbs_vec.size());
        words.push_back(wordlist::verbs_vec.at(wordbase_indices.at(i)));
        break;
      case 3:
        ASPHR_ASSERT(static_cast<size_t>(wordbase_indices.at(i)) <=
                     wordlist::nouns_vec.size());
        words.push_back(wordlist::nouns_vec.at(wordbase_indices.at(i)));
        break;
      default:
        ASPHR_LOG_ERR("unexpected case", i, i, i_mod_4, i % 4);
        ASPHR_ASSERT_MSG(false, "should never ever happen");
    }
  }

  // join words
  string story = "";
  for (int i = 0; i < std::ssize(words); i++) {
    if (i != 0) {
      story += " ";
    }
    auto word = words.at(i);
    if (i % 4 == 0) {
      // capitalize word
      auto first_letter = word.substr(0, 1);
      word = string(absl::AsciiStrToUpper(first_letter)) + word.substr(1);
    }
    story += word;
    if (i % 4 == 3) {
      story += ".";
    }
  }
  if (words.size() % 4 != 0) {
    story += ".";
  }

  return story;
}