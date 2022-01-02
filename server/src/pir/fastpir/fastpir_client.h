#include "fastpir.h"
#include "fastpir_config.h"

class FastPIRClient
{
public:
    using pir_query_t = FastPIRQuery;
    using pir_answer_t = FastPIRAnswer;

    FastPIRClient(seal::SecretKey secret_key) : sc(create_context_params()), batch_encoder(sc), seal_slot_count(batch_encoder.slot_count()), secret_key(secret_key), encryptor(secret_key) {}

    auto query(pir_index_t index, size_t db_rows) -> pir_query_t
    {
        assert(index < db_rows);
        pir_query_t pir_query;
        auto seal_db_rows = CEIL_DIV(db_rows, seal_slot_count);
        pir_query.query.resize(seal_db_rows);
        auto seal_db_index = index / seal_slot_count;
        for (size_t i = 0; i < seal_db_rows; i++)
        {
            if (i == seal_db_index)
            {
                continue;
            }
            seal::Plaintext p("0");
            // note: even though these ciphertexts are all encryptions of 0, it is CRUCIAL that they are independent encryptions
            // that is, this code MAY NOT be moved out of this loop, despite it looking like it can be. the encryption
            // is randomized.
            pir_query[i] = encryptor.encrypt_symmetric(p);
        }
        // compute seal_db_index encryption!
        auto coefficient_index = index % seal_slot_count;
        vector<uint64_t> plain_coefficients(seal_slot_count, 0);
        plain_coefficients[coefficient_index] = 1;
        seal::Plaintext select_p(plain_coefficients);
        pir_query[seal_db_index] = encryptor.encrypt_symmetric(select_p);

        return pir_query;
    }

    auto decode(pir_answer_t answer) -> pir_value_t
    {
        // TOOD: implement this
        return pir_value_t{};
    }

private:
    seal::SEALContext sc;
    seal::BatchEncoder batch_encoder;
    // number of slots in the plaintext
    size_t seal_slot_count;
    seal::SecretKey secret_key;
    seal::Encryptor encryptor;
};