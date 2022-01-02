#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

// these parameters are taken from https://github.com/ishtiyaque/FastPIR/blob/master/src/bfvparams.h
// TODO: optimize them

using std::array;
using std::size_t;

constexpr size_t POLY_MODULUS_DEGREE = 4096;

constexpr uint64_t PRIME_54 = 18'014'398'509'309'953ULL;

constexpr uint64_t PRIME_55 = 36'028'797'018'652'673ULL;

constexpr array<uint64_t, 2> COEFF_MODULUS_FACTORIZATION({PRIME_54, PRIME_55});

constexpr uint64_t PLAIN_BIT = 18;

// must be greater than 2^PLAIN_BIT
// must also be congruent to 1 modulo 2*POLY_MODULUS_DEGREE
// finally, must also be a prime
constexpr uint64_t PLAIN_MODULUS = 270'337;