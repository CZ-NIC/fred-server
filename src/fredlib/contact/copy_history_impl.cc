#include "src/fredlib/contact/copy_history_impl.h"

namespace Fred
{

    void copy_contact_data_to_contact_history_impl(
        Fred::OperationContext& _ctx,
        const unsigned long long _contact_id,
        const unsigned long long _historyid
    ) {
        const Database::Result res = _ctx.get_conn().exec_params(
            "INSERT INTO contact_history( "
                "historyid,  id, name, organization, street1, street2, street3, city, stateorprovince, postalcode, country, telephone, fax, email, notifyemail, vat, ssntype, ssn, "
                "disclosename, discloseorganization, discloseaddress, disclosetelephone, disclosefax, discloseemail, disclosevat, discloseident, disclosenotifyemail, warning_letter "
            ") "
             /* ^^^ columns should match vvv */
            "SELECT "
                "$1::bigint, id, name, organization, street1, street2, street3, city, stateorprovince, postalcode, country, telephone, fax, email, notifyemail, vat, ssntype, ssn, "
                "disclosename, discloseorganization, discloseaddress, disclosetelephone, disclosefax, discloseemail, disclosevat, discloseident, disclosenotifyemail, warning_letter "
            "FROM contact "
            "WHERE id = $2::integer ",
            Database::query_param_list(_historyid)(_contact_id)
        );

        if(res.rows_affected() != 1) {
            throw std::runtime_error("INSERT INTO contact_history failed");
        }

        _ctx.get_conn().exec_params(
            "INSERT INTO contact_address_history ( "
                "historyid,  id, contactid, type, company_name, street1, street2, street3, city, stateorprovince, postalcode, country "
            ") "
            /* ^^^ columns should match vvv */
            "SELECT "
                "$1::bigint, id, contactid, type, company_name, street1, street2, street3, city, stateorprovince, postalcode, country "
            "FROM contact_address WHERE contactid=$2::bigint ",
            Database::query_param_list(_historyid)(_contact_id)
        );
    }
}
