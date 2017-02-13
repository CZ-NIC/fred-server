#include "src/fredlib/object/get_present_object_id.h"
#include "src/fredlib/db_settings.h"

#include <stdexcept>

namespace Fred
{

unsigned long long get_present_object_id(
        OperationContext& ctx,
        Object_Type::Enum object_type,
        const std::string& handle)
{
     const Database::Result dbres = ctx.get_conn().exec_params(
             "SELECT id "
             "FROM object_registry "
             "WHERE type=get_object_type_id($1::TEXT) AND "
                   "name=$2::TEXT AND "
                   "erdate IS NULL",
             Database::query_param_list(Conversion::Enums::to_db_handle(object_type))(handle));
     if (dbres.size() < 1)
     {
         throw UnknownObject();
     }
     if (1 < dbres.size())
     {
         throw std::runtime_error("too many objects for given handle and type");
     }
     return static_cast<unsigned long long>(dbres[0][0]);
}

}
