#include "src/libfred/registrable_object/contact/transfer_contact.hh"

#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/contact/copy_history_impl.hh"
#include "src/libfred/object/generate_authinfo_password.hh"
#include "src/libfred/object/transfer_object.hh"

#include "src/libfred/exception.hh"

namespace LibFred
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
            if( authinfopw_for_authorization_ != LibFred::InfoContactById(contact_id_).set_lock().exec(_ctx).info_contact_data.authinfopw ) {
                throw IncorrectAuthInfoPw();
            }
        } catch(const LibFred::InfoContactById::Exception& e) {
            if( e.is_set_unknown_object_id() ) {
                throw UnknownContactId();
            }

            throw;
        }

        struct ExceptionTranslation {
            static unsigned long long transfer_object(
                LibFred::OperationContext& _ctx,
                const unsigned long long _contact_id,
                const std::string& _new_registrar_handle,
                const Nullable<unsigned long long>& _logd_request_id
            ) {
                try {
                    return ::LibFred::transfer_object(_ctx, _contact_id, _new_registrar_handle, generate_authinfo_pw(), _logd_request_id );
                } catch(const UnknownObjectId& e) {
                    throw UnknownContactId();
                }
            }
        };
        const unsigned long long new_history_id = ExceptionTranslation::transfer_object(_ctx, contact_id_, new_registrar_handle_, logd_request_id_ );

        copy_contact_data_to_contact_history_impl(_ctx, contact_id_, new_history_id);

        return new_history_id;
    }

}
