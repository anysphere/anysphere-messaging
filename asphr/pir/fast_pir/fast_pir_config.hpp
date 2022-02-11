//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "asphr/asphr.hpp"

// these parameters are taken from
// https://github.com/ishtiyaque/FastPIR/blob/master/src/bfvparams.h
// TODO: optimize them

using std::array;
using std::size_t;
using std::vector;

constexpr size_t POLY_MODULUS_DEGREE = 4096;
static_assert(std::popcount(POLY_MODULUS_DEGREE) == 1,
              "POLY_MODULUS_DEGREE must be a power of 2");

constexpr uint64_t PRIME_54 = 18'014'398'509'309'953ULL;
// static_assert(is_prime(PRIME_54), "PRIME_54 must be prime"); // seems
// unnecessary to code this up just to check this. VERIFY MANUALLY.

constexpr uint64_t PRIME_55 = 36'028'797'018'652'673ULL;
// static_assert(is_prime(PRIME_55), "PRIME_55 must be prime"); // seems
// unnecessary to code this up just to check this. VERIFY MANUALLY.

constexpr array<uint64_t, 2> COEFF_MODULUS_FACTORIZATION({PRIME_54, PRIME_55});

// number of bits we store per coefficient. we store a power of 2 because
// performing modulo mod 2 is much much faster, when encoding
constexpr uint64_t PLAIN_BITS = 18;

// must be greater than 2^PLAIN_BITS
// must also be congruent to 1 modulo 2*POLY_MODULUS_DEGREE
// finally, must also be a prime
constexpr uint64_t PLAIN_MODULUS = 270'337;
static_assert(PLAIN_MODULUS >= 1ULL << PLAIN_BITS,
              "PLAIN_MODULUS must be greater than 2^PLAIN_BITS");
static_assert(
    PLAIN_MODULUS % (2 * POLY_MODULUS_DEGREE) == 1,
    "PLAIN_MODULUS must be congruent to 1 modulo 2*POLY_MODULUS_DEGREE");
// static_assert(is_prime(PLAIN_MODULUS), "PLAIN_MODULUS must be prime"); //
// seems unnecessary to code this up just to check this. VERIFY MANUALLY.

static auto create_context_params() -> seal::EncryptionParameters {
  seal::EncryptionParameters params(seal::scheme_type::bfv);
  params.set_poly_modulus_degree(POLY_MODULUS_DEGREE);
  vector<seal::Modulus> coeff_modulus;
  for (auto& prime : COEFF_MODULUS_FACTORIZATION) {
    coeff_modulus.push_back(seal::Modulus(prime));
  }
  params.set_coeff_modulus(coeff_modulus);
  params.set_plain_modulus(PLAIN_MODULUS);
  return params;
}

constexpr size_t SEAL_DB_COLUMNS = CEIL_DIV(MESSAGE_SIZE_BITS, PLAIN_BITS);

// CLIENT_DB_ROWS is the number of rows that the client thinks is in the
// database. this must be an upper bound on the actual database size. note that
// it is crucial for security that the client doesn't query the server for the
// actual database size and simply trusts that number — the server might lie! in
// particular, the server could tell one pair of users the size is 1M, everyone
// else that the size is 2M, and allocate indices in the first half only to that
// one pair. this would destroy security!
//
// We set it to 360K for now, limiting # edges in the social graph to 360K. This
// is to be under the GRPC message size limit of 4 MB — to increase this, use
// GRPC streaming instead. See
// https://jbrandhorst.com/post/grpc-binary-blob-stream/.
constexpr size_t CLIENT_DB_ROWS = 360'000;