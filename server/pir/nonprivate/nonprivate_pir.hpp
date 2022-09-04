//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "asphr/asphr/asphr.hpp"

using std::array;
using std::string;
using std::vector;

struct NonPrivatePIRQuery {
  pir_index_t index;

  auto serialize_to_string() noexcept -> string {
    return reinterpret_cast<const char*>(index);
  }
  // throws if deserialization was successful
  auto deserialize_from_string(const string& s) noexcept(false) -> void {
    if (s.size() != sizeof(pir_index_t)) {
      throw std::invalid_argument("invalid size");
    }
    index = *reinterpret_cast<const pir_index_t*>(s.data());
  }
};

struct NonPrivatePIRAnswer {
  pir_value_t value;

  auto serialize_to_string() noexcept -> string {
    string s(value.begin(), value.end());
    return s;
  }
  // throws if deserialization was successful
  auto deserialize_from_string(const string& s) noexcept(false) -> bool {
    if (s.size() != MESSAGE_SIZE) {
      throw std::invalid_argument("invalid size");
    }
    std::copy(s.begin(), s.end(), value.begin());

    return true;
  }
};

// TODO: add thread safety argument
class NonPrivatePIR {
 public:
  using pir_query_t = NonPrivatePIRQuery;
  using pir_answer_t = NonPrivatePIRAnswer;

  NonPrivatePIR();

  auto set_value(pir_index_t index, pir_value_t value) noexcept -> void;
  auto get_value_privately(pir_query_t pir_query) noexcept -> pir_answer_t;
  auto allocate() noexcept -> pir_index_t;

  // may throw if deserialization fails
  static auto query_from_string(const string& s) noexcept(false) -> pir_query_t;

 private:
  vector<pir_value_t> db;
};