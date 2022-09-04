//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "asphr/asphr/asphr.hpp"

inline auto beta_key_authenticator(const std::string& beta_key) -> bool {
#ifdef USE_HN_BETA_KEY
  if (absl::StrContains(beta_key, "showhn")) {
#else
  if (absl::StrContains(beta_key, "asphr") &&
      absl::StrContains(beta_key, "magic")) {
#endif
    return true;
  }
  return false;
}
