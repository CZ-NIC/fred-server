#include "src/libfred/registrar/group/membership/info_group_membership_by_group.hh"
#include "src/libfred/db_settings.hh"

namespace LibFred {
namespace Registrar {

std::vector<GroupMembershipByGroup> InfoGroupMembershipByGroup::exec(OperationContext& _ctx)
{
    try
    {
        const Database::Result membership = _ctx.get_conn().exec_params(
                "SELECT id, registrar_id, member_from, member_until "
                "FROM registrar_group_map WHERE registrar_group_id=$1::bigint "
                "ORDER BY member_from DESC, id DESC",
                Database::query_param_list(group_id_));
        std::vector<GroupMembershipByGroup> result;
        result.reserve(membership.size());
        for (Database::Result::Iterator it = membership.begin(); it != membership.end(); ++it)
        {
            GroupMembershipByGroup tmp;
            tmp.membership_id = (*it)["id"];
            tmp.registrar_id = (*it)["registrar_id"];
            tmp.member_from = (*it)["member_from"];
            if (static_cast<std::string>((*it)["member_until"]).empty())
            {
                tmp.member_until = boost::gregorian::date(pos_infin);
            }
            else
            {
                tmp.member_until = (*it)["member_until"];
            }
            result.push_back(std::move(tmp));
        }
        return result;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Failed to get info group membership by group due to an unknown exception");
        throw;
    }
}

} // namespace Registrar
} // namespace LibFred
