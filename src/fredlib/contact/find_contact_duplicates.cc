#include "util/optional_value.h"
#include "find_contact_duplicates.h"
#include <boost/algorithm/string/join.hpp>


namespace Fred {
namespace Contact {

FindContactDuplicates::FindContactDuplicates()
{
}

FindContactDuplicates& FindContactDuplicates::set_registrar(const std::string &registrar_handle)
{
    registrar_handle_ = registrar_handle;
    return *this;
}

FindContactDuplicates& FindContactDuplicates::set_exclude_contacts(const std::set<std::string> &exclude_contacts)
{
    exclude_contacts_ = exclude_contacts;
    return *this;
}

FindContactDuplicates& FindContactDuplicates::set_dest_contact(const std::string& dest_contact_handle)
{
    dest_contact_handle_ = dest_contact_handle;
    return *this;
}

std::set<std::string> FindContactDuplicates::exec(Fred::OperationContext &ctx)
{

    if(!dest_contact_handle_.isset())
    {
        //cursor WITHOUT HOLD released with CLOSE or at the end of the transaction
        ctx.get_conn().exec("DECLARE get_contacts_by_name CURSOR FOR SELECT unnest(array_accum(c.id)) FROM contact c "
            " GROUP BY trim(both ' ' from COALESCE(c.name,'')) HAVING array_upper(array_accum(c.id), 1) > 1 ");
    }

    Database::Result contact_handle_result;

    do
    {
        Database::Result duplicate_suspect_contact_id_result;

        if(!dest_contact_handle_.isset())
        {
            duplicate_suspect_contact_id_result = ctx.get_conn().exec("FETCH NEXT FROM get_contacts_by_name");
            if(duplicate_suspect_contact_id_result.size() == 0) break;
        }

        Database::query_param_list contact_handle_query_params;

        std::string contact_handle_query =
        "SELECT oreg_src.name::text "

        " FROM (object_registry oreg_src "
        " JOIN contact c_src ON c_src.id = oreg_src.id AND oreg_src.erdate IS NULL ";

        //exclude given contacts
        for(std::set<std::string>::const_iterator ci = exclude_contacts_.begin(); ci != exclude_contacts_.end(); ++ci)
        {
            contact_handle_query += " AND oreg_src.name != $";
            contact_handle_query += contact_handle_query_params.add(std::string(*ci));
            contact_handle_query += "::text ";
        }

        contact_handle_query += " JOIN object o_src ON o_src.id = oreg_src.id) "

        " JOIN (object_registry oreg_dst "
        " JOIN object o_dst ON o_dst.id = oreg_dst.id ";

        //specific registrar restriction
        if(registrar_handle_.isset())
        {
            contact_handle_query += " AND o_dst.clid = (SELECT id FROM registrar WHERE handle = $";
            contact_handle_query += contact_handle_query_params.add(registrar_handle_.get_value());
            contact_handle_query += "::text) ";
        }

        contact_handle_query += " JOIN contact c_dst ON c_dst.id = oreg_dst.id  AND oreg_dst.erdate IS NULL ";

        if(dest_contact_handle_.isset())
        {
            //set winner contact handle
            contact_handle_query += " AND oreg_dst.name = $";
            contact_handle_query += contact_handle_query_params.add(dest_contact_handle_.get_value());
            contact_handle_query += "::text ";
        }
        else
        {
            //duplicate suspect contact from group by contact name cursor
            contact_handle_query += " AND c_dst.id = $";
            contact_handle_query += contact_handle_query_params.add(static_cast<unsigned long long>(duplicate_suspect_contact_id_result[0][0]));
            contact_handle_query += "::bigint ";
        }
        contact_handle_query += " ) ON TRUE "

        " LEFT JOIN object_state forbidden_src_os ON forbidden_src_os.object_id = c_src.id "
        " AND forbidden_src_os.state_id IN (SELECT eos.id FROM enum_object_states eos WHERE eos.name = 'mojeidContact'::text "
        " OR eos.name = 'serverDeleteProhibited'::text OR eos.name = 'serverBlocked'::text) "//forbidden states of src contact
        " AND forbidden_src_os.valid_from <= CURRENT_TIMESTAMP AND (forbidden_src_os.valid_to is null OR forbidden_src_os.valid_to > CURRENT_TIMESTAMP) "

        " LEFT JOIN object_state forbidden_dst_os ON forbidden_dst_os.object_id = c_dst.id "
        " AND forbidden_dst_os.state_id IN (SELECT eos.id FROM enum_object_states eos WHERE eos.name = 'serverBlocked'::text) "//forbidden state of dst contact
        " AND forbidden_dst_os.valid_from <= CURRENT_TIMESTAMP AND (forbidden_dst_os.valid_to is null OR forbidden_dst_os.valid_to > CURRENT_TIMESTAMP) "

        " LEFT JOIN object_state update_src_os ON update_src_os.object_id = c_src.id AND update_src_os.state_id IN "
        " (SELECT eos.id FROM enum_object_states eos WHERE eos.name = 'serverUpdateProhibited'::text) "//prohibited update of src contact
        " AND update_src_os.valid_from <= CURRENT_TIMESTAMP AND (update_src_os.valid_to IS NULL OR update_src_os.valid_to > CURRENT_TIMESTAMP) "

        " LEFT JOIN object_state transfer_src_os ON transfer_src_os.object_id = c_src.id AND transfer_src_os.state_id IN "
        " (SELECT eos.id FROM enum_object_states eos WHERE eos.name = 'serverTransferProhibited'::text) "//prohibited transfer of src contact
        " AND transfer_src_os.valid_from <= CURRENT_TIMESTAMP AND (transfer_src_os.valid_to IS NULL OR transfer_src_os.valid_to > CURRENT_TIMESTAMP) "

        " LEFT JOIN object_state update_dst_os ON update_dst_os.object_id = c_src.id AND update_dst_os.state_id IN "
        " (SELECT eos.id FROM enum_object_states eos WHERE eos.name = 'serverUpdateProhibited'::text) "//prohibited update of dst contact
        " AND update_dst_os.valid_from <= CURRENT_TIMESTAMP AND (update_dst_os.valid_to IS NULL OR update_dst_os.valid_to > CURRENT_TIMESTAMP) "

        " LEFT JOIN object_state transfer_dst_os ON transfer_dst_os.object_id = c_src.id AND transfer_dst_os.state_id IN "
        " (SELECT eos.id FROM enum_object_states eos WHERE eos.name = 'serverTransferProhibited'::text) "//prohibited transfer of dst contact
        " AND transfer_dst_os.valid_from <= CURRENT_TIMESTAMP AND (transfer_dst_os.valid_to IS NULL OR transfer_dst_os.valid_to > CURRENT_TIMESTAMP) "

        " WHERE "
        " ( "
        //the same
        " (trim(both ' ' from COALESCE(c_src.name,'')) = trim(both ' ' from COALESCE(c_dst.name,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.organization,'')) = trim(both ' ' from COALESCE(c_dst.organization,''))) AND "
        " (COALESCE(c_src.ssntype,0) = COALESCE(c_dst.ssntype,0)) AND "
        " (trim(both ' ' from COALESCE(c_src.ssn,'')) = trim(both ' ' from COALESCE(c_dst.ssn,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.vat,'')) = trim(both ' ' from COALESCE(c_dst.vat,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.telephone,'')) = trim(both ' ' from COALESCE(c_dst.telephone,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.fax,'')) = trim(both ' ' from COALESCE(c_dst.fax,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.fax,'')) = trim(both ' ' from COALESCE(c_dst.fax,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.email,'')) = trim(both ' ' from COALESCE(c_dst.email,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.notifyemail,'')) = trim(both ' ' from COALESCE(c_dst.notifyemail,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.street1,'')) = trim(both ' ' from COALESCE(c_dst.street1,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.street2,'')) = trim(both ' ' from COALESCE(c_dst.street2,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.street3,'')) = trim(both ' ' from COALESCE(c_dst.street3,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.city,'')) = trim(both ' ' from COALESCE(c_dst.city,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.postalcode,'')) = trim(both ' ' from COALESCE(c_dst.postalcode,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.stateorprovince,'')) = trim(both ' ' from COALESCE(c_dst.stateorprovince,''))) AND "
        " (trim(both ' ' from COALESCE(c_src.country,'')) = trim(both ' ' from COALESCE(c_dst.country,''))) AND "

        " c_src.disclosename = c_dst.disclosename AND "
        " c_src.discloseorganization = c_dst.discloseorganization AND "
        " c_src.discloseaddress = c_dst.discloseaddress AND "
        " c_src.disclosetelephone = c_dst.disclosetelephone AND "
        " c_src.disclosefax = c_dst.disclosefax AND "
        " c_src.discloseemail = c_dst.discloseemail AND "
        " c_src.disclosevat = c_dst.disclosevat AND "
        " c_src.discloseident = c_dst.discloseident AND "
        " c_src.disclosenotifyemail = c_dst.disclosenotifyemail AND "

        " o_src.clid = o_dst.clid "

        " AND forbidden_src_os.id IS NULL AND forbidden_dst_os.id IS NULL "
        " AND (update_src_os.id IS NULL) = (update_dst_os.id IS NULL) "
        " AND (transfer_src_os.id IS NULL) = (transfer_dst_os.id IS NULL) ";

        //exclude winner contact from set of duplicates
        if(dest_contact_handle_.isset())
        {
            contact_handle_query += " AND oreg_src.name != oreg_dst.name ";
        }

        contact_handle_query += " ) ";

        contact_handle_result = ctx.get_conn().exec_params(contact_handle_query , contact_handle_query_params);
        if(contact_handle_result.size() > 0) break;

    }
    while(true);

    if(!dest_contact_handle_.isset())
    {
        //cursor WITHOUT HOLD released with CLOSE or at the end of the transaction
        ctx.get_conn().exec("CLOSE get_contacts_by_name");
    }

    std::set<std::string> result;
    for (Database::Result::size_type i = 0; i < contact_handle_result.size(); i++)
    {
        result.insert(static_cast<std::string>(contact_handle_result[i][0]));
    }
    return result;
}



FindAnyContactDuplicates::FindAnyContactDuplicates()
{
}


FindAnyContactDuplicates& FindAnyContactDuplicates::set_registrar(
        const Optional<std::string> &_registrar_handle)
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
        " JOIN registrar r ON r.id = o.clid"

