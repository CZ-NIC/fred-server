#include "src/libfred/registrar/group/membership/info_group_membership_by_registrar.hh"
#include "src/libfred/db_settings.hh"

namespace LibFred {
namespace Registrar {

std::vector<GroupMembershipByRegistrar> InfoGroupMembershipByRegistrar::exec(OperationContext& ctx)
{
    try
    {
        Database::Result membership = ctx.get_conn().exec_params(
              "SELECT id, registrar_group_id, member_from, member_until "
              "FROM registrar_group_map WHERE registrar_id=$1::bigint "
              "ORDER BY member_from DESC, id DESC",
              Database::query_param_list(registrar_id));
        std::vector<GroupMembershipByRegistrar> result;
        result.reserve(membership.size());
        for (Database::Result::Iterator it = membership.begin(); it != membership.end(); ++it)
        {
            GroupMembershipByRegistrar tmp;
            tmp.membership_id = (*it)["id"];
            tmp.group_id = (*it)["registrar_group_id"];
            tmp.member_from = (*it)["member_from"];
            if (static_cast<std::string>((*it)["member_until"]).empty())
            {
                tmp.member_until = boost::gregorian::date(pos_infin);
            }
            else
            {
                tmp.member_until = (*it)["member_until"];
            }
            result.push_back(tmp);
        }
        return result;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Unknown error");
        throw;
    }
}

} // namespace Registrar
} // namespace LibFred
