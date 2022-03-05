#include "nonprivate_pir.hpp"

NonPrivatePIR::NonPrivatePIR() {}

auto NonPrivatePIR::set_value(pir_index_t index, pir_value_t value) noexcept
    -> void {
  db[index] = value;
}

auto NonPrivatePIR::get_value_privately(pir_query_t pir_query) noexcept
    -> pir_answer_t {
  return NonPrivatePIRAnswer{db[pir_query.index]};
}

auto NonPrivatePIR::allocate() noexcept -> pir_index_t {
  auto new_index = db.size();
  db.push_back(pir_value_t{});
  return new_index;
}

auto NonPrivatePIR::query_from_string(const string& s) noexcept(false)
    -> pir_query_t {
  pir_query_t query;
  query.deserialize_from_string(s);
  return query;
}