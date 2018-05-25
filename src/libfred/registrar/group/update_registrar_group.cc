#include "src/libfred/db_settings.hh"
#include "src/libfred/registrar/group/exceptions.hh"
#include "src/libfred/registrar/group/update_registrar_group.hh"

#include <string>

namespace LibFred {
namespace Registrar {

void UpdateRegistrarGroup::exec(OperationContext& _ctx)
{
    try
    {
        if (group_name_.empty())
        {
            throw EmptyGroupName();
        }

        _ctx.get_conn().exec("LOCK TABLE registrar_group IN ACCESS EXCLUSIVE MODE");
        const Database::Result group_exists = _ctx.get_conn().exec_params(
                "SELECT id FROM registrar_group WHERE short_name=$1::text ",
                Database::query_param_list(group_name_));
        if (group_exists.size() > 0)
        {
            throw GroupExists();
        }

        _ctx.get_conn().exec_params(
                Database::ParamQuery(
                    "UPDATE registrar_group "
                    "SET short_name=")
                .param_text(group_name_)
                (" WHERE id=")
                .param_bigint(group_id_));
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Failed to update registrar group due to an unknown exception");
        throw;
    }
}

} // namespace Registrar
} // namespace LibFred
