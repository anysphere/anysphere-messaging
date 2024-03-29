We need a PIR algorithm. All PIR algorithms in use depend on a homomorphic encryption library, so we need that too.

## Homomorphic encryption libraries

1. [https://arxiv.org/pdf/2101.07078.pdf](https://arxiv.org/pdf/2101.07078.pdf) is a VERY GOOD guide.
   1. conclusion: we want SEAL, PALISADE or HElib
      1. [https://github.com/microsoft/seal](https://github.com/microsoft/seal)
         1. license: MIT ✅
         2. release year: 2015
         3. supported by microsoft
         4. documentation: excellent. the example directory is amazing.
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

There doesn’t seem to be a production-ready PIR library yet. All of them are bad research code. But that's ok!

Ideally, once again here, we want to create something that can be swapped out. So we just need to define a good interface. How to deal with multiretrievals, though? They are important for performance.

- SealPIR:
  - paper: https://eprint.iacr.org/2017/1142.pdf
  - code: [https://github.com/microsoft/SealPIR](https://github.com/microsoft/SealPIR)
  - uses SEAL (duh)
  - what pung v2 uses, which means that it can be integrated with PBCs for multi-retrieval
  - uses BFV as a black box
  - BFV operations:
   - ADD(cipher, cipher)
   - MULT(plain, cipher)
   - SUB(cipher, k): converts enc(p(x)) to enc(p(x^k)). WEIRD.
  - request size: 64 KB
  - response size: (unclear, but at least 320 KB)
  - server runtime: ???
  - multi: ???
- OnionPIR:
  - paper: https://eprint.iacr.org/2021/1081.pdf
  - code: [https://github.com/mhmughees/Onion-PIR](https://github.com/mhmughees/Onion-PIR)
  - uses SEAL
  - not used by any anonymous communication
  - uses BFV as a black box
  - request size: ???
  - server runtime: ???
  - multi: ???
  - response size does not seem to be our bottleneck.
   - although.... having a small response size means we can send much more data which is very good.
- FastPIR:
  - paper: https://www.usenix.org/system/files/osdi21-ahmad.pdf
  - code: [https://github.com/ishtiyaque/fastpir](https://github.com/ishtiyaque/fastpir)
  - uses SEAL
  - what addra uses
  - uses BFV as a black box
  - BFV operations:
   - ADD(cipher, cipher)
   - MULT(plain, cipher)
   - ROWROTATE(cipher, i): rotates each row of the 2 x N/2 matrix by i. WEIRD.
   - COLROTATE(cipher): swaps the two rows. WEIRD.
  - feels like the easiest thing to implement!!!! i like it. maybe not ideal but good thing to start with at least.
  - request size: 32 * (# of users). explanation: n/N ciphertexts where n is # of mailboxes and N is BFV plaintext dimension. so smth like n/N * log(q) * 2N  = n * log(q) or something like that. wtf. this is too big..... or what is log(q)? maybe can only change friends once every day, or something like that... addra claims only a factor of 32 hmmm....
  - response size: 64 KB
  - server runtime: ???
  - multi: ???
- XPIR:
  - precursor to SealPIR it seems 
  - NO!

#### notes

- the onionPIR comparison in table 3 & 4 use 30KB entries, whereas the sealPIR table 9 uses 288byte entries. is there a tradeoff between onionPIR and sealPIR as the entries get bigger?
- for MVP: just use sealPIR. it has the least ugly code.
- for MVP: ignore multi-retrieval.
- "Thus, it may
    be beneficial to use Microsoft SEAL in secret-key mode whenever the public
    key is not truly needed." from `6_serialization.cc`
- be careful: "Currently Microsoft SEAL supports only little-endian systems." problem for distribution?

conclusion:
- fastPIR is the best. even though we need to investigate whether probabilistic batch codes still work with it. i believe they do.

## parameter choices

1. see http://homomorphicencryption.org/wp-content/uploads/2018/11/HomomorphicEncryptionStandardv1.1.pdf for information.

## questions

1. "To enable batching, we need to set the plain_modulus to be a prime number
    congruent to 1 modulo 2*poly_modulus_degree." in `2_encoders.cc`. WHY?
2. how does fastPIR do the fast rotation? maybe implement it itself?
3. do we need to care about SEAL_THROW_ON_TRANSPARENT_CIPHERTEXT? basically, seal doesn't like if you multiply a ciphertext by a plaintext that is identically zero, because the only preimage to the zero polynomial is the zero plaintext. but this doesn't actually reveal any information to the server. this doesn't actually affect security for us at all. yep.

## optimizations

1. can we trade off some of the fastPIR rotation-style stuff for faster computation time but larger response size? security is still fine and we can still decrypt. so literally a performance engineering question.
2. PBC for multi-retrieval
3. gzip the response / query? (seems like seal is already doing this...)
4. use modulus switching to reduce the ciphertext size? probably our parameters should be set competitively enough that this should never be possible though... see `seal_examples/3_levels.c`. seems like this is mostly used for ckks though which we will not use.