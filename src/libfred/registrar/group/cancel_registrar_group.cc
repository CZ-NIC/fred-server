#include "src/libfred/db_settings.hh"
#include "src/libfred/registrar/group/cancel_registrar_group.hh"
#include "src/libfred/registrar/group/exceptions.hh"

namespace LibFred {
namespace Registrar {

void CancelRegistrarGroup::exec(OperationContext& _ctx)
{
    try
    {
        const Database::Result already_cancelled = _ctx.get_conn().exec_params(
                "SELECT cancelled FROM registrar_group WHERE id=$1::bigint AND cancelled IS NOT NULL FOR UPDATE",
                Database::query_param_list(group_id_));
        if (already_cancelled.size() > 0)
        {
            throw AlreadyCancelled();
        }

        _ctx.get_conn().exec("LOCK TABLE registrar_group_map IN ACCESS EXCLUSIVE MODE");
        const Database::Result nonempty_group = _ctx.get_conn().exec_params(
                "SELECT id "
                "FROM registrar_group_map "
                "WHERE registrar_group_id = $1::bigint "
                "AND member_from <= CURRENT_DATE "
                "AND (member_until IS NULL "
                "OR (member_until >= CURRENT_DATE "
                "AND member_from <> member_until)) ",
                Database::query_param_list(group_id_));
        if (nonempty_group.size() > 0)
        {
            throw NonemptyGroupDelete();
        }

        _ctx.get_conn().exec_params(
                Database::ParamQuery(
                    "UPDATE registrar_group "
                    "SET cancelled=now() "
                    "WHERE id=")
                .param_bigint(group_id_));
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Failed to cancel registrar group due to an unknown exception");
        throw;
    }
}

} // namespace Registrar
} // namespace LibFred
