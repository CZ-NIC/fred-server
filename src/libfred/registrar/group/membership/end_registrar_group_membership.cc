#include "src/libfred/registrar/group/membership/end_registrar_group_membership.hh"
#include "src/libfred/registrar/group/membership/exceptions.hh"

#include "src/libfred/db_settings.hh"

namespace LibFred {
namespace Registrar {

void EndRegistrarGroupMembership::exec(OperationContext& _ctx)
{
    try
    {
        const Database::Result membership = _ctx.get_conn().exec_params(
                "SELECT id FROM registrar_group_map "
                "WHERE registrar_id=$1::bigint "
                "AND registrar_group_id=$2::bigint "
                "AND (member_until IS NULL OR member_until >= NOW())",
                Database::query_param_list(registrar_id_)(group_id_));
        if (membership.size() < 1)
        {
            throw MembershipNotFound();
        }

        _ctx.get_conn().exec_params(
            "UPDATE registrar_group_map SET member_until=NOW()::date WHERE id=$1::bigint",
            Database::query_param_list(static_cast<unsigned long long>(membership[0][0])));
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Failed to end registrar group membership due to an unknown exception");
        throw;
    }
}

} // namespace Registrar
} // namespace LibFred

