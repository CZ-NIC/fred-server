#include "src/whois/registrar_group.h"


namespace Whois {
    std::vector<std::string> get_managed_zone_list(
        Fred::OperationContext& ctx)
    {
        Database::Result zone_list_res = ctx.get_conn().exec(
        "SELECT fqdn FROM zone ORDER BY fqdn");

        std::vector<std::string> ret;
        for(unsigned long long i = 0; i < zone_list_res.size(); ++i)
        {
            ret.push_back(static_cast<std::string>(zone_list_res[i]["fqdn"]));
        }
        return ret;
    }
}
