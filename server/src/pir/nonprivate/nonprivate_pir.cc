#include "nonprivate_pir.h"

NonPrivatePIR::NonPrivatePIR() {}

auto NonPrivatePIR::set_value(pir_index_t index, pir_value_t value) -> void
{
  db[index] = value;
}

auto NonPrivatePIR::get_value_privately(pir_query_t pir_query) -> pir_answer_t
{
  return NonPrivatePIRAnswer{db[pir_query.index]};
}

auto NonPrivatePIR::allocate() -> pir_index_t
{
  auto new_index = db.size();
  db.push_back(pir_value_t{});
  return new_index;
}