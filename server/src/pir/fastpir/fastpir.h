#pragma once

#include <seal/seal.h>
#include <assert.h>

#pragma once

#include <vector>
#include <string>
#include <array>

#include "../pir_common.h"
#include "fastpir_config.h"

using std::array;
using std::size_t;
using std::string;
using std::vector;

#define CEIL_DIV(a, b) (((a) + (b)-1) / (b))

constexpr size_t SEAL_DB_COLUMNS = CEIL_DIV(MESSAGE_SIZE_BITS, PLAIN_BITS);

struct FastPIRQuery
{
    // TODO: STORE GALOIS KEYS IN POSTGRES, DO NOT SEND IN EVERY QUERY BECAUSE THEY DO NOT CHANGE
    vector<seal::Ciphertext> query;
    seal::GaloisKeys galois_keys;

    // throws if deserialization fails
    auto deserialize_from_string(const string &s, seal::SEALContext sc) noexcept(false) -> void
    {
        auto s_stream = std::stringstream(s);
        size_t position = 0;
        position += galois_keys.load(sc, s_stream);
        while (position < s.size())
        {
            seal::Ciphertext c;
            position += c.load(sc, s_stream);
            query.push_back(c);
        }
    }
};

struct FastPIRAnswer
{
    seal::Ciphertext answer;

    // throws if serialization fails
    auto serialize_to_string() noexcept(false) -> string
    {
        std::stringstream s_stream;
        answer.save(s_stream);
        return s_stream.str();
    }
    // throws if deserialization fails
    auto deserialize_from_string(const string &s, seal::SEALContext sc) noexcept(false) -> void
    {
        auto s_stream = std::stringstream(s);
        answer.load(sc, s_stream);
    }
};

// TODO: add thread safety argument
// probablyyyy all races are benign.
class FastPIR
{
public:
    using pir_query_t = FastPIRQuery;
    using pir_answer_t = FastPIRAnswer;

    FastPIR() : sc(create_context_params()), batch_encoder(sc), evaluator(sc), seal_slot_count(batch_encoder.slot_count())
    {
        check_rep();
    }

    auto set_value(pir_index_t index, pir_value_t value) noexcept -> void
    {
        std::copy(value.begin(), value.end(), db.begin() + index * MESSAGE_SIZE);
        update_seal_db(index);
        check_rep();
    }

    auto get_value_privately(pir_query_t pir_query) noexcept(false) -> pir_answer_t
    {
        if (pir_query.query.size() > seal_db_rows)
        {
            throw std::invalid_argument("query too large");
        }
        // note: pir_query might contain fewer ciphertexts than we expect, but that is ok!
        vector<seal::Ciphertext> compressed_cols(SEAL_DB_COLUMNS);
        for (size_t i = 0; i < SEAL_DB_COLUMNS; ++i)
        {
            evaluator.multiply_plain(pir_query.query[0], seal_db[0 + i], compressed_cols[i]);
            for (size_t j = 1; j < pir_query.query.size(); ++j)
            {
                seal::Ciphertext tmp;
                evaluator.multiply_plain(pir_query.query[j], seal_db[j * SEAL_DB_COLUMNS + i], tmp);
                evaluator.add_inplace(compressed_cols[i], tmp);
            }
        }
        seal::Ciphertext s_top = compressed_cols[0];
        seal::Ciphertext s_bottom = compressed_cols[seal_slot_count / 2];
        // combine using rotations!
        // TODO: optimize this by using the clever galois key thing
        for (size_t i = 0; i < SEAL_DB_COLUMNS; ++i)
        {
            if (i == 0 || i == seal_slot_count / 2)
            {
                continue;
            }
            if (i < seal_slot_count / 2)
            {
                evaluator.rotate_rows_inplace(compressed_cols[i], -i, pir_query.galois_keys);
                evaluator.add_inplace(s_top, compressed_cols[i]);
            }
            else
            {
                evaluator.rotate_rows_inplace(compressed_cols[i], -i + seal_slot_count / 2, pir_query.galois_keys);
                evaluator.add_inplace(s_bottom, compressed_cols[i]);
            }
        }
        evaluator.rotate_columns_inplace(s_bottom, pir_query.galois_keys);
        evaluator.add_inplace(s_top, s_bottom);
        return pir_answer_t{s_top};
    }

