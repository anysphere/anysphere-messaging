//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include <array>
#include <bitset>

#include "asphr/asphr.hpp"
#include "fast_pir.hpp"

using std::array;
using std::bitset;

#define DUMMY_INDEX 1'000'000

struct Galois_string {
  Galois_string(const string& s) : galois_string(s) {}

  string galois_string;
  auto save(std::ostream& os) const -> void { os << galois_string; }
};

// struct of data needs to be switched each encryption
struct keys {
  seal::SecretKey secret_key;
  Galois_string galois_keys;
};

auto generate_keys() -> std::pair<std::string, std::string>;

auto gen_secret_key(seal::KeyGenerator keygen) -> seal::SecretKey;

auto gen_galois_keys(seal::Serializable<seal::GaloisKeys> gk) -> string;

class FastPIRClient {
 public:
  using pir_query_t =
      FastPIRQuery<seal::Serializable<seal::Ciphertext>, Galois_string>;
  using pir_answer_t = FastPIRAnswer;
  using pir_map = std::map<pir_index_t, keys>;

  FastPIRClient() : FastPIRClient(create_context_params()) {
    ASPHR_LOG_INFO("Creating FastPIRClient.", from, "base");
  }

  FastPIRClient(seal::SEALContext sc)
      : FastPIRClient(sc, seal::KeyGenerator(sc)) {
    ASPHR_LOG_INFO("Creating FastPIRClient.", from, "context params");
  }

  FastPIRClient(seal::SEALContext sc, seal::KeyGenerator keygen)
      : sc(sc),
        batch_encoder(sc),
        seal_slot_count(batch_encoder.slot_count()),
        evaluator(sc),
        keys_map({}) {
    ASPHR_LOG_INFO("Creating FastPIRClient.", from, "context params, keygen");
  }

  auto query(pir_index_t index, size_t db_rows) -> pir_query_t {
    // reinitialize the secret key to deal with the pir replay attack
    const auto new_keys = generate_keys();
    const auto secret_key = this->deserialize_secret_key(sc, new_keys.first);
    const auto galois_keys = Galois_string(new_keys.second);
    // assign new keys to the keys map
    // note: you can save some time for the dummy index here.
    keys_map.insert_or_assign(index, keys{secret_key, galois_keys});
    // initialize encryptor
    auto encryptor = seal::Encryptor(sc, secret_key);
    // create the query
    assert(index < db_rows || index == DUMMY_INDEX);
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
    // obtain the last decryptor for this query.
    auto decryptor = seal::Decryptor(sc, keys_map.at(index).secret_key);
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
  auto answer_from_string(const string& s) const noexcept(false)
      -> pir_answer_t {
    pir_answer_t answer;
    answer.deserialize_from_string(s, sc);
    return answer;
  }

 private:
  seal::SEALContext sc;
  seal::BatchEncoder batch_encoder;
  // number of slots in the plaintext
  const size_t seal_slot_count;
  seal::Evaluator evaluator;

  // because we "batch" PIR encryption together, we need to know the keypair
  // corresponding to each index.
  // A Map (index -> keypair)
  pir_map keys_map;

  auto deserialize_secret_key(seal::SEALContext sc, string secret_key)
      -> seal::SecretKey {
    auto s_stream = std::stringstream(secret_key);
    seal::SecretKey sk;
    sk.load(sc, s_stream);
    return sk;
  }
};
