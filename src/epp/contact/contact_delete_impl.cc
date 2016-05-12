#include "src/epp/contact/contact_delete_impl.h"

#include "src/epp/exception.h"

#include <fredlib/contact.h>
#include <fredlib/registrar.h>
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/epp/impl/util.h"

namespace Epp {

unsigned long long contact_delete_impl(
    Fred::OperationContext& _ctx,
    const std::string& _handle,
    const unsigned long long _registrar_id
) {

    if( _registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    {
        Fred::CheckContact check(_handle);

        if(check.is_invalid_handle()) {
            throw InvalidHandle();

        } else if(!check.is_registered(_ctx)) {
            throw NonexistentHandle();
        }
    }

    const Fred::InfoContactData contact_data_before_delete = Fred::InfoContactByHandle(_handle).set_lock().exec(_ctx).info_contact_data;

    const Fred::InfoRegistrarData sponsoring_registrar_before_update =
        Fred::InfoRegistrarByHandle(contact_data_before_delete.sponsoring_registrar_handle)
            .set_lock(/* TODO az to bude mozne, staci lock registrar for share */ )
            .exec(_ctx)
            .info_registrar_data;

    if( sponsoring_registrar_before_update.id != _registrar_id ) {
        throw AutorError();
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(contact_data_before_delete.id).exec(_ctx);
    Fred::PerformObjectStateRequest(contact_data_before_delete.id).exec(_ctx);

    if( Fred::ObjectHasState(contact_data_before_delete.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(_ctx)
        ||
        Fred::ObjectHasState(contact_data_before_delete.id, Fred::ObjectState::SERVER_DELETE_PROHIBITED).exec(_ctx)
        ||
        Fred::ObjectHasState(contact_data_before_delete.id, Fred::ObjectState::DELETE_CANDIDATE).exec(_ctx)
        ||
        Fred::ObjectHasState(contact_data_before_delete.id, Fred::ObjectState::LINKED).exec(_ctx)
    ) {
        throw ObjectStatusProhibitingOperation();
    }

    try {

        Fred::DeleteContactByHandle(_handle).exec(_ctx);

        return contact_data_before_delete.historyid;

    } catch(const Fred::DeleteContactByHandle::Exception& e) {

        /* general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority */
        if( e.is_set_unknown_contact_handle() ) {
            throw;
        }

        if( e.is_set_object_linked_to_contact_handle() ) {
            throw ObjectStatusProhibitingOperation();
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
