#include "src/fredlib/object/get_id_of_registered.h"
#include "src/fredlib/db_settings.h"

#include <stdexcept>

namespace Fred {

const char* UnknownObject::what()const throw()
{
    return "unknown registry object type or handle";
}

namespace {

enum CaseSensitivity
{
    lowercase,
    uppercase
};

template <Object_Type::Enum object_type>
struct ObjectTypeTraits
{
    static const CaseSensitivity handle_case_sensitivity = uppercase;//contacts, nssets and keysets have uppercase handles
};

template < >
struct ObjectTypeTraits<Object_Type::domain>
{
    static const CaseSensitivity handle_case_sensitivity = lowercase;//only domains have lowercase handles
};

template <CaseSensitivity case_sensitivity>
const char* get_sql_conversion_function();

template < >
const char* get_sql_conversion_function<lowercase>() { return "LOWER"; }

template < >
const char* get_sql_conversion_function<uppercase>() { return "UPPER"; }

}//namespace Fred::{anonymous}

template < Object_Type::Enum object_type >
unsigned long long get_id_of_registered(
        OperationContext& ctx,
        const std::string& handle)
{
    static const std::string normalize_function =
            get_sql_conversion_function< ObjectTypeTraits<object_type>::handle_case_sensitivity >();
    static const std::string sql =
            "SELECT id "
            "FROM object_registry "
            "WHERE type=get_object_type_id($1::TEXT) AND "
                  "name=" + normalize_function + "($2::TEXT) AND "
                  "erdate IS NULL";
    static const std::string object_type_handle = Conversion::Enums::to_db_handle(object_type);
    const Database::Result dbres = ctx.get_conn().exec_params(
            sql,
            Database::query_param_list(object_type_handle)(handle));
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

template unsigned long long get_id_of_registered<Object_Type::contact>(OperationContext&, const std::string&);
template unsigned long long get_id_of_registered<Object_Type::nsset>(OperationContext&, const std::string&);
template unsigned long long get_id_of_registered<Object_Type::domain>(OperationContext&, const std::string&);
template unsigned long long get_id_of_registered<Object_Type::keyset>(OperationContext&, const std::string&);

}//namespace Fred
