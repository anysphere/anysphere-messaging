#include "asphr/asphr.hpp"

auto beta_key_authenticator(const std::string& beta_key) -> bool {
  if (absl::StrContains(beta_key, "asphr") &&
      absl::StrContains(beta_key, "magic")) {
    return true;
  }
  return false;
}
