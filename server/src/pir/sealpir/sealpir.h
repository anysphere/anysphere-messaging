#pragma once

#include <seal/seal.h>

#pragma once

#include <array>
#include <string>
#include <vector>

#include "asphr/asphr.h"

using std::array;
using std::string;
using std::vector;

struct SealPIRQuery {
  pir_index_t index;

  auto serialize_to_string() noexcept -> string {
    return reinterpret_cast<const char *>(index);
  }
  // returns: whether deserialization was successful
  auto deserialize_from_string(const string &s) noexcept -> bool {
    if (s.size() != sizeof(pir_index_t)) {
      return false;
    }
    index = *reinterpret_cast<const pir_index_t *>(s.data());

    return true;
  }
};

struct SealPIRAnswer {
  pir_value_t value;

  auto serialize_to_string() noexcept -> string {
    string s(value.begin(), value.end());
    return s;
  }
  // returns: whether deserialization was successful
  auto deserialize_from_string(const string &s) noexcept -> bool {
    if (s.size() != MESSAGE_SIZE) {
      return false;
    }
    std::copy(s.begin(), s.end(), value.begin());

    return true;
  }
};

// TODO: add thread safety argument
class SealPIR {
 public:
  using pir_query_t = SealPIRQuery;
  using pir_answer_t = SealPIRAnswer;

  SealPIR() {}

  auto set_value(pir_index_t index, pir_value_t value) -> void {
    db[index] = value;
  }

  auto get_value_privately(pir_query_t pir_query) -> pir_answer_t {
    return SealPIRAnswer{db[pir_query.index]};
  }

 private:
  vector<pir_value_t> db;
};