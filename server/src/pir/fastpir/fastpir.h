#pragma once

#include <assert.h>
#include <seal/seal.h>

#include <array>
#include <string>
#include <vector>

#include "asphr/asphr.h"
#include "fastpir_config.h"

using std::array;
using std::size_t;
using std::string;
using std::vector;

template <typename Ciphertext_t, typename GaloisKeys_t>
struct FastPIRQuery {
  // TODO: STORE GALOIS KEYS IN POSTGRES, DO NOT SEND IN EVERY QUERY BECAUSE
  // THEY DO NOT CHANGE
  vector<Ciphertext_t> query;
  GaloisKeys_t galois_keys;

  auto serialize_to_string() noexcept(false) -> string {
    std::stringstream s_stream;
    galois_keys.save(s_stream);
    for (const auto &c : query) {
      c.save(s_stream);
    }
    return s_stream.str();
  }

  // throws if deserialization fails
  auto deserialize_from_string(const string &s,
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
  auto deserialize_from_string(const string &s,
                               seal::SEALContext sc) noexcept(false) -> void {
    auto s_stream = std::stringstream(s);
    answer.load(sc, s_stream);
  }
};

// TODO: add thread safety argument
// probablyyyy all races are benign.
class FastPIR {
 public:
  using pir_query_t = FastPIRQuery<seal::Ciphertext, seal::GaloisKeys>;
  using pir_answer_t = FastPIRAnswer;

  FastPIR()
      : sc(create_context_params()),
        batch_encoder(sc),
        evaluator(sc),
        seal_slot_count(batch_encoder.slot_count()) {
    check_rep();
  }

  auto set_value(pir_index_t index, pir_value_t value) noexcept -> void {
    std::copy(value.begin(), value.end(), db.begin() + index * MESSAGE_SIZE);
    update_seal_db(index);
    check_rep();
  }

  auto get_value_privately(pir_query_t pir_query) noexcept(false)
      -> pir_answer_t {
    for (size_t i = 0; i < SEAL_DB_COLUMNS; i++) {
      vector<uint64_t> query_vec;
      batch_encoder.decode(seal_db[i], query_vec);
    }

    if (pir_query.query.size() > seal_db_rows) {
      throw std::invalid_argument("query too large");
    }
    // note: pir_query might contain fewer ciphertexts than we expect, but that
    // is ok!
    vector<seal::Ciphertext> compressed_cols(SEAL_DB_COLUMNS);
    for (size_t i = 0; i < SEAL_DB_COLUMNS; ++i) {
      evaluator.multiply_plain(pir_query.query[0], seal_db[0 + i],
                               compressed_cols[i]);
      for (size_t j = 1; j < pir_query.query.size(); ++j) {
        seal::Ciphertext tmp;
        evaluator.multiply_plain(pir_query.query[j],
                                 seal_db[j * SEAL_DB_COLUMNS + i], tmp);
        evaluator.add_inplace(compressed_cols[i], tmp);
      }
    }
    // we compress the answer into a single ciphertext. to be able to do that,
    // we need to have that SEAL_DB_COLUMNS <= seal_slot_count, or else we will
    // be overwriting information note: as with all things server-side, this is
    // about availability, not security.
    assert(SEAL_DB_COLUMNS <= seal_slot_count);

    seal::Ciphertext s_top = compressed_cols[0];
    seal::Ciphertext s_bottom;
    if (seal_slot_count / 2 < SEAL_DB_COLUMNS) {
      seal::Ciphertext s_bottom = compressed_cols[seal_slot_count / 2];
    }
    // combine using rotations!
    // TODO: optimize this by using the clever galois key thing
    for (size_t i = 0; i < SEAL_DB_COLUMNS; ++i) {
      if (i == 0 || i == seal_slot_count / 2) {
        continue;
      }
      if (i < seal_slot_count / 2) {
        evaluator.rotate_rows_inplace(compressed_cols[i], -i,
                                      pir_query.galois_keys);
        evaluator.add_inplace(s_top, compressed_cols[i]);
      } else {
        evaluator.rotate_rows_inplace(compressed_cols[i],
                                      -i + seal_slot_count / 2,
                                      pir_query.galois_keys);
        evaluator.add_inplace(s_bottom, compressed_cols[i]);
      }
    }
    if (seal_slot_count / 2 < SEAL_DB_COLUMNS) {
      evaluator.rotate_columns_inplace(s_bottom, pir_query.galois_keys);
      evaluator.add_inplace(s_top, s_bottom);
    }
    return pir_answer_t{s_top};
  }

  auto allocate() noexcept -> pir_index_t {
    auto new_index = db_rows;
    db_rows++;
    db.resize(db_rows * MESSAGE_SIZE);
    update_seal_db(new_index);
    check_rep();
    return new_index;
  }

  // throws if deserialization fails
  auto query_from_string(const string &s) const noexcept(false) -> pir_query_t {
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
  seal::BatchEncoder batch_encoder;
  seal::Evaluator evaluator;
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

  auto update_seal_db(pir_index_t index) -> void {
    seal_db_rows = CEIL_DIV(db_rows, seal_slot_count);
    seal_db.resize(seal_db_rows * SEAL_DB_COLUMNS);

    auto seal_db_index = index / seal_slot_count;
    auto db_start_block_index = db_index(seal_db_index * seal_slot_count);

    for (size_t j = 0; j < SEAL_DB_COLUMNS; j++) {
      auto plain_text_coeffs = get_submatrix_as_uint64s(
          db, MESSAGE_SIZE_BITS, db_start_block_index + j * PLAIN_BITS,
          PLAIN_BITS, seal_slot_count);
      batch_encoder.encode(plain_text_coeffs,
                           seal_db[seal_db_index * SEAL_DB_COLUMNS + j]);
    }

    check_rep();
  }
};