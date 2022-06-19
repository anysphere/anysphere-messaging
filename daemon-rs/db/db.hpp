#pragma once

#include "daemon-rs/db/mod.rs.h"
#include "rust/cxx.h"

// TODO: check that they are moved.
auto string_to_rust_u8Vec(const std::string& str) -> rust::Vec<uint8_t> {
  auto vec = rust::Vec<uint8_t>();
  for (auto c : str) {
    vec.push_back(c);
  }
  return vec;
}

auto rust_u8Vec_to_string(const rust::Vec<uint8_t>& vec) -> std::string {
  std::string str(vec.begin(), vec.end());
  return str;
}