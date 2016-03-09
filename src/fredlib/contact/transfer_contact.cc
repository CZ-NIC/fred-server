#include "src/fredlib/contact/transfer_contact.h"

#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object/generate_authinfo_password.h"
#include "src/fredlib/object/transfer_object.h"

#include "src/fredlib/exception.h"

namespace Fred
{

    TransferContact::TransferContact(
        const unsigned long long _contact_id,
        const std::string& _new_registrar_handle,
        const std::string& _authinfopw_for_authorization,
        const Nullable<unsigned long long>& _logd_request_id
    ) :
        contact_id_(_contact_id),
        new_registrar_handle_(_new_registrar_handle),
        authinfopw_for_authorization_(_authinfopw_for_authorization),
        logd_request_id_(_logd_request_id)
    { }

    unsigned long long TransferContact::exec(OperationContext& _ctx) {

        try {
            if( authinfopw_for_authorization_ != Fred::InfoContactById(contact_id_).set_lock().exec(_ctx).info_contact_data.authinfopw ) {
                throw IncorrectAuthInfoPw();
            }
        } catch(const Fred::InfoContactById::Exception& e) {
            if( e.is_set_unknown_object_id() ) {
                /* XXX relying on the fact that exception is throw if CONTACT is not found */
                throw UnknownContactId();
            }
            throw;
        }

        struct ExceptionTranslation {
            static unsigned long long transfer_object(
                Fred::OperationContext& _ctx,
                const unsigned long long _contact_id,
                const std::string& _new_registrar_handle,
                const Nullable<unsigned long long>& _logd_request_id
            ) {
                try {
                    return ::Fred::transfer_object(_ctx, _contact_id, _new_registrar_handle, generate_authinfo_pw(), _logd_request_id );
                } catch(const UnknownObjectId& e) {
                    throw UnknownContactId();
                }
            }
        };
        const unsigned long long new_history_id = ExceptionTranslation::transfer_object(_ctx, contact_id_, new_registrar_handle_, logd_request_id_ );

        {
            // XXX copy+paste from ContactUpdate
            // TODO factor out to "InsertContactHistory" and use here, in UpdateContact and CreateContact
            const Database::Result res = _ctx.get_conn().exec_params(
                "INSERT INTO contact_history(historyid,id "
                " , name, organization, street1, street2, street3, city, stateorprovince, postalcode "
                " , country, telephone, fax, email, notifyemail, vat, ssntype, ssn "
                " , disclosename, discloseorganization, discloseaddress, disclosetelephone "
                " , disclosefax, discloseemail, disclosevat, discloseident, disclosenotifyemail "
                " , warning_letter "
                ") "
                "SELECT $1::bigint, id "
                " , name, organization, street1, street2, street3, city, stateorprovince, postalcode "
                " , country, telephone, fax, email, notifyemail, vat, ssntype, ssn "
                " , disclosename, discloseorganization, discloseaddress, disclosetelephone "
                " , disclosefax, discloseemail, disclosevat, discloseident, disclosenotifyemail "
                " , warning_letter "
                " FROM contact "
                " WHERE id = $2::integer ",
                Database::query_param_list
                    (new_history_id)
                    (contact_id_)
            );

            if(res.rows_affected() != 1) {
                throw std::runtime_error("INSERT INTO contact_history failed");
            }
        }
        {
            // XXX copy+paste from ContactUpdate
            // TODO factor out to "InsertContactHistory" and use here, in UpdateContact and CreateContact
            const Database::Result res = _ctx.get_conn().exec_params(
                "INSERT INTO contact_address_history (historyid, id, contactid, type, company_name, "
                " street1, street2, street3, city, stateorprovince, postalcode, country) "
                " SELECT $1::bigint, id, contactid, type, company_name, "
                " street1, street2, street3, city, stateorprovince, postalcode, country "
                " FROM contact_address WHERE contactid=$2::bigint ",
                Database::query_param_list
                    (new_history_id)
                    (contact_id_)
            );
        }

        return new_history_id;
    }

}
