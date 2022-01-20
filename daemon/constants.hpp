#pragma once
#include "client/client_lib/client_lib.hpp"

// we support up to 4 billion messages! that's a lot.
// (we use unsigned integers)
const size_t ACKING_BYTES = 4;
// the encryption takes a nonce + a mac
const size_t ENCRYPTED_ACKING_BYTES =
    ACKING_BYTES + CRYPTO_ABYTES + CRYPTO_NPUBBYTES;
// the maximum number of friends!
const size_t MAX_FRIENDS = MESSAGE_SIZE / ENCRYPTED_ACKING_BYTES;