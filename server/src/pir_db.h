#pragma once

// #include <array>
#include <cstddef>
#include <vector>
#include <concepts>

using std::array;
using std::byte;
using std::vector;
using std::string;

class pir_query_type_base {
  public:

  virtual ~pir_query_type_base() {};

  virtual auto serialize_to_string() noexcept -> std::string = 0;
  // returns: whether deserialization was successful
  virtual auto deserialize_from_string(const string &) noexcept -> bool = 0;
};

class pir_answer_type_base {
  public:

  virtual ~pir_answer_type_base() {};

  virtual auto serialize_to_string() -> string = 0;
  // returns: whether deserialization was successful
  virtual auto deserialize_from_string(string& ) noexcept -> string = 0;
};

// TODO: this should be templated by a single thing — a PIR implementation,
// and each of these types should be PIR::index_type, PIR::value_type, PIR::query_type, etc etc
template <typename index_type, 
  typename value_type, 
  class pir_query_type,
  class pir_answer_type>
  requires std::is_base_of_v<pir_query_type_base, pir_query_type> &&
  std::is_base_of_v<pir_answer_type_base, pir_answer_type>
class PirDB {
  // static_assert(std::derived_from<pir_answer_type, pir_answer_type_base> == true,
  //   "pir_answer_type must be derived from pir_answer_type_base");
  // static_assert(std::derived_from<pir_query_type, pir_query_type_base> == true,
  //   "pir_query_type must be derived from pir_query_type_base");

  // TODO: this needs to be threadsafe
 public:
  PirDB() {}

  void set_value(index_type index, value_type value);
  // TODO: does this need to return an error?
  pir_answer_type get_value_privately(pir_query_type pir_query);
};