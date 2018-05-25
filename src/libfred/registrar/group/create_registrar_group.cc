#include "src/libfred/db_settings.hh"
#include "src/libfred/registrar/group/create_registrar_group.hh"
#include "src/libfred/registrar/group/exceptions.hh"

#include <string>

namespace LibFred {
namespace Registrar {

unsigned long long CreateRegistrarGroup::exec(OperationContext& _ctx)
{
    try
    {
        if (group_name_.empty())
        {
            throw EmptyGroupName();
        }

        _ctx.get_conn().exec("LOCK TABLE registrar_group IN ACCESS EXCLUSIVE MODE");
        const Database::Result grp_exists = _ctx.get_conn().exec_params(
                "SELECT id FROM registrar_group WHERE short_name=$1::text ",
                Database::query_param_list(group_name_));
        if (grp_exists.size() > 0)
        {
            throw GroupExists();
        }

        return _ctx.get_conn().exec_params(
                Database::ParamQuery(
                    "INSERT INTO registrar_group (short_name) "
                    "VALUES (")
                .param_text(group_name_)
                (") returning id")
                )[0][0];
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Failed to create a registrar group due to an unknown exception");
        throw;
    }
}

} // namespace Registrar
} // namespace LibFred
