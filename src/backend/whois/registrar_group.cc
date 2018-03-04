#include "src/backend/whois/registrar_group.hh"


namespace Fred {
namespace Backend {
namespace Whois {
    std::map<std::string, std::vector<std::string> > get_registrar_groups(
        LibFred::OperationContext& ctx)
    {
        Database::Result registrar_groups_res = ctx.get_conn().exec(
        "SELECT rg.short_name AS registrar_group, "
                "r.handle AS registrar_handle "
            "FROM registrar_group rg "
                "JOIN registrar_group_map rgm ON rg.id = rgm.registrar_group_id "
                    "AND rg.cancelled IS NULL "
                "JOIN registrar r ON r.id = rgm.registrar_id "
                    "AND rgm.member_from <= CURRENT_DATE "
                    "AND (rgm.member_until IS NULL "
                        "OR (rgm.member_until >= CURRENT_DATE "
                        "AND rgm.member_from <> rgm.member_until))");

        std::map<std::string, std::vector<std::string> > ret;
        for(unsigned long long i = 0; i < registrar_groups_res.size(); ++i)
        {
            ret[static_cast<std::string>(registrar_groups_res[i]["registrar_group"])].push_back(
                static_cast<std::string>(registrar_groups_res[i]["registrar_handle"]));
        }
        return ret;
    }
} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred
