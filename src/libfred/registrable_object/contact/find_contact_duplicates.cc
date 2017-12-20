#include "src/util/optional_value.hh"
#include "src/libfred/registrable_object/contact/find_contact_duplicates.hh"

#include <boost/algorithm/string/join.hpp>

#include <string>


namespace LibFred {
namespace Contact {

FindContactDuplicates::FindContactDuplicates()
{
}

FindContactDuplicates& FindContactDuplicates::set_registrar(const Optional<std::string>& _registrar_handle)
{
    registrar_handle_ = _registrar_handle;
    return *this;
}

FindContactDuplicates& FindContactDuplicates::set_exclude_contacts(const std::set<std::string>& _exclude_contacts)
{
    exclude_contacts_ = _exclude_contacts;
    return *this;
}

std::set<std::string> FindContactDuplicates::exec(LibFred::OperationContext& _ctx)
{
    std::set<std::string> result;
    Database::QueryParams dup_params;
    std::vector<std::string> contact_address_types = Util::vector_of<std::string>
            ("MAILING")
            ("BILLING")
            ("SHIPPING")
            ("SHIPPING_2")
            ("SHIPPING_3");
    std::stringstream dup_sql;
    dup_sql << "SELECT unnest(dup_set)"
        " FROM (SELECT array_agg(oreg.name) AS dup_set,";
        for (std::vector<std::string>::const_iterator contact_address_type = contact_address_types.begin();
            contact_address_type != contact_address_types.end();
            ++contact_address_type)
        {
            dup_sql << \
            " (SELECT row("
               " trim(BOTH ' ' FROM ca.company_name),"
               " trim(BOTH ' ' FROM ca.street1),"
               " trim(BOTH ' ' FROM ca.street2),"
               " trim(BOTH ' ' FROM ca.street3),"
               " trim(BOTH ' ' FROM ca.city),"
               " trim(BOTH ' ' FROM ca.stateorprovince),"
               " trim(BOTH ' ' FROM ca.postalcode),"
               " trim(BOTH ' ' FROM ca.country)"
               " )"
              " FROM contact_address ca"
             " WHERE ca.type = '" << *contact_address_type << "'"
               " AND ca.contactid = c.id"
            " ) AS " << *contact_address_type << "_addr" << (contact_address_type != contact_address_types.end() - 1 ? "," : "");
        }
    dup_sql << \
        " FROM object_registry oreg"
        " JOIN contact c ON c.id = oreg.id"
        " JOIN object o ON o.id = c.id"
        " JOIN registrar r ON r.id = o.clid";

    if (registrar_handle_.isset()) {
        dup_params.push_back(registrar_handle_.get_value());
        dup_sql << " WHERE r.handle = $" << dup_params.size() << "::text";
    }

    dup_sql << " GROUP BY"
        " trim(BOTH ' ' FROM c.name),"
        " trim(BOTH ' ' FROM c.organization),"
        " c.ssntype,"
        " trim(BOTH ' ' FROM c.ssn),"
        " trim(BOTH ' ' FROM c.vat),"
        " trim(BOTH ' ' FROM c.telephone),"
        " trim(BOTH ' ' FROM c.fax),"
        " trim(BOTH ' ' FROM c.email),"
        " trim(BOTH ' ' FROM c.notifyemail),"
        " trim(BOTH ' ' FROM c.street1),"
        " trim(BOTH ' ' FROM c.street2),"
        " trim(BOTH ' ' FROM c.street3),"
        " trim(BOTH ' ' FROM c.city),"
        " trim(BOTH ' ' FROM c.stateorprovince),"
        " trim(BOTH ' ' FROM c.postalcode),"
        " trim(BOTH ' ' FROM c.country),"
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
        " c.warning_letter,";
    for (std::vector<std::string>::const_iterator contact_address_type = contact_address_types.begin();
        contact_address_type != contact_address_types.end();
        ++contact_address_type)
    {
        dup_sql << \
        " " << *contact_address_type << "_addr" << (contact_address_type != contact_address_types.end() - 1 ? "," : "");
    }
    dup_sql << \
        " HAVING array_upper(array_agg(oreg.name), 1) > 1";
    if (!exclude_contacts_.empty()) {
        std::vector<std::string> array_params;
        for (std::set<std::string>::const_iterator i = exclude_contacts_.begin();
                i != exclude_contacts_.end(); ++i)
        {
            dup_params.push_back(*i);
            array_params.push_back("$" + boost::lexical_cast<std::string>(dup_params.size()));
        }

        dup_sql << " AND NOT (array_agg(oreg.name)"
                << " && array[" << boost::algorithm::join(array_params, ", ") << "]::varchar[])";
    }
    dup_sql << " LIMIT 1) as dup_q";

    Database::Result dup_result = _ctx.get_conn().exec_params(dup_sql.str(), dup_params);

    for (Database::Result::size_type i = 0; i < dup_result.size(); i++) {
        result.insert(static_cast<std::string>(dup_result[i][0]));
    }

    return result;
}

}
}
