#pragma once

#include <seal/seal.h>

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
    vector<seal::Ciphertext> query;

    // throws if deserialization fails
    auto deserialize_from_string(const string &s, seal::SEALContext sc) noexcept(false) -> void
    {
        auto s_stream = std::stringstream(s);
        size_t position = 0;
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
    auto deserialize_from_string(const string &s, seal::SEALContext sc) noexcept(false) -> bool
    {
        auto s_stream = std::stringstream(s);
        return answer.load(sc, s_stream);
    }
};

// TODO: add thread safety argument
// probablyyyy all races are benign.
class FastPIR
{
public:
    using pir_query_t = FastPIRQuery;
    using pir_answer_t = FastPIRAnswer;

    FastPIR() : sc(create_context_params()), batch_encoder(sc), seal_slot_count(batch_encoder.slot_count())
    {
        check_rep();
    }

    auto set_value(pir_index_t index, pir_value_t value) noexcept -> void
    {
        std::copy(value.begin(), value.end(), db.begin() + index * MESSAGE_SIZE);
        update_seal_db(index);
        check_rep();
    }

    auto get_value_privately(pir_query_t pir_query) noexcept -> pir_answer_t
    {
        // TODO: this is the hard part. do it
        return pir_answer_t{};
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
    // it contains the raw data
    vector<byte> db;
    int num_indices = 0;
    // seal context contains the parameters for the homomorphic encryption scheme
    seal::SEALContext sc;
    seal::BatchEncoder batch_encoder;
    // number of slots in the plaintext
    int seal_slot_count;
    // seal_db must have # of rows a multiple of seal_slot_count
    int seal_db_rows = 0;
    // seal_db contains a seal plaintext-encoded version of db
    // note that because of races, we might not have that this is exactly true...
    // the dimension of this database is seal_db_rows x SEAL_DB_COLUMNS
    vector<seal::Plaintext> seal_db;

    auto check_rep() const -> void
    {
        assert(db.size() == num_indices * MESSAGE_SIZE);

        assert(seal_db_rows % seal_slot_count == 0);
        assert(seal_db.size() == seal_db_rows * SEAL_DB_COLUMNS);
        assert(seal_db.size() * PLAIN_BITS >= num_indices * MESSAGE_SIZE_BITS);
        assert(seal_db.size() * PLAIN_BITS < num_indices * MESSAGE_SIZE_BITS + seal_slot_count * SEAL_DB_COLUMNS * PLAIN_BITS);
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
        seal_db_rows = CEIL_DIV(num_indices * MESSAGE_SIZE_BITS, seal_slot_count * PLAIN_BITS);
        seal_db.resize(seal_db_rows * SEAL_DB_COLUMNS);

        auto seal_db_index = index / seal_slot_count;
        auto db_start_block_index = db_index(seal_db_index * seal_slot_count);

        for (int j = 0; j < SEAL_DB_COLUMNS; j++)
        {
            vector<uint64_t> plain_text_coeffs(seal_slot_count);
            for (int slot = 0; slot < seal_slot_count; slot++)
            {

                plain_text_coeffs[slot] = db
            }
            seal_db[seal_db_index * SEAL_DB_COLUMNS + j] = seal::Plaintext(db.begin() + i * seal_slot_count * PLAIN_BITS + j * PLAIN_BITS, seal_slot_count);
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