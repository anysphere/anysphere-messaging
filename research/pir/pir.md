We need a PIR algorithm. All PIR algorithms in use depend on a homomorphic encryption library, so we need that too.

## Homomorphic encryption libraries

1. [https://arxiv.org/pdf/2101.07078.pdf](https://arxiv.org/pdf/2101.07078.pdf) is a VERY GOOD guide.
   1. conclusion: we want SEAL, PALISADE or HElib
      1. [https://github.com/microsoft/seal](https://github.com/microsoft/seal)
         1. license: MIT ✅
         2. release year: 2015
         3. supported by microsoft
      2. [https://gitlab.com/palisade/palisade-development](https://gitlab.com/palisade/palisade-development)
         1. license: BSD-2 ✅
         2. release year: 2014
         3. supported by startupish
      3. [https://github.com/homenc/HElib](https://github.com/homenc/HElib)
         1. license: Apache v2 ✅
         2. release year: 2013
         3. supported by intel
2. conclusion: SEAL seems like the best choice.
3. there’s also [https://github.com/intel/hexl](https://github.com/intel/hexl)
   1. it seems like all of the three above are already using this
4. conclusion: all of them seem fine, so we should probably choose based on performance. ideally what we do is implement a wrapper interface a la [https://github.com/ParAlg/parlaylib/blob/8e46802e5e71a88fd950c53bcca1caf35aee9637/include/parlay/parallel.h](https://github.com/ParAlg/parlaylib/blob/8e46802e5e71a88fd950c53bcca1caf35aee9637/include/parlay/parallel.h)

## PIR algorithms

There doesn’t seem to be a production-ready PIR library yet.

- SealPIR
  - paper:
  - [https://github.com/microsoft/SealPIR](https://github.com/microsoft/SealPIR)
  - old code, needs to be completely rewritten, but can be used for inspiration
- OnionPIR
