#include "src/fredlib/keyset/transfer_keyset.h"

#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/copy_history_impl.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object/generate_authinfo_password.h"
#include "src/fredlib/object/transfer_object.h"

#include "src/fredlib/exception.h"

#include <boost/foreach.hpp>

namespace Fred
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

    static std::set<std::string> get_authinfopws_to_check(OperationContext& _ctx, const Fred::InfoKeysetData& _keyset_data) {
        std::set<std::string> result;

        result.insert(_keyset_data.authinfopw);

        BOOST_FOREACH(const ObjectIdHandlePair& admin_contact, _keyset_data.tech_contacts) {
            result.insert(
                InfoContactByHandle(admin_contact.handle)
                    .exec(_ctx)
                    .info_contact_data.authinfopw
            );
        }

        return result;
    }

    unsigned long long TransferKeyset::exec(OperationContext& _ctx) {

        Fred::InfoKeysetData keyset_data;

        try {
            keyset_data = Fred::InfoKeysetById(keyset_id_).set_lock().exec(_ctx).info_keyset_data;

        } catch(const Fred::InfoKeysetById::Exception& e) {
            if( e.is_set_unknown_object_id() ) {
                throw UnknownKeysetId();
            }
            throw;
        }

        {
            const std::set<std::string> authinfopws_to_check = get_authinfopws_to_check(_ctx, keyset_data);

            if( authinfopws_to_check.find(authinfopw_for_authorization_) == authinfopws_to_check.end() ) {
                throw IncorrectAuthInfoPw();
            }
        }

        unsigned long long new_history_id;

        try {
            new_history_id = Fred::transfer_object(_ctx, keyset_id_, new_registrar_handle_, generate_authinfo_pw(), logd_request_id_ );

        } catch(const UnknownObjectId& e) {
            throw UnknownKeysetId();
        }

        copy_keyset_data_to_keyset_history_impl(_ctx, keyset_id_, new_history_id);

        return new_history_id;
    }

}
