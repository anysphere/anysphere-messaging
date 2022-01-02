#include "nonprivate_pir.h"

NonPrivatePIR::NonPrivatePIR() {}

auto NonPrivatePIR::set_value(pir_index_t index, pir_value_t value) -> void {
  db[index] = value;
}

auto NonPrivatePIR::get_value_privately(pir_query_t pir_query) -> pir_answer_t { 
  return NonPrivatePIRAnswer{db[pir_query.index]};
}