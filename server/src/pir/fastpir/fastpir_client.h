#include "fastpir.h"
#include <bitset>
#include <array>
#include "fastpir_config.h"

using std::array;
using std::bitset;

auto gen_secret_key(seal::KeyGenerator keygen) -> seal::SecretKey
{
    auto secret_key = keygen.secret_key();
    return secret_key;
}

auto gen_galois_keys(seal::KeyGenerator keygen) -> seal::Serializable<seal::GaloisKeys>
{
    auto galois_keys = keygen.create_galois_keys();
    return galois_keys;
}

template <int N>
auto concat_lsb(const vector<uint64_t> &v) -> vector<byte>
{
    // const auto num_bits = 18;
    bitset<8 * N> bits;
    vector<byte> result;

    for (size_t i = 0; i < v.size(); i += 8)
    {
        bits = 0;
        for (size_t j = 0; j < 8; j++)
        {
            if (i + j >= v.size())
            {
                break;
            }
            auto extract_N_bits = v[i + j] & ((1 << N) - 1);
            bits <<= N;
            bits |= extract_N_bits;
        }
        for (size_t j = 0; j < N; j++)
        {
            // get 8 bits from the j-th bit to the j+3-th bit
            bitset<8 *N> mask = 0;
            mask = (1 << 8) - 1;
            mask <<= (j * 8);
            auto extract_bits = (bits & mask) >> (j * 8);
            byte b = static_cast<byte>(extract_bits.to_ulong());
            result.push_back(b);
        }
    }

    size_t output_size = v.size() * N / 8;
    assert(output_size <= result.size());
    for (size_t i = 0; i < result.size() - output_size; i++)
    {
        result.pop_back();
    }

    return result;
}

class FastPIRClient
{
public:
    using pir_query_t = FastPIRQuery<seal::Serializable<seal::Ciphertext>, seal::Serializable<seal::GaloisKeys>>;
    using pir_answer_t = FastPIRAnswer;

    // TODO: fix this absolute mess of initializers???

    FastPIRClient() : FastPIRClient(create_context_params()) {}

    FastPIRClient(seal::SEALContext sc) : FastPIRClient(sc, seal::KeyGenerator(sc)) {}

    FastPIRClient(seal::SEALContext sc, seal::KeyGenerator keygen) : FastPIRClient(sc, keygen.secret_key(), keygen.create_galois_keys()) {}

    FastPIRClient(seal::SEALContext sc, seal::SecretKey secret_key, seal::Serializable<seal::GaloisKeys> galois_keys) : sc(sc), batch_encoder(sc), seal_slot_count(batch_encoder.slot_count()), secret_key(secret_key), galois_keys(galois_keys), encryptor(sc, secret_key), decryptor(sc, secret_key), evaluator(sc) {}

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
                seal::Plaintext select_p;
                batch_encoder.encode(plain_coefficients, select_p);
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

    auto decode(pir_answer_t answer, pir_index_t index) -> pir_value_t
    {
        seal::Plaintext plain_answer;
        decryptor.decrypt(answer.answer, plain_answer);

        vector<uint64_t> message_coefficients;
        batch_encoder.decode(plain_answer, message_coefficients);

        assert(message_coefficients.size() == seal_slot_count);

        // TODO: remove this because it is debug
        // std::cout << "message_coefficients: ";
        // for (auto c : message_coefficients)
        // {
        //     std::cout << c << " ";
        // }
        // std::cout << std::endl;

        // rotate!
        if (index % seal_slot_count > seal_slot_count / 2)
        {
            vector<uint64_t> message_coefficients_new(seal_slot_count);
            for (size_t i = 0; i < seal_slot_count; i++)
            {
                message_coefficients_new[i] = message_coefficients[(i + seal_slot_count / 2) % seal_slot_count];
            }
            message_coefficients = message_coefficients_new;
        }
        // rotate even more!
        vector<uint64_t> message_coefficients_new(seal_slot_count);
        for (size_t r = 0; r < 2; r++)
        {
            for (size_t i = 0; i < seal_slot_count / 2; i++)
            {
                message_coefficients_new[r * seal_slot_count / 2 + i] = message_coefficients[(i + index) % (seal_slot_count / 2) + r * seal_slot_count / 2];
            }
        }
        message_coefficients = message_coefficients_new;

        std::cout << "message_coefficients_new: ";
        for (auto c : message_coefficients)
        {
            std::cout << c << ",";
        }
        std::cout << std::endl;

        // convert to bytes!
        std::cout << "message_bytes_vector: " << std::endl;
        auto message_bytes_vector = concat_lsb<PLAIN_BITS>(message_coefficients);
        for (auto b : message_bytes_vector)
        {
            std::cout << static_cast<int>(b) << ",";
        }
        std::cout << std::endl;

        assert(message_bytes_vector.size() >= MESSAGE_SIZE);

        array<byte, MESSAGE_SIZE> message_bytes;
        for (size_t i = 0; i < MESSAGE_SIZE; i++)
        {
            message_bytes[i] = message_bytes_vector[i];
        }

        return pir_value_t{message_bytes};
    }

    // TODO: REMOVE THIS
    auto decrypt(seal::Ciphertext ciphertext) -> seal::Plaintext
    {
        seal::Plaintext plaintext;
        decryptor.decrypt(ciphertext, plaintext);
        return plaintext;
    }

    // TODO: REMOVE THIS
    auto get_context() -> seal::SEALContext
    {
        return sc;
    }
    auto get_secret_key() -> seal::SecretKey
    {
        return secret_key;
    }

private:
    seal::SEALContext sc;
    seal::BatchEncoder batch_encoder;
    // number of slots in the plaintext
    size_t seal_slot_count;
    seal::SecretKey secret_key;
    seal::Serializable<seal::GaloisKeys> galois_keys;
    seal::Encryptor encryptor;
    seal::Decryptor decryptor;
    seal::Evaluator evaluator;
};
