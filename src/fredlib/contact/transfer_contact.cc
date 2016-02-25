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
        const std::string& _authinfopw_for_authorization
    ) :
        contact_id_(_contact_id),
        new_registrar_handle_(_new_registrar_handle),
        authinfopw_for_authorization_(_authinfopw_for_authorization)
    { }

    void TransferContact::exec(OperationContext& _ctx) {

        try {
            if( authinfopw_for_authorization_ != Fred::InfoContactById(contact_id_).set_lock().exec(_ctx).info_contact_data.authinfopw ) {
                throw IncorrectAuthInfoPw();
            }
        } catch(const Fred::InfoContactById::Exception& e) {
            if( e.is_set_unknown_object_id() ) {
                /* XXX relying on the fact that exception is throw if CONTACT is not found */
                throw UnknownContactId();
            }
        }

        transfer_object(_ctx, contact_id_, new_registrar_handle_, generate_authinfo_pw() );
    }

}
