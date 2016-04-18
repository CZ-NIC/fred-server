#include "src/fredlib/nsset/transfer_nsset.h"

#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/copy_history_impl.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object/generate_authinfo_password.h"
#include "src/fredlib/object/transfer_object.h"

#include "src/fredlib/exception.h"

#include <boost/foreach.hpp>

namespace Fred
{

    TransferNsset::TransferNsset(
        const unsigned long long _nsset_id,
        const std::string& _new_registrar_handle,
        const std::string& _authinfopw_for_authorization,
        const Nullable<unsigned long long>& _logd_request_id
    ) :
        nsset_id_(_nsset_id),
        new_registrar_handle_(_new_registrar_handle),
        authinfopw_for_authorization_(_authinfopw_for_authorization),
        logd_request_id_(_logd_request_id)
    { }

    /**
     * @returns true if _authinfopw_for_authorization is correct
     * @throws UnknownNssetId
     */
    static bool is_transfer_authorized(OperationContext& _ctx, const unsigned long long _nsset_id, const std::string& _authinfopw_for_authorization) {

        Fred::InfoNssetData nsset_data;

        try {
            nsset_data = Fred::InfoNssetById(_nsset_id).set_lock().exec(_ctx).info_nsset_data;

        } catch(const Fred::InfoNssetById::Exception& e) {
            if( e.is_set_unknown_object_id() ) {
                throw UnknownNssetId();
            }
            throw;
        }

        if(nsset_data.authinfopw == _authinfopw_for_authorization) {
            return true;
        }

        BOOST_FOREACH(const ObjectIdHandlePair& tech_contact, nsset_data.tech_contacts) {
            if( InfoContactByHandle(tech_contact.handle).exec(_ctx).info_contact_data.authinfopw
                == _authinfopw_for_authorization
            ) {
                return true;
            }
        }

        return false;
    }

    unsigned long long TransferNsset::exec(OperationContext& _ctx) {

        if( is_transfer_authorized(_ctx, nsset_id_, authinfopw_for_authorization_) ) {
            unsigned long long new_history_id;

            try {
                new_history_id = Fred::transfer_object(_ctx, nsset_id_, new_registrar_handle_, generate_authinfo_pw(), logd_request_id_ );

            } catch(const UnknownObjectId& e) {
                throw UnknownNssetId();
            }

            copy_nsset_data_to_nsset_history_impl(_ctx, nsset_id_, new_history_id);

            return new_history_id;

        } else {
            throw IncorrectAuthInfoPw();
        }
    }
}
