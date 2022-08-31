//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "asphr/asphr/asphr.hpp"
#include "identifier.hpp"

int main(int argc, char** argv) {
  using namespace std::string_literals;
  auto si =
      SyncIdentifier(1000001, "\0\0\0xjjU\0\1\0\0xjji\0\1\0\0xjji\0\0\0xjji"s);
  auto pi = PublicIdentifier(
      1000001, "\0\1\0\0xjji\0\1A\0xjji\0\1\0\0xjji\0\1\0\0xjji"s,
      "y\0\1\0\0xjji\0\1\0\0xjji\0\1\0\0xjji\0\1\0\0xjj"s);

  cout << pi.index << endl;
  cout << pi.kx_public_key.size() << endl;
  cout << pi.invitation_public_key.size() << endl;

  cout << "public id: " << pi.to_public_id() << endl;

  cout << "story: " << si.to_story() << endl;

  return 0;
}