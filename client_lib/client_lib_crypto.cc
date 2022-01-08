#include <sodium.h>

extern constexpr size_t CRYPTO_ABYTES =
    crypto_aead_xchacha20poly1305_ietf_ABYTES;
extern constexpr size_t CRYPTO_NPUBBYTES =
    crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;