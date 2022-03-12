#include "./fastpir.hpp"

auto FastPIR::set_value(pir_index_t index, pir_value_t value) noexcept -> void {
  // this assert is guaranteed by the fact that the index is a valid index
  // access
  assert(index < db_rows);
  std::copy(value.begin(), value.end(), db.begin() + index * MESSAGE_SIZE);
  update_seal_db(index);
  check_rep();
}

auto FastPIR::get_value_privately(pir_query_t pir_query) noexcept(false)
    -> pir_answer_t {
  auto batch_encoder = seal::BatchEncoder(sc);
  auto evaluator = seal::Evaluator(sc);

  if (pir_query.query.size() > seal_db_rows) {
    // this is ok, and necessary for security.
    // we can just truncate!
    pir_query.query.resize(seal_db_rows);
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
  // TODO(unknown): optimize this by using the clever galois key thing
  for (size_t i = 0; i < SEAL_DB_COLUMNS; ++i) {
    if (i == 0 || i == seal_slot_count / 2) {
      continue;
    }
    if (i < seal_slot_count / 2) {
      evaluator.rotate_rows_inplace(
          compressed_cols[i], -1 * static_cast<int>(i), pir_query.galois_keys);
      evaluator.add_inplace(s_top, compressed_cols[i]);
    } else {
      evaluator.rotate_rows_inplace(
          compressed_cols[i], -i + seal_slot_count / 2, pir_query.galois_keys);
      evaluator.add_inplace(s_bottom, compressed_cols[i]);
    }
  }
  if (seal_slot_count / 2 < SEAL_DB_COLUMNS) {
    evaluator.rotate_columns_inplace(s_bottom, pir_query.galois_keys);
    evaluator.add_inplace(s_top, s_bottom);
  }
  return pir_answer_t{s_top};
}

auto FastPIR::allocate() noexcept -> pir_index_t {
  auto new_index = db_rows;
  db_rows++;
  db.resize(db_rows * MESSAGE_SIZE);
  update_seal_db(new_index);
  check_rep();
  return new_index;
}

auto FastPIR::allocate_to_max(pir_index_t max_index) noexcept -> void {
  assert(db_rows == 0);
  db_rows = max_index + 1;
  db.resize(db_rows * MESSAGE_SIZE);
  // TODO: performance-optimize this. it is not in the critical path tho, so not super important. but affects uptime
  for (pir_index_t i = 0; i <= max_index; i++) {
    update_seal_db(i);
  }
  check_rep();
}

auto FastPIR::update_seal_db(pir_index_t index) -> void {
  seal_db_rows = CEIL_DIV(db_rows, seal_slot_count);
  seal_db.resize(seal_db_rows * SEAL_DB_COLUMNS);

  auto seal_db_index = index / seal_slot_count;
  auto db_start_block_index = db_index(seal_db_index * seal_slot_count);

  auto batch_encoder = seal::BatchEncoder(sc);

  for (size_t j = 0; j < SEAL_DB_COLUMNS; j++) {
    auto plain_text_coeffs = get_submatrix_as_uint64s(
        db, MESSAGE_SIZE_BITS, db_start_block_index + j * PLAIN_BITS,
        PLAIN_BITS, seal_slot_count);
    batch_encoder.encode(plain_text_coeffs,
                         seal_db[seal_db_index * SEAL_DB_COLUMNS + j]);
  }

  check_rep();
}