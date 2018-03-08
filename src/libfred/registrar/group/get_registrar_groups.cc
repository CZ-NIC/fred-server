#include "src/libfred/db_settings.hh"
#include "src/libfred/registrar/group/get_registrar_groups.hh"
#include "src/libfred/registrar/group/registrar_group_type.hh"

namespace LibFred {
namespace Registrar {

std::vector<RegistrarGroup> GetRegistrarGroups::exec(OperationContext& _ctx)
{
    try
    {
        std::vector<RegistrarGroup> result;
        const Database::Result groups = _ctx.get_conn().exec(
                "SELECT id, short_name, cancelled FROM registrar_group "
                "ORDER BY cancelled, short_name");
        result.reserve(groups.size());
        for (Database::Result::Iterator it = groups.begin(); it != groups.end(); ++it)
        {
            Database::Row::Iterator col = (*it).begin();
            RegistrarGroup rg;
            rg.id = *col;
            rg.name = static_cast<std::string>(*(++col));
            rg.cancelled = *(++col);
            result.push_back(std::move(rg));
        }
        return result;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Failed to get registrar group due to an unknown exception");
        throw;
    }
}

} // namespace LibFred
} // namespace Registrar