    auto allocate() noexcept -> pir_index_t
    {
        auto new_index = num_indices;
        num_indices++;
        db.resize(num_indices);
        update_seal_db(new_index);
        check_rep();
        return new_index;
    }

    // throws if deserialization fails
    auto query_from_string(const string &s) const noexcept(false) -> pir_query_t
    {
        pir_query_t query;
        query.deserialize_from_string(s, sc);
        return query;
    }

private:
    // db is an num_indices x MESSAGE_SIZE matrix
    // it contains the raw data, in row-major order
    vector<byte> db;
    int num_indices = 0;
    // seal context contains the parameters for the homomorphic encryption scheme
    seal::SEALContext sc;
    seal::BatchEncoder batch_encoder;
    seal::Evaluator evaluator;
    // number of slots in the plaintext
    int seal_slot_count;
    // this will be basically ceil(num_indices / seal_slot_count)
    int seal_db_rows = 0;
    // seal_db contains a seal plaintext-encoded version of db
    // note that because of races, we might not have that this is exactly true...
    // the dimension of this database is seal_db_rows x SEAL_DB_COLUMNS, stored in row-major order
    vector<seal::Plaintext> seal_db;

    auto check_rep() const -> void
    {
        assert(db.size() == num_indices * MESSAGE_SIZE);

        assert(seal_db_rows == CEIL_DIV(num_indices, seal_slot_count));
        assert(seal_db.size() == seal_db_rows * SEAL_DB_COLUMNS);
    }

    auto db_index(pir_index_t index) const -> int
    {
        return index * MESSAGE_SIZE;
    }

    auto create_context_params() -> seal::EncryptionParameters
    {
        seal::EncryptionParameters params(seal::scheme_type::bfv);
        params.set_poly_modulus_degree(POLY_MODULUS_DEGREE);
        vector<seal::Modulus> coeff_modulus;
        for (auto &prime : COEFF_MODULUS_FACTORIZATION)
        {
            coeff_modulus.push_back(seal::Modulus(prime));
        }
        params.set_coeff_modulus(coeff_modulus);
        params.set_plain_modulus(PLAIN_MODULUS);
        return params;
    }

    auto update_seal_db(pir_index_t index) -> void
    {
        seal_db_rows = CEIL_DIV(num_indices, seal_slot_count);
        seal_db.resize(seal_db_rows * SEAL_DB_COLUMNS);

        auto seal_db_index = index / seal_slot_count;
        auto db_start_block_index = db_index(seal_db_index * seal_slot_count);

        for (int j = 0; j < SEAL_DB_COLUMNS; j++)
        {
            auto plain_text_coeffs = get_submatrix_as_uint64s(db, MESSAGE_SIZE_BITS, db_start_block_index + j * PLAIN_BITS, PLAIN_BITS, seal_slot_count);
            seal_db[seal_db_index * SEAL_DB_COLUMNS + j] = seal::Plaintext(plain_text_coeffs);
        }

        check_rep();
    }
};

// extract a submatrix from a matrix db, where each row in the submatrix is a uint64_t
//
// db is a row-major stored matrix with db_row_length_in_bits bits per row.
// subm_top_left_corner_in_bits represent the index of the top left corner of the submatrix, in bits.
// subm_row_length_in_bits is the number of bits in each row of the submatrix.
// subm_cols is the number of columns in the submatrix.
//
// note: if subm_top_left_corner_in_bits + subm_row_length_in_bits goes past the right edge of the matrix, we DONT want
// to wrap around, but instead pretend that the db matrix is padded to the right with 0s.
//
// precondition: db.size() is a multiple of row_length_in_bits/8
//
//
auto get_submatrix_as_uint64s(const vector<byte> &db, size_t db_row_length_in_bits, size_t subm_top_left_corner_in_bits, size_t subm_row_length_in_bits, size_t subm_cols) -> vector<uint64_t>
{
    vector<uint64_t> coeffs(subm_cols);
    return coeffs;
}