        " LEFT JOIN object_state forbidden_os ON forbidden_os.object_id = o.id AND forbidden_os.state_id IN "
            " (SELECT eos.id FROM enum_object_states eos WHERE eos.name = 'mojeidContact'::text "
            " OR eos.name = 'serverDeleteProhibited'::text OR eos.name = 'serverBlocked'::text) "//forbidden states of merged contact
            " AND forbidden_os.valid_from <= CURRENT_TIMESTAMP AND (forbidden_os.valid_to IS NULL OR forbidden_os.valid_to > CURRENT_TIMESTAMP) "

        " LEFT JOIN object_state update_os ON update_os.object_id = o.id AND update_os.state_id IN "
            " (SELECT eos.id FROM enum_object_states eos WHERE eos.name = 'serverUpdateProhibited'::text) "//prohibited update of merged contact
            " AND update_os.valid_from <= CURRENT_TIMESTAMP AND (update_os.valid_to IS NULL OR update_os.valid_to > CURRENT_TIMESTAMP) "

        " LEFT JOIN object_state transfer_os ON transfer_os.object_id = o.id AND transfer_os.state_id IN "
            " (SELECT eos.id FROM enum_object_states eos WHERE eos.name = 'serverTransferProhibited'::text) "//prohibited transfer of merged contact
            " AND transfer_os.valid_from <= CURRENT_TIMESTAMP AND (transfer_os.valid_to IS NULL OR transfer_os.valid_to > CURRENT_TIMESTAMP) ";

    if (registrar_handle_.isset()) {
        dup_params.push_back(registrar_handle_.get_value());
        dup_sql << " AND r.handle = $" << dup_params.size() << "::varchar";
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
        " c.disclosenotifyemail,"
        " update_os.id IS NOT NULL,"
        " transfer_os.id IS NOT NULL,"
        " forbidden_os.id IS NULL"
        " HAVING array_upper(array_accum(oreg.name), 1) > 1 "
        " AND forbidden_os.id IS NULL";
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

