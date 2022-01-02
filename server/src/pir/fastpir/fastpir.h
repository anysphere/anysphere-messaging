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

    FastPIR() : sc(create_context()) {}

    auto set_value(pir_index_t index, pir_value_t value) noexcept -> void
    {
        std::copy(value.begin(), value.end(), db.begin() + index * MESSAGE_SIZE);
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
        return new_index;
    }

    // throws if deserialization fails
    auto query_from_string(const string &s) noexcept(false) -> pir_query_t
    {
        pir_query_t query;
        query.deserialize_from_string(s, sc);
        return query;
    }

private:
    // db is an num_indices x MESSAGE_SIZE matrix
    vector<byte> db;
    int num_indices = 0;
    // seal context contains the parameters for the homomorphic encryption scheme
    seal::SEALContext sc;

    auto db_index(pir_index_t index) -> int
    {
        return index * MESSAGE_SIZE;
    }

    auto create_context() -> seal::SEALContext
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
        return seal::SEALContext(params);
    }
};