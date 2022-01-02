#include "fastpir.h"
#include "fastpir_config.h"

class FastPIRClient
{
public:
    using pir_query_t = FastPIRQuery;
    using pir_answer_t = FastPIRAnswer;

    FastPIRClient() : sc(create_context_params()) {}

    auto query(pir_index_t index) -> pir_query_t
    {
        // TOOD: implement this
        return pir_query_t{};
    }

    auto decode(pir_answer_t answer) -> pir_value_t
    {
        // TOOD: implement this
        return pir_value_t{};
    }

private:
    seal::SEALContext sc;
}