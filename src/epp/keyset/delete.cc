#include "src/epp/keyset/delete.h"

#include "src/epp/exception.h"

#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/delete_keyset.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"

namespace Epp {

unsigned long long keyset_delete(
    Fred::OperationContext &_ctx,
    const std::string &_keyset_handle,
    unsigned long long _registrar_id)
{
    static const unsigned long long invalid_registrar_id = 0;
    if (_registrar_id == invalid_registrar_id) {
        throw AuthErrorServerClosingConnection();
    }

    try {
        const Fred::InfoKeysetData keyset_data = Fred::InfoKeysetByHandle(_keyset_handle).set_lock()
            .exec(_ctx).info_keyset_data;
        const Fred::InfoRegistrarData sponsoring_registrar_data =
            Fred::InfoRegistrarByHandle(keyset_data.sponsoring_registrar_handle)
                .set_lock()//TODO az to bude mozne, staci lock registrar for share
                .exec(_ctx)
                .info_registrar_data;
        if (sponsoring_registrar_data.id != _registrar_id) {
            throw AutorError();
        }

        // do it before any object state related checks
        Fred::LockObjectStateRequestLock(keyset_data.id).exec(_ctx);
        Fred::PerformObjectStateRequest(keyset_data.id).exec(_ctx);
        typedef std::vector< Fred::ObjectStateData > StatesData;
        StatesData states_data = Fred::GetObjectStates(keyset_data.id).exec(_ctx);
        typedef std::set< Fred::Object_State::Enum > ObjectStates;
        ObjectStates keyset_states;
        for (StatesData::const_iterator data_ptr = states_data.begin(); data_ptr != states_data.end(); ++data_ptr) {
            keyset_states.insert(Conversion::Enums::from_db_handle< Fred::Object_State >(data_ptr->state_name));
        }
        if ((keyset_states.find(Fred::Object_State::server_update_prohibited) != keyset_states.end()) ||
            (keyset_states.find(Fred::Object_State::server_delete_prohibited) != keyset_states.end()) ||
            (keyset_states.find(Fred::Object_State::delete_candidate) != keyset_states.end()) ||
            (keyset_states.find(Fred::Object_State::linked) != keyset_states.end()))
        {
            throw ObjectStatusProhibitingOperation();
        }

        Fred::DeleteKeysetByHandle(_keyset_handle).exec(_ctx);

        return keyset_data.historyid;
    }
    catch (const Fred::InfoKeysetByHandle::Exception &e) {
        if (e.is_set_unknown_handle()) {
            throw NonexistentHandle();
        }
        throw;
    }
    catch( const Fred::DeleteKeysetByHandle::Exception &e) {

        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_keyset_handle()) {
            throw;
        }

        if (e.is_set_object_linked_to_keyset_handle()) {
            throw ObjectStatusProhibitingOperation();
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}

}
