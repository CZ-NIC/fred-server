#include "src/backend/whois/zone_list.hh"


namespace Fred {
namespace Backend {
namespace Whois {
    std::vector<std::string> get_managed_zone_list(
        LibFred::OperationContext& ctx)
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
} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred
