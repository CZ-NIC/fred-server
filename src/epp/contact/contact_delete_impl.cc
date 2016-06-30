#include "src/epp/contact/contact_delete_impl.h"

#include "src/epp/exception.h"

#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/delete_contact.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/object/states_info.h"
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

    if( Fred::Contact::get_handle_registrability(_ctx, _handle) != Fred::ContactHandleState::Registrability::registered ) {
        throw NonexistentHandle();
    }

    const Fred::InfoRegistrarData callers_registrar =
        Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data;
    const Fred::InfoContactData contact_data_before_delete = Fred::InfoContactByHandle(_handle).set_lock().exec(_ctx).info_contact_data;

    const bool is_sponsoring_registrar = (contact_data_before_delete.sponsoring_registrar_handle ==
                                          callers_registrar.handle);
    const bool is_system_registrar = callers_registrar.system.get_value_or(false);
    const bool is_operation_permitted = (is_system_registrar || is_sponsoring_registrar);

    if (!is_operation_permitted) {
        throw AuthorizationError();
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(contact_data_before_delete.id).exec(_ctx);
    Fred::PerformObjectStateRequest(contact_data_before_delete.id).exec(_ctx);

    const Fred::ObjectStatesInfo contact_states(Fred::GetObjectStates(contact_data_before_delete.id).exec(_ctx));
    if ((!is_system_registrar && (contact_states.presents(Fred::Object_State::server_update_prohibited) ||
                                  contact_states.presents(Fred::Object_State::server_delete_prohibited) ||
                                  contact_states.presents(Fred::Object_State::delete_candidate))) ||
        contact_states.presents(Fred::Object_State::linked))
    {
        throw ObjectStatusProhibitsOperation();
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
            throw ObjectStatusProhibitsOperation();
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
