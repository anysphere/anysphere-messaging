//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#pragma once

#include <assert.h>
#include <seal/seal.h>

#include <array>
#include <string>
#include <vector>

#include "asphr/asphr/asphr.hpp"
#include "asphr/pir/fast_pir/fast_pir.hpp"

// TODO: add thread safety argument
// probablyyyy all races are benign.
class FastPIR {
 public:
  using pir_query_t = FastPIRQuery<seal::Ciphertext, seal::GaloisKeys>;
  using pir_answer_t = FastPIRAnswer;

  FastPIR()
      : sc(create_context_params()),
        seal_slot_count(seal::BatchEncoder(sc).slot_count()) {
    check_rep();
  }

  auto set_value(pir_index_t index, pir_value_t value) noexcept -> void;

  // throws if database is empty
  auto get_value_privately(pir_query_t pir_query) noexcept(false)
      -> pir_answer_t;

  auto allocate() noexcept -> pir_index_t;
  auto allocate_to_max(pir_index_t max_index) noexcept -> void;

  // throws if deserialization fails
  auto query_from_string(const string& s) const noexcept(false) -> pir_query_t {
    pir_query_t query;
    query.deserialize_from_string(s, sc);
    return query;
  }

  auto get_db_rows() const noexcept -> size_t { return db_rows; }

 private:
  // db is an db_rows x MESSAGE_SIZE matrix
  // it contains the raw data, in row-major order
  vector<byte> db;
  size_t db_rows = 0;
  // seal context contains the parameters for the homomorphic encryption scheme
  seal::SEALContext sc;
  // number of slots in the plaintext
  size_t seal_slot_count;
  // this will be basically ceil(db_rows / seal_slot_count)
  size_t seal_db_rows = 0;
  // seal_db contains a seal plaintext-encoded version of db
  // note that because of races, we might not have that this is exactly true...
  // the dimension of this database is seal_db_rows x SEAL_DB_COLUMNS, stored in
  // row-major order
  vector<seal::Plaintext> seal_db;

  auto check_rep() const -> void {
    assert(db.size() == db_rows * MESSAGE_SIZE);

    assert(seal_db_rows == CEIL_DIV(db_rows, seal_slot_count));
    assert(seal_db.size() == seal_db_rows * SEAL_DB_COLUMNS);
  }

  auto db_index(pir_index_t index) const -> int { return index * MESSAGE_SIZE; }

  auto update_seal_db(pir_index_t index) -> void;
};