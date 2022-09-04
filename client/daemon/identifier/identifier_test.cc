//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "identifier.hpp"

#include <gtest/gtest.h>

#include "asphr/schema/identifier.pb.h"
#include "base58.hpp"
#include "identifier_helpers_private.hpp"

using namespace std::string_literals;

TEST(IdentifierTest, Checksum) {
  vector<string> ss = {"", "a",
                       "\0\1jd"s
                       "\0"s,
                       "qiwejfioqwejfoiqwejf"s};

  for (auto s : ss) {
    auto cs = append_checksum(s);
    auto sp = remove_checksum(cs);
    ASSERT_TRUE(sp.ok());
    ASSERT_EQ(s, sp.value());
  }
}

TEST(IdentifierTest, ChecksumProto) {
  asphrclient::PublicID proto_struct;
  proto_struct.set_index(68);
  proto_struct.set_kx_public_key("kx_public_key"s);
  proto_struct.set_invitation_public_key("friend_request_public_key"s);
  auto bytes = proto_struct.SerializeAsString();

  auto cs = append_checksum(bytes);
  auto sp = remove_checksum(cs);
  ASSERT_TRUE(sp.ok());
  ASSERT_EQ(bytes, sp.value());

  auto css = base58::Encode(cs);
  auto cs2 = base58::Decode(css).value();
  auto spp = remove_checksum(cs2);
  ASSERT_EQ(cs, cs2);
  ASSERT_TRUE(spp.ok());
  ASSERT_EQ(bytes, spp.value());
}

TEST(IdentifierTest, Public) {
  vector<PublicIdentifier> ps = {
      PublicIdentifier(68, "kx_public_key", "friend_request_public_key"),
      PublicIdentifier(0, "x", "y"),
      PublicIdentifier(1000001,
                       "\0\1\0\0xjji\0\1\0\0xjji\0\1\0\0xjji\0\1\0\0xjji"s,
                       "y\0\1\0\0xjji\0\1\0\0xjji\0\1\0\0xjji\0\1\0\0xjj"s),
  };

  ASSERT_EQ(68, ps.at(0).index);
  ASSERT_EQ("kx_public_key", ps.at(0).kx_public_key);
  ASSERT_EQ("friend_request_public_key", ps.at(0).invitation_public_key);

  for (auto p : ps) {
    auto public_id = p.to_public_id();

    for (auto c : public_id) {
      ASSERT_NE(
          string("123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz")
              .find(c),
          string::npos);
    }

    auto p2maybe = PublicIdentifier::from_public_id(public_id);
    ASSERT_TRUE(p2maybe.ok())
        << p2maybe.status().message() << " " << p.index << " " << public_id;
    auto p2 = p2maybe.value();

    ASSERT_EQ(p.index, p2.index);
    ASSERT_EQ(p.kx_public_key, p2.kx_public_key);
    ASSERT_EQ(p.invitation_public_key, p2.invitation_public_key);

    // these tests should pass with probability 99.99%. if they fail, it might
    // be that we are in the 0.01%
    auto public_id_typo = p.to_public_id() + "x";
    auto p3maybe = PublicIdentifier::from_public_id(public_id_typo);
    ASSERT_FALSE(p3maybe.ok());
    auto public_id_typo_again = p.to_public_id();
    public_id_typo_again.at(9) = 'x';
    auto p4maybe = PublicIdentifier::from_public_id(public_id_typo_again);
    ASSERT_FALSE(p4maybe.ok());
  }
}

TEST(IdentifierTest, Sync) {
  vector<SyncIdentifier> ps = {
      SyncIdentifier(68, "kx_public_key"),
      SyncIdentifier(0, "x"),
      SyncIdentifier(1000001,
                     "\0\1\0\0xjji\0\1\0\0xjji\0\1\0\0xjji\0\1\0\0xjji"s),
  };

  ASSERT_EQ(68, ps.at(0).index);
  ASSERT_EQ("kx_public_key", ps.at(0).kx_public_key);

  for (auto p : ps) {
    auto story = p.to_story();

    for (auto c : story) {
      ASSERT_NE(string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz. ")
                    .find(c),
                string::npos);
    }

    auto p2maybe = SyncIdentifier::from_story(story);
    ASSERT_TRUE(p2maybe.ok());
    auto p2 = p2maybe.value();

    ASSERT_EQ(p.index, p2.index);
    ASSERT_EQ(p.kx_public_key, p2.kx_public_key);
  }
}

TEST(IdentifierTest, ChangeBasis) {
  {
    vector<int> hex16 = {1, 1};
    vector<int> dec16 = {1, 7};
    vector<int> hex16todec = change_base(
        hex16, [](int i) { return 16; }, [](int i) { return 10; });
    ASSERT_EQ(dec16, hex16todec);
    vector<int> dec16tohex = change_base(
        dec16, [](int i) { return 10; }, [](int i) { return 16; });
    ASSERT_EQ(hex16, dec16tohex);
  }

  {
    vector<int> hex16 = {1, 3};
    vector<int> dec16 = {1, 9};
    vector<int> hex16todec = change_base(
        hex16, [](int i) { return 16; }, [](int i) { return 10; });
    ASSERT_EQ(dec16, hex16todec);
    vector<int> dec16tohex = change_base(
        dec16, [](int i) { return 10; }, [](int i) { return 16; });
    ASSERT_EQ(hex16, dec16tohex);
  }
  {
    vector<int> x = {1, 2};
    vector<int> y = {1, 1, 1};
    vector<int> xtoy = change_base(
        x,
        [](int i) {
          switch (i % 2) {
            case 0:
              return 15;
            case 1:
              return 3;
          }
        },
        [](int i) {
          switch (i % 3) {
            case 0:
              return 4;
            case 1:
              return 3;
            case 2:
              return 3;
          }
        });
    ASSERT_EQ(y, xtoy);
    vector<int> ytox = change_base(
        y,
        [](int i) {
          switch (i % 3) {
            case 0:
              return 4;
            case 1:
              return 3;
            case 2:
              return 3;
          }
        },
        [](int i) {
          switch (i % 2) {
            case 0:
              return 15;
            case 1:
              return 3;
          }
        });
    ASSERT_EQ(x, ytox);
  }
}