#pragma once

#include <array>
#include <bitset>

#include "asphr/asphr.h"
#include "fastpir.h"

using std::array;
using std::bitset;

struct Galois_string {
  Galois_string(const string &s) : galois_string(s) {}

  string galois_string;
  auto save(std::ostream &os) const -> void { os << galois_string; }
};

auto generate_keys() -> std::pair<std::string, std::string> {
  seal::SEALContext sc(create_context_params());
  seal::KeyGenerator keygen(sc);
  auto secret_key = keygen.secret_key();
  auto galois_keys = keygen.create_galois_keys();

  std::stringstream s_stream;
  secret_key.save(s_stream);

  std::stringstream g_stream;
  galois_keys.save(g_stream);

  return {s_stream.str(), g_stream.str()};
}

auto gen_secret_key(seal::KeyGenerator keygen) -> seal::SecretKey {
  auto secret_key = keygen.secret_key();
  return secret_key;
}

auto gen_galois_keys(seal::Serializable<seal::GaloisKeys> gk) -> string {
  std::stringstream g_stream;
  gk.save(g_stream);
  return g_stream.str();
}

class FastPIRClient {
 public:
  using pir_query_t =
      FastPIRQuery<seal::Serializable<seal::Ciphertext>, Galois_string>;
  using pir_answer_t = FastPIRAnswer;

  // TODO: fix this absolute mess of initializers???

  FastPIRClient() : FastPIRClient(create_context_params()) {}

  FastPIRClient(seal::SEALContext sc)
      : FastPIRClient(sc, seal::KeyGenerator(sc)) {}

  FastPIRClient(seal::SEALContext sc, seal::KeyGenerator keygen)
      : FastPIRClient(sc, keygen.secret_key(),
                      gen_galois_keys(keygen.create_galois_keys())) {}

  FastPIRClient(string secret_key, string galois_keys)
      : FastPIRClient(seal::SEALContext(create_context_params()), secret_key,
                      galois_keys) {}

  FastPIRClient(seal::SEALContext sc, string secret_key, string galois_keys)
      : FastPIRClient(sc, deserialize_secret_key(sc, secret_key), galois_keys) {
  }

  FastPIRClient(seal::SecretKey secret_key, string galois_keys)
      : FastPIRClient(seal::SEALContext(create_context_params()), secret_key,
                      galois_keys) {}

  FastPIRClient(seal::SEALContext sc, seal::SecretKey secret_key,
                string galois_keys)
      : sc(sc),
        batch_encoder(sc),
        seal_slot_count(batch_encoder.slot_count()),
        secret_key(secret_key),
        galois_keys(galois_keys),
        encryptor(sc, secret_key),
        decryptor(sc, secret_key),
        evaluator(sc) {}

  auto query(pir_index_t index, size_t db_rows) -> pir_query_t {
    assert(index < db_rows);
    vector<seal::Serializable<seal::Ciphertext>> query;
    auto seal_db_rows = CEIL_DIV(db_rows, seal_slot_count);
    query.reserve(seal_db_rows);
    auto seal_db_index = index / seal_slot_count;
    for (size_t i = 0; i < seal_db_rows; i++) {
      if (i == seal_db_index) {
        // compute seal_db_index encryption!
        auto coefficient_index = index % seal_slot_count;
        vector<uint64_t> plain_coefficients(seal_slot_count, 0);
        plain_coefficients[coefficient_index] = 1;
        seal::Plaintext select_p;
        batch_encoder.encode(plain_coefficients, select_p);
        query.push_back(encryptor.encrypt_symmetric(select_p));
      } else {
        // TODO: we could use encyptor.encrypt_zero_symmetric here. we would
        // probably want to audit that code first, though, because it is a less
        // commonly used function so it has a higher risk of having bugs. and
        // bugs here are CRITICAL.
        seal::Plaintext p("0");
        // note: even though these ciphertexts are all encryptions of 0, it is
        // CRUCIAL that they are independent encryptions that is, this code MAY
        // NOT be moved out of this loop, despite it looking like it can be. the
        // encryption is randomized.
        query.push_back(encryptor.encrypt_symmetric(p));
      }
    }

    // TODO: optimize this by sending over the galois keys on registration, NOT
    // on every single query
    auto pir_query = pir_query_t{query, galois_keys};

    return pir_query;
  }

  auto decode(pir_answer_t answer, pir_index_t index) -> pir_value_t {
    seal::Plaintext plain_answer;
    decryptor.decrypt(answer.answer, plain_answer);

    vector<uint64_t> message_coefficients;
    batch_encoder.decode(plain_answer, message_coefficients);

    assert(message_coefficients.size() == seal_slot_count);

    // rotate!
    if (index % seal_slot_count > seal_slot_count / 2) {
      vector<uint64_t> message_coefficients_new(seal_slot_count);
      for (size_t i = 0; i < seal_slot_count; i++) {
        message_coefficients_new[i] =
            message_coefficients[(i + seal_slot_count / 2) % seal_slot_count];
      }
      message_coefficients = message_coefficients_new;
    }
    // rotate even more!
    vector<uint64_t> message_coefficients_new(seal_slot_count);
    for (size_t r = 0; r < 2; r++) {
      for (size_t i = 0; i < seal_slot_count / 2; i++) {
        message_coefficients_new[r * seal_slot_count / 2 + i] =
            message_coefficients[(i + index) % (seal_slot_count / 2) +
                                 r * seal_slot_count / 2];
      }
    }
    message_coefficients = message_coefficients_new;

    auto message_bytes_vector =
        concat_N_lsb_bits<PLAIN_BITS>(message_coefficients);

    assert(message_bytes_vector.size() >= MESSAGE_SIZE);

    array<byte, MESSAGE_SIZE> message_bytes;
    for (size_t i = 0; i < MESSAGE_SIZE; i++) {
      message_bytes[i] = message_bytes_vector[i];
    }

    return pir_value_t{message_bytes};
  }

  // throws if deserialization fails
  auto answer_from_string(const string &s) const noexcept(false)
      -> pir_answer_t {
    pir_answer_t answer;
    answer.deserialize_from_string(s, sc);
    return answer;
  }

 private:
  seal::SEALContext sc;
  seal::BatchEncoder batch_encoder;
  // number of slots in the plaintext
  size_t seal_slot_count;
  seal::SecretKey secret_key;
  Galois_string galois_keys;
  seal::Encryptor encryptor;
  seal::Decryptor decryptor;
  seal::Evaluator evaluator;

  auto deserialize_secret_key(seal::SEALContext sc, string secret_key)
      -> seal::SecretKey {
    auto s_stream = std::stringstream(secret_key);
    seal::SecretKey sk;
    sk.load(sc, s_stream);
    return sk;
  }
};
