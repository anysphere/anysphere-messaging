#pragma once

#include "asphr/asphr.hpp"
#include "rust/cxx.h"

inline auto unix_micros_now() -> int64_t {
  auto now = absl::Now();
  return absl::ToUnixMicros(now);
}