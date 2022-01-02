#pragma once

#include <seal/seal.h>

#pragma once

#include <vector>
#include <string>
#include <array>

#include "../pir_common.h"

using std::array;
using std::string;
using std::vector;

struct FastPIRQuery
{
    pir_index_t index;

    auto serialize_to_string() noexcept -> string
    {
        return reinterpret_cast<const char *>(index);
    }
    // returns: whether deserialization was successful
    auto deserialize_from_string(const string &s) noexcept -> bool
    {
        if (s.size() != sizeof(pir_index_t))
        {
            return false;
        }
        index = *reinterpret_cast<const pir_index_t *>(s.data());

        return true;
    }
};

struct FastPIRAnswer
{
    pir_value_t value;

    auto serialize_to_string() noexcept -> string
    {
        string s(value.begin(), value.end());
        return s;
    }
    // returns: whether deserialization was successful
    auto deserialize_from_string(const string &s) noexcept -> bool
    {
        if (s.size() != MESSAGE_SIZE)
        {
            return false;
        }
        std::copy(s.begin(), s.end(), value.begin());

        return true;
    }
};

// TODO: add thread safety argument
class FastPIR
{
public:
    using pir_query_t = FastPIRQuery;
    using pir_answer_t = FastPIRAnswer;

    FastPIR() {}

    auto set_value(pir_index_t index, pir_value_t value) -> void
    {
        db[index] = value;
    }

    auto get_value_privately(pir_query_t pir_query) -> pir_answer_t
    {
        return FastPIRAnswer{db[pir_query.index]};
    }

    auto allocate() -> pir_index_t
    {
        auto new_index = db.size();
        db.push_back(pir_value_t{});
        return new_index;
    }

private:
    // db is an
    vector<pir_value_t> db;
};