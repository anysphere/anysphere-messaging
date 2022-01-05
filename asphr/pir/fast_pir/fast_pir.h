#pragma once

#include <assert.h>
#include <seal/seal.h>

#include <array>
#include <string>
#include <vector>

#include "asphr/asphr.h"
#include "fast_pir_config.h"

template <typename Ciphertext_t, typename GaloisKeys_t>
struct FastPIRQuery {
  // TODO: STORE GALOIS KEYS IN POSTGRES, DO NOT SEND IN EVERY QUERY BECAUSE
  // THEY DO NOT CHANGE
  vector<Ciphertext_t> query;
  GaloisKeys_t galois_keys;

  auto serialize_to_string() noexcept(false) -> string {
    std::stringstream s_stream;
    galois_keys.save(s_stream);
    for (const auto& c : query) {
      c.save(s_stream);
    }
    return s_stream.str();
  }

  // throws if deserialization fails
  auto deserialize_from_string(const string& s,
                               seal::SEALContext sc) noexcept(false) -> void {
    auto s_stream = std::stringstream(s);
    size_t position = 0;
    position += galois_keys.load(sc, s_stream);
    while (position < s.size()) {
      seal::Ciphertext c;
      position += c.load(sc, s_stream);
      query.push_back(c);
    }
  }
};

struct FastPIRAnswer {
  seal::Ciphertext answer;

  // throws if serialization fails
  auto serialize_to_string() noexcept(false) -> string {
    std::stringstream s_stream;
    answer.save(s_stream);
    return s_stream.str();
  }
  // throws if deserialization fails
  auto deserialize_from_string(const string& s,
                               seal::SEALContext sc) noexcept(false) -> void {
    auto s_stream = std::stringstream(s);
    answer.load(sc, s_stream);
  }
};
