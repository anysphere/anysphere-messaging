#include "fastpir.h"
#include "fastpir_config.h"

auto gen_secret_key(seal::SEALContext context) -> seal::SecretKey
{
    seal::KeyGenerator keygen(context);
    auto secret_key = keygen.secret_key();
    return secret_key;
}

auto gen_galois_keys(seal::SEALContext context) -> seal::Serializable<seal::GaloisKeys>
{
    seal::KeyGenerator keygen(context);
    auto galois_keys = keygen.create_galois_keys();
    return galois_keys;
}

class FastPIRClient
{
public:
    using pir_query_t = FastPIRQuery<seal::Serializable<seal::Ciphertext>, seal::Serializable<seal::GaloisKeys>>;
    using pir_answer_t = FastPIRAnswer;

    // TODO: fix this absolute mess of initializers???

    FastPIRClient() : FastPIRClient(create_context_params()) {}

    FastPIRClient(seal::SEALContext sc) : FastPIRClient(gen_secret_key(sc), gen_galois_keys(sc)) {}

    FastPIRClient(seal::SecretKey secret_key, seal::Serializable<seal::GaloisKeys> galois_keys) : FastPIRClient(create_context_params(), secret_key, galois_keys) {}

    FastPIRClient(seal::SEALContext sc, seal::SecretKey secret_key, seal::Serializable<seal::GaloisKeys> galois_keys) : sc(sc), batch_encoder(sc), seal_slot_count(batch_encoder.slot_count()), secret_key(secret_key), galois_keys(galois_keys), encryptor(sc, secret_key) {}

    auto query(pir_index_t index, size_t db_rows) -> pir_query_t
    {
        assert(index < db_rows);
        vector<seal::Serializable<seal::Ciphertext>> query;
        auto seal_db_rows = CEIL_DIV(db_rows, seal_slot_count);
        query.reserve(seal_db_rows);
        auto seal_db_index = index / seal_slot_count;
        for (size_t i = 0; i < seal_db_rows; i++)
        {
            if (i == seal_db_index)
            {
                // compute seal_db_index encryption!
                auto coefficient_index = index % seal_slot_count;
                vector<uint64_t> plain_coefficients(seal_slot_count, 0);
                plain_coefficients[coefficient_index] = 1;
                seal::Plaintext select_p(plain_coefficients);
                query.push_back(encryptor.encrypt_symmetric(select_p));
            }
            else
            {
                // TODO: we could use encyptor.encrypt_zero_symmetric here. we would probably want to audit
                // that code first, though, because it is a less commonly used function so it has a higher risk
                // of having bugs. and bugs here are CRITICAL.
                seal::Plaintext p("0");
                // note: even though these ciphertexts are all encryptions of 0, it is CRUCIAL that they are independent encryptions
                // that is, this code MAY NOT be moved out of this loop, despite it looking like it can be. the encryption
                // is randomized.
                query.push_back(encryptor.encrypt_symmetric(p));
            }
        }

        // TODO: optimize this by sending over the galois keys on registration, NOT on every single query
        auto pir_query = pir_query_t{query, galois_keys};

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
    seal::Serializable<seal::GaloisKeys> galois_keys;
    seal::Encryptor encryptor;
};
