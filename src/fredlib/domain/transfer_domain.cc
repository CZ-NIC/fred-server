#include "src/fredlib/domain/transfer_domain.h"

#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/copy_history_impl.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object/generate_authinfo_password.h"
#include "src/fredlib/object/transfer_object.h"

#include "src/fredlib/exception.h"

#include <boost/foreach.hpp>

namespace Fred
{

    TransferDomain::TransferDomain(
        const unsigned long long _domain_id,
        const std::string& _new_registrar_handle,
        const std::string& _authinfopw_for_authorization,
        const Nullable<unsigned long long>& _logd_request_id
    ) :
        domain_id_(_domain_id),
        new_registrar_handle_(_new_registrar_handle),
        authinfopw_for_authorization_(_authinfopw_for_authorization),
        logd_request_id_(_logd_request_id)
    { }

    static std::set<std::string> get_authinfopws_to_check(OperationContext& _ctx, const Fred::InfoDomainData& _domain_data) {
        std::set<std::string> result;

        result.insert(_domain_data.authinfopw);
        result.insert(
            InfoContactByHandle(_domain_data.registrant.handle)
                .exec(_ctx)
                .info_contact_data.authinfopw
        );
        BOOST_FOREACH(const ObjectIdHandlePair& admin_contact, _domain_data.admin_contacts) {
            result.insert(
                InfoContactByHandle(admin_contact.handle)
                    .exec(_ctx)
                    .info_contact_data.authinfopw
            );
        }

        return result;
    }

    unsigned long long TransferDomain::exec(OperationContext& _ctx) {

        Fred::InfoDomainData domain_data;

        try {
            domain_data = Fred::InfoDomainById(domain_id_).set_lock().exec(_ctx).info_domain_data;

        } catch(const Fred::InfoDomainById::Exception& e) {
            if( e.is_set_unknown_object_id() ) {
                throw UnknownDomainId();
            }
            throw;
        }

        {
            const std::set<std::string> authinfopws_to_check = get_authinfopws_to_check(_ctx, domain_data);

            if( authinfopws_to_check.find(authinfopw_for_authorization_) == authinfopws_to_check.end() ) {
                throw IncorrectAuthInfoPw();
            }
        }

        unsigned long long new_history_id;

        try {
            new_history_id = Fred::transfer_object(_ctx, domain_id_, new_registrar_handle_, generate_authinfo_pw(), logd_request_id_ );

        } catch(const UnknownObjectId& e) {
            throw UnknownDomainId();
        }

        copy_domain_data_to_domain_history_impl(_ctx, domain_id_, new_history_id);

        return new_history_id;
    }

}
