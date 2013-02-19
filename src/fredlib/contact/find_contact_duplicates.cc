#include "find_contact_duplicates.h"
#include <boost/algorithm/string/join.hpp>


namespace Fred {
namespace Contact {



FindAnyContactDuplicates::FindAnyContactDuplicates()
{
}


FindAnyContactDuplicates& FindAnyContactDuplicates::set_registrar(
        const optional_string &_registrar_handle)
{
    registrar_handle_ = _registrar_handle;
    return *this;
}


FindAnyContactDuplicates& FindAnyContactDuplicates::set_exclude_contacts(
        const std::set<std::string> &_exclude_contacts)
{
    exclude_contacts_ = _exclude_contacts;
    return *this;
}


std::set<std::string> FindAnyContactDuplicates::exec(Fred::OperationContext &_ctx)
{
    Database::QueryParams dup_params;
    std::stringstream dup_sql;
    dup_sql << "SELECT unnest(dup_set)"
        " FROM (SELECT array_accum(oreg.name) as dup_set"
        " FROM object_registry oreg"
        " JOIN contact c ON c.id = oreg.id"
        " JOIN object o ON o.id = c.id"
        " JOIN registrar r ON r.id = o.clid";

    if (registrar_handle_.is_value_set()) {
        dup_params.push_back(registrar_handle_.get_value());
        dup_sql << " WHERE r.handle = $" << dup_params.size() << "::varchar";
    }

    dup_sql << " GROUP BY"
        " trim(both ' ' from c.name),"
        " trim(both ' ' from c.organization),"
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
        " o.clid,"
        " c.disclosename,"
        " c.discloseorganization,"
        " c.discloseaddress,"
        " c.disclosetelephone,"
        " c.disclosefax,"
        " c.discloseemail,"
        " c.disclosevat,"
        " c.discloseident,"
        " c.disclosenotifyemail"
        " HAVING array_upper(array_accum(oreg.name), 1) > 1";
    if (!exclude_contacts_.empty()) {
        std::vector<std::string> array_params;
        for (std::set<std::string>::const_iterator i = exclude_contacts_.begin();
                i != exclude_contacts_.end(); ++i)
        {
            dup_params.push_back(*i);
            array_params.push_back("$" + boost::lexical_cast<std::string>(dup_params.size()));
        }

        dup_sql << " AND NOT (array_accum(oreg.name)"
                << " && array[" << boost::algorithm::join(array_params, ", ") << "]::varchar[])";
    }

    dup_sql << " LIMIT 1) as dup_q";

    Database::Result dup_result = _ctx.get_conn().exec_params(dup_sql.str(), dup_params);

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
    std::string dup_sql = "SELECT cother.handle FROM"
        "(SELECT coreg2.name AS handle, co2.clid, c2.*"
        " FROM object_registry coreg2"
        " JOIN contact c2 ON c2.id = coreg2.id AND coreg2.erdate IS NULL"
        " JOIN object co2 ON co2.id = c2.id) cother"
        " JOIN"
        " (SELECT o.clid, c.* FROM object_registry coreg"
        " JOIN contact c ON c.id = coreg.id AND coreg.erdate IS NULL"
        " JOIN object o ON o.id = c.id"
        " WHERE coreg.name = $1::varchar) csearch"
        " ON"
        " trim(both ' ' from csearch.name) IS NOT DISTINCT FROM trim(both ' ' from cother.name)"
        " AND trim(both ' ' from csearch.organization) IS NOT DISTINCT FROM trim(both ' ' from cother.organization)"
        " AND csearch.ssntype IS NOT DISTINCT FROM cother.ssntype"
        " AND trim(both ' ' from csearch.ssn) IS NOT DISTINCT FROM trim(both ' ' from cother.ssn)"
        " AND trim(both ' ' from csearch.vat) IS NOT DISTINCT FROM trim(both ' ' from cother.vat)"
        " AND trim(both ' ' from csearch.telephone) IS NOT DISTINCT FROM  trim(both ' ' from cother.telephone)"
        " AND trim(both ' ' from csearch.fax) IS NOT DISTINCT FROM  trim(both ' ' from cother.fax)"
        " AND trim(both ' ' from csearch.email) IS NOT DISTINCT FROM  trim(both ' ' from cother.email)"
        " AND trim(both ' ' from csearch.notifyemail) IS NOT DISTINCT FROM trim(both ' ' from cother.notifyemail)"
        " AND trim(both ' ' from csearch.street1) IS NOT DISTINCT FROM trim(both ' ' from cother.street1)"
        " AND trim(both ' ' from csearch.street2) IS NOT DISTINCT FROM trim(both ' ' from cother.street2)"
        " AND trim(both ' ' from csearch.street3) IS NOT DISTINCT FROM trim(both ' ' from cother.street3)"
        " AND trim(both ' ' from csearch.city) IS NOT DISTINCT FROM trim(both ' ' from cother.city)"
        " AND trim(both ' ' from csearch.stateorprovince) IS NOT DISTINCT FROM trim(both ' ' from cother.stateorprovince)"
        " AND trim(both ' ' from csearch.postalcode) IS NOT DISTINCT FROM trim(both ' ' from cother.postalcode)"
        " AND trim(both ' ' from csearch.country) IS NOT DISTINCT FROM trim(both ' ' from cother.country)"
        " AND csearch.disclosename = cother.disclosename"
        " AND csearch.discloseorganization = cother.discloseorganization"
        " AND csearch.discloseaddress = cother.discloseaddress"
        " AND csearch.disclosetelephone = cother.disclosetelephone"
        " AND csearch.disclosefax = cother.disclosefax"
        " AND csearch.discloseemail = cother.discloseemail"
        " AND csearch.disclosevat = cother.disclosevat"
        " AND csearch.discloseident = cother.discloseident"
        " AND csearch.disclosenotifyemail = cother.disclosenotifyemail"
        " AND csearch.clid = cother.clid";

    Database::Result dup_result = _ctx.get_conn().exec_params(
            dup_sql, Database::query_param_list(contact_handle_));

    std::set<std::string> result;

    if (dup_result.size() == 1 && static_cast<std::string>(dup_result[0][0]) == contact_handle_) {
        return result;
    }

    for (Database::Result::size_type i = 0; i < dup_result.size(); i++) {
        result.insert(static_cast<std::string>(dup_result[i][0]));
    }
    return result;
}


}
}

