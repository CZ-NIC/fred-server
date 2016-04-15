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

    static std::set<std::string> get_authinfopws_to_check(OperationContext& _ctx, const Fred::InfoNssetData& _nsset_data) {
        std::set<std::string> result;

        result.insert(_nsset_data.authinfopw);

        BOOST_FOREACH(const ObjectIdHandlePair& admin_contact, _nsset_data.tech_contacts) {
            result.insert(
                InfoContactByHandle(admin_contact.handle)
                    .exec(_ctx)
                    .info_contact_data.authinfopw
            );
        }

        return result;
    }

    unsigned long long TransferNsset::exec(OperationContext& _ctx) {

        Fred::InfoNssetData nsset_data;

        try {
            nsset_data = Fred::InfoNssetById(nsset_id_).set_lock().exec(_ctx).info_nsset_data;

        } catch(const Fred::InfoNssetById::Exception& e) {
            if( e.is_set_unknown_object_id() ) {
                throw UnknownNssetId();
            }
            throw;
        }

        {
            const std::set<std::string> authinfopws_to_check = get_authinfopws_to_check(_ctx, nsset_data);

            if( authinfopws_to_check.find(authinfopw_for_authorization_) == authinfopws_to_check.end() ) {
                throw IncorrectAuthInfoPw();
            }
        }

        unsigned long long new_history_id;

        try {
            new_history_id = Fred::transfer_object(_ctx, nsset_id_, new_registrar_handle_, generate_authinfo_pw(), logd_request_id_ );

        } catch(const UnknownObjectId& e) {
            throw UnknownNssetId();
        }

        copy_nsset_data_to_nsset_history_impl(_ctx, nsset_id_, new_history_id);

        return new_history_id;
    }

}
