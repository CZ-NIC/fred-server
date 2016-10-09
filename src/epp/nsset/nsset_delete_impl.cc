#include "src/epp/nsset/nsset_delete_impl.h"

#include "src/epp/exception.h"

#include <fredlib/nsset.h>
#include <fredlib/registrar.h>
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/epp/impl/util.h"

namespace Epp {

unsigned long long nsset_delete_impl(
    Fred::OperationContext& _ctx,
    const std::string& _handle,
    const unsigned long long _registrar_id
) {

    if( _registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    if( Fred::Nsset::get_handle_registrability(_ctx, _handle) != Fred::NssetHandleState::Registrability::registered ) {
        throw NonexistentHandle();
    }

    const Fred::InfoNssetData nsset_data_before_delete = Fred::InfoNssetByHandle(_handle).set_lock().exec(_ctx).info_nsset_data;

    const Fred::InfoRegistrarData sponsoring_registrar_before_update =
        Fred::InfoRegistrarByHandle(nsset_data_before_delete.sponsoring_registrar_handle)
            .set_lock(/* TODO az to bude mozne, staci lock registrar for share */ )
            .exec(_ctx)
            .info_registrar_data;

    const Fred::InfoRegistrarData logged_in_registrar = Fred::InfoRegistrarById(_registrar_id)
                .set_lock(/* TODO lock registrar for share */ )
                .exec(_ctx)
                .info_registrar_data;

    if( sponsoring_registrar_before_update.id != _registrar_id
        && !logged_in_registrar.system.get_value_or_default() ) {
        throw AuthorizationError();
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(nsset_data_before_delete.id).exec(_ctx);
    Fred::PerformObjectStateRequest(nsset_data_before_delete.id).exec(_ctx);

    if(!logged_in_registrar.system.get_value_or_default()
            && ( Fred::ObjectHasState(nsset_data_before_delete.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(_ctx)
        ||
        Fred::ObjectHasState(nsset_data_before_delete.id, Fred::ObjectState::SERVER_DELETE_PROHIBITED).exec(_ctx)
        ||
        Fred::ObjectHasState(nsset_data_before_delete.id, Fred::ObjectState::DELETE_CANDIDATE).exec(_ctx)
    )) {
        throw ObjectStatusProhibitsOperation();
    }

    if(Fred::ObjectHasState(nsset_data_before_delete.id, Fred::ObjectState::LINKED).exec(_ctx))
    {
        throw ObjectAssociationProhibitsOperation();
    }

    try {

        Fred::DeleteNssetByHandle(_handle).exec(_ctx);

        return nsset_data_before_delete.historyid;

    } catch(const Fred::DeleteNssetByHandle::Exception& e) {

        /* general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority */
        if( e.is_set_unknown_nsset_handle() ) {
            throw;
        }

        if( e.is_set_object_linked_to_nsset_handle() ) {
            throw ObjectStatusProhibitsOperation();
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
