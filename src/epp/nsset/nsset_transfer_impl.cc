#include "src/epp/nsset/nsset_transfer_impl.h"

#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"

#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/transfer_nsset.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/registrar/info_registrar.h"

namespace Epp {

unsigned long long nsset_transfer_impl(
    Fred::OperationContext& _ctx,
    const std::string& _nsset_handle,
    const std::string& _authinfopw,
    const unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
) {

    if( _registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    if( Fred::Nsset::get_handle_registrability(_ctx, _nsset_handle) != Fred::NssetHandleState::Registrability::registered ) {
        throw NonexistentHandle();
    }

    // TODO optimize out
    const Fred::InfoNssetData nsset_data = Fred::InfoNssetByHandle(_nsset_handle).set_lock().exec(_ctx).info_nsset_data;
    const std::string session_registrar_handle = Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data.handle;

    if(nsset_data.sponsoring_registrar_handle == session_registrar_handle) {
        throw ObjectNotEligibleForTransfer();
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(nsset_data.id).exec(_ctx);
    Fred::PerformObjectStateRequest(nsset_data.id).exec(_ctx);

    if( Fred::ObjectHasState(nsset_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(_ctx)
        ||
        Fred::ObjectHasState(nsset_data.id, Fred::ObjectState::DELETE_CANDIDATE).exec(_ctx)
    ) {
        throw ObjectStatusProhibitsOperation();
    }

    if(nsset_data.authinfopw != _authinfopw) {
        throw AuthorizationError();
    }

    try {
        return
            Fred::TransferNsset(
                nsset_data.id,
                session_registrar_handle,
                _authinfopw,
                _logd_request_id.isset() ? _logd_request_id.get_value() : Nullable<unsigned long long>()
            ).exec(_ctx);

    } catch (const Fred::UnknownNssetId&) {
        throw NonexistentHandle();

    } catch (const Fred::IncorrectAuthInfoPw&) {
        throw AuthorizationError();

    } catch (const Fred::NewRegistrarIsAlreadySponsoring&) {
        throw ObjectNotEligibleForTransfer();
    }
}

}
