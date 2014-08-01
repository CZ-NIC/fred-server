#include "util/optional_value.h"
#include "find_contact_duplicates.h"

#include <boost/algorithm/string/join.hpp>


namespace Fred {
namespace Contact {

FindContactDuplicates::FindContactDuplicates()
{
}

FindContactDuplicates& FindContactDuplicates::set_registrar(const Optional<std::string>& registrar_handle)
{
    registrar_handle_ = registrar_handle;
    return *this;
}

FindContactDuplicates& FindContactDuplicates::set_exclude_contacts(const std::set<std::string> &exclude_contacts)
{
    exclude_contacts_ = exclude_contacts;
    return *this;
}

//find set of duplicate contacts that contains given specific_contact_handle
FindContactDuplicates& FindContactDuplicates::set_specific_contact(const std::string& specific_contact_handle)
{
    specific_contact_handle_ = specific_contact_handle;
    return *this;
}

std::set<std::string> FindContactDuplicates::exec(Fred::OperationContext &ctx)
{
    if(!specific_contact_handle_.isset())
    {
        //cursor WITHOUT HOLD released with CLOSE or at the end of the transaction
        ctx.get_conn().exec("DECLARE get_contacts_by_name CURSOR FOR SELECT unnest(array_accum(c.id)) FROM contact c "
            " GROUP BY trim(both ' ' from COALESCE(c.name,'')) HAVING array_upper(array_accum(c.id), 1) > 1 ");
    }

    Database::Result contact_handle_result;

    do
    {
        Database::Result duplicate_suspect_contact_id_result;

        if(!specific_contact_handle_.isset())
        {
            duplicate_suspect_contact_id_result = ctx.get_conn().exec("FETCH NEXT FROM get_contacts_by_name");
            if(duplicate_suspect_contact_id_result.size() == 0) break;
        }

        Database::query_param_list contact_handle_query_params;

        std::string contact_handle_query =
        "SELECT unnest(tmp.contact_names) FROM ( "
        " SELECT array_accum(oreg_src.name::text) as contact_names "
        " FROM (object_registry oreg_src "
        " JOIN contact c_src ON c_src.id = oreg_src.id AND oreg_src.erdate IS NULL "
        " JOIN object o_src ON o_src.id = oreg_src.id) "
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

        if(specific_contact_handle_.isset())
        {
            //set specific contact handle
            contact_handle_query += " AND oreg_dst.name = $";
            contact_handle_query += contact_handle_query_params.add(specific_contact_handle_.get_value());
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
        " AND (transfer_src_os.id IS NULL) = (transfer_dst_os.id IS NULL) "
        " )) as tmp ";

        if(!specific_contact_handle_.isset() && !exclude_contacts_.empty())
        {
            //optional exclusion of contact sets containing given contacts
            contact_handle_query += "WHERE NOT (tmp.contact_names && array[";

            std::vector<std::string> array_params;
            for (std::set<std::string>::const_iterator i = exclude_contacts_.begin();
                    i != exclude_contacts_.end(); ++i)
            {
                array_params.push_back("$" + contact_handle_query_params.add(*i) + "::text");
            }
            contact_handle_query += boost::algorithm::join(array_params, ", ");
            contact_handle_query +="])";
        }

        contact_handle_result = ctx.get_conn().exec_params(contact_handle_query , contact_handle_query_params);

        if(contact_handle_result.size() > 0) break;

    }
    while(true);

    if(!specific_contact_handle_.isset())
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

}
}

