#include "src/epp/nsset/transfer_nsset.h"

#include "src/epp/error.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/exception_aggregate_param_errors.h"
#include "src/epp/impl/reason.h"
#include "src/epp/impl/util.h"

#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/nsset/transfer_nsset.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/poll/create_transfer_nsset_poll_message.h"
#include "src/fredlib/registrar/info_registrar.h"

#include <boost/foreach.hpp>

namespace Epp {
namespace Nsset {

unsigned long long transfer_nsset(
    Fred::OperationContext& _ctx,
    const std::string& _nsset_handle,
    const std::string& _authinfopw,
    const unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id)
{
    static const unsigned long long invalid_registrar_id = 0;
    if (_registrar_id == invalid_registrar_id) {
        throw AuthErrorServerClosingConnection();
    }

    if( Fred::Nsset::get_handle_registrability(_ctx, _nsset_handle) != Fred::NssetHandleState::Registrability::registered ) {
        throw NonexistentHandle();
    }

    const Fred::InfoNssetData nsset_data = Fred::InfoNssetByHandle(_nsset_handle).set_lock().exec(_ctx).info_nsset_data;

    //set of authinfopw
    std::set<std::string> nsset_tech_c_authinfo;
    nsset_tech_c_authinfo.insert(nsset_data.authinfopw);
    BOOST_FOREACH(const Fred::ObjectIdHandlePair& tech_c,nsset_data.tech_contacts)
    {
        nsset_tech_c_authinfo.insert(Fred::InfoContactById(tech_c.id).exec(_ctx).info_contact_data.authinfopw);
    }

    const std::string session_registrar_handle = Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data.handle;

    if(nsset_data.sponsoring_registrar_handle == session_registrar_handle) {
        throw ObjectNotEligibleForTransfer();
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(nsset_data.id).exec(_ctx);
    Fred::PerformObjectStateRequest(nsset_data.id).exec(_ctx);

    // FIXME use Fred::Object_State instead
    if(Fred::ObjectHasState(nsset_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(_ctx) ||
        Fred::ObjectHasState(nsset_data.id, Fred::ObjectState::DELETE_CANDIDATE).exec(_ctx))
    {
        throw ObjectStatusProhibitsOperation();
    }

    if(nsset_tech_c_authinfo.find(_authinfopw) == nsset_tech_c_authinfo.end()) {
        throw AuthorizationInformationError();
    }

    try {
        unsigned long long post_transfer_history_id =
            Fred::TransferNsset(
                    nsset_data.id,
                    session_registrar_handle,
                    _authinfopw,
                    _logd_request_id.isset() ? _logd_request_id.get_value() : Nullable<unsigned long long>())
            .exec(_ctx);

        // TODO use 
        //Fred::Poll::CreateEppActionPollMessage(post_transfer_history_id,
        //                                       Fred::Poll::nsset,
        //                                       Fred::Poll::TRANSFER_NSSET).exec(_ctx);
        Fred::Poll::CreateTransferNssetPollMessage(post_transfer_history_id).exec(_ctx);

        return post_transfer_history_id;

    }
    catch (const Fred::UnknownNssetId&) {
        throw NonexistentHandle();
    }
    catch (const Fred::IncorrectAuthInfoPw&) {
        throw AuthorizationInformationError();
    }
    catch (const Fred::NewRegistrarIsAlreadySponsoring&) {
        throw ObjectNotEligibleForTransfer();
    }
}

} // namespace Epp::Nsset
} // namespace Epp
