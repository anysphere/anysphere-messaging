//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "asphr/asphr/asphr.hpp"

struct PublicIdentifier {
  PublicIdentifier(int index, string kx_public_key,
                   string invitation_public_key);

  // not a constructor because may fail, and we don't want to use exceptions
  static auto from_public_id(string public_id)
      -> asphr::StatusOr<PublicIdentifier>;
  auto to_public_id() const -> string;

  int index;
  string kx_public_key;
  string invitation_public_key;
};

struct SyncIdentifier {
  SyncIdentifier(int index, string kx_public_key);

  // not a constructor because may fail, and we don't want to use exceptions
  static auto from_story(string story) -> asphr::StatusOr<SyncIdentifier>;
  auto to_story() const -> string;

  int index;
  string kx_public_key;
};