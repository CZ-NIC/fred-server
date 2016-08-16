#include "src/fredlib/object/get_present_object_id.h"
#include "src/fredlib/db_settings.h"

namespace Fred
{

unsigned long long get_present_object_id(
        OperationContext& ctx,
        Object_Type::Enum object_type,
        const std::string& handle)
{
     Database::Result id = ctx.get_conn().exec_params(
             "SELECT id "
             "FROM object_registry "
             "WHERE type = get_object_type_id($1) "
               "AND name = $2::text "
               "AND erdate IS NULL ",
             Database::query_param_list(Conversion::Enums::to_db_handle(object_type))(handle));
     if (id.size() < 1)
     {
         throw UnknownObject();
     }
     return id[0][0];
}

}
