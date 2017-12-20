#include "src/libfred/registrable_object/keyset/transfer_keyset.hh"

#include "src/libfred/registrable_object/keyset/info_keyset.hh"
#include "src/libfred/registrable_object/keyset/copy_history_impl.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/object/generate_authinfo_password.hh"
#include "src/libfred/object/transfer_object.hh"

#include "src/libfred/exception.hh"

#include <boost/foreach.hpp>

namespace LibFred
{

    TransferKeyset::TransferKeyset(
        const unsigned long long _keyset_id,
        const std::string& _new_registrar_handle,
        const std::string& _authinfopw_for_authorization,
        const Nullable<unsigned long long>& _logd_request_id
    ) :
        keyset_id_(_keyset_id),
        new_registrar_handle_(_new_registrar_handle),
        authinfopw_for_authorization_(_authinfopw_for_authorization),
        logd_request_id_(_logd_request_id)
    { }

    /**
     * @returns true if _authinfopw_for_authorization is correct
     * @throws UnknownKeysetId
     */
    static bool is_transfer_authorized(OperationContext& _ctx, const unsigned long long _keyset_id, const std::string& _authinfopw_for_authorization) {

        LibFred::InfoKeysetData keyset_data;

        try {
            keyset_data = LibFred::InfoKeysetById(_keyset_id).set_lock().exec(_ctx).info_keyset_data;

        } catch(const LibFred::InfoKeysetById::Exception& e) {
            if( e.is_set_unknown_object_id() ) {
                throw UnknownKeysetId();
            }
            throw;
        }

        if(keyset_data.authinfopw == _authinfopw_for_authorization) {
            return true;
        }

        BOOST_FOREACH(const ObjectIdHandlePair& tech_contact, keyset_data.tech_contacts) {
            if( InfoContactByHandle(tech_contact.handle).exec(_ctx).info_contact_data.authinfopw
                == _authinfopw_for_authorization
            ) {
                return true;
            }
        }

        return false;
    }

    unsigned long long TransferKeyset::exec(OperationContext& _ctx) {

        if( is_transfer_authorized(_ctx, keyset_id_, authinfopw_for_authorization_) ) {
            unsigned long long new_history_id;

            try {
                new_history_id = LibFred::transfer_object(_ctx, keyset_id_, new_registrar_handle_, generate_authinfo_pw(), logd_request_id_ );

            } catch(const UnknownObjectId& e) {
                throw UnknownKeysetId();
            }

            copy_keyset_data_to_keyset_history_impl(_ctx, keyset_id_, new_history_id);

            return new_history_id;

        } else {
            throw IncorrectAuthInfoPw();
        }
    }
}
