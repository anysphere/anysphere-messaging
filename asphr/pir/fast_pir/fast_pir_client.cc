
#include "fast_pir_client.hpp"

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