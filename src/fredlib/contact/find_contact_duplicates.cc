#include "find_contact_duplicates.h"


namespace Fred {
namespace Contact {


FindAnyContactDuplicates::FindAnyContactDuplicates(const optional_string &_registrar_handle)
    : registrar_handle_(_registrar_handle)
{
}


std::set<std::string> FindAnyContactDuplicates::exec(Fred::OperationContext &_ctx)
{
    Database::QueryParams dup_params;
    std::string dup_sql = "SELECT unnest(dup_set)"
        " FROM (SELECT array_accum(oreg.name) as dup_set"
        " FROM object_registry oreg"
        " JOIN contact c ON c.id = oreg.id"
        " JOIN object o ON o.id = c.id"
        " JOIN registrar r ON r.id = o.clid"
        " GROUP BY"
        " trim(both ' ' from c.name),"
        " c.ssntype,"
        " trim(both ' ' from c.ssn),"
        " trim(both ' ' from c.vat),"
        " trim(both ' ' from c.telephone),"
        " trim(both ' ' from c.fax),"
        " trim(both ' ' from c.email),"
        " trim(both ' ' from c.notifyemail),"
        " trim(both ' ' from c.street1),"
        " trim(both ' ' from c.street2),"
        " trim(both ' ' from c.street3),"
        " trim(both ' ' from c.city),"
        " trim(both ' ' from c.stateorprovince),"
        " trim(both ' ' from c.postalcode),"
        " trim(both ' ' from c.country),"
        " o.clid";
    if (registrar_handle_.is_value_set()) {
        dup_sql += " WHERE r.handle = $1::varchar";
        dup_params.push_back(registrar_handle_.get_value());
    }
    dup_sql += " HAVING array_upper(array_accum(oreg.name), 1) > 1"
               " LIMIT 1) as dup_q";

    Database::Result dup_result = _ctx.get_conn().exec_params(dup_sql, dup_params);

    std::set<std::string> result;
    for (Database::Result::size_type i = 0; i < dup_result.size(); i++) {
        result.insert(static_cast<std::string>(dup_result[i][0]));
    }

    return result;
}



FindSpecificContactDuplicates::FindSpecificContactDuplicates(const std::string &_contact_handle)
    : contact_handle_(_contact_handle)
{
}


std::set<std::string> FindSpecificContactDuplicates::exec(Fred::OperationContext &_ctx)
{
    std::string dup_sql = "SELECT unnest(dup_set)"
        " FROM (SELECT array_accum(oreg.name) as dup_set"
        " FROM object_registry oreg"
        " JOIN contact c ON c.id = oreg.id"
        " JOIN object o ON o.id = c.id"
        " JOIN registrar r ON r.id = o.clid"
        " GROUP BY"
        " trim(both ' ' from c.name),"
        " c.ssntype,"
        " trim(both ' ' from c.ssn),"
        " trim(both ' ' from c.vat),"
        " trim(both ' ' from c.telephone),"
        " trim(both ' ' from c.fax),"
        " trim(both ' ' from c.email),"
        " trim(both ' ' from c.notifyemail),"
        " trim(both ' ' from c.street1),"
        " trim(both ' ' from c.street2),"
        " trim(both ' ' from c.street3),"
        " trim(both ' ' from c.city),"
        " trim(both ' ' from c.stateorprovince),"
        " trim(both ' ' from c.postalcode),"
        " trim(both ' ' from c.country),"
        " o.clid"
        " HAVING array_upper(array_accum(oreg.name), 1) > 1"
        " AND array[$1::varchar] <@ array_accum(oreg.name)) as dup_q";

    Database::Result dup_result = _ctx.get_conn().exec_params(
            dup_sql, Database::query_param_list(contact_handle_));

    std::set<std::string> result;
    for (Database::Result::size_type i = 0; i < dup_result.size(); i++) {
        result.insert(static_cast<std::string>(dup_result[i][0]));
    }

    return result;
}


}
}

