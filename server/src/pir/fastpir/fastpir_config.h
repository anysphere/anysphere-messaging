#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <vector>

// these parameters are taken from https://github.com/ishtiyaque/FastPIR/blob/master/src/bfvparams.h
// TODO: optimize them

#define CEIL_DIV(a, b) (((a) + (b)-1) / (b))

using std::array;
using std::size_t;
using std::vector;

constexpr size_t POLY_MODULUS_DEGREE = 4096;

constexpr uint64_t PRIME_54 = 18'014'398'509'309'953ULL;

constexpr uint64_t PRIME_55 = 36'028'797'018'652'673ULL;

constexpr array<uint64_t, 2> COEFF_MODULUS_FACTORIZATION({PRIME_54, PRIME_55});

// number of bits we store per coefficient. we store a power of 2 because performing modulo mod 2 is much much faster, when encoding
constexpr uint64_t PLAIN_BITS = 18;

// must be greater than 2^PLAIN_BITS
// must also be congruent to 1 modulo 2*POLY_MODULUS_DEGREE
// finally, must also be a prime
constexpr uint64_t PLAIN_MODULUS = 270'337;

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