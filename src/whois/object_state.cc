#include "src/whois/object_state.h"


namespace Whois {
    std::string get_object_state_name_by_state_id(
        unsigned long long state_id,
        Fred::OperationContext& ctx)
    {
        Database::Result state_name_res = ctx.get_conn().exec_params(
            "SELECT name FROM enum_object_states WHERE id = $1::bigint",
                Database::query_param_list(state_id));

        if(state_name_res.size() != 1) throw std::runtime_error("invalid state_id");

        return static_cast<std::string>(state_name_res[0]["name"]);

    }
}
