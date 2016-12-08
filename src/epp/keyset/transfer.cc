#include "src/epp/keyset/transfer.h"

#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"

#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/transfer_keyset.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/registrar/info_registrar.h"

namespace Epp {

unsigned long long keyset_transfer(
    Fred::OperationContext &_ctx,
    const std::string &_keyset_handle,
    const std::string &_authinfopw,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id)
{
    static const unsigned long long invalid_registrar_id = 0;
    if (_registrar_id == invalid_registrar_id) {
        throw AuthErrorServerClosingConnection();
    }

    try {
        const Fred::InfoKeysetData keyset_data = Fred::InfoKeysetByHandle(_keyset_handle).set_lock()
            .exec(_ctx).info_keyset_data;
        const std::string session_registrar_handle =
            Fred::InfoRegistrarById(_registrar_id)
                .set_lock()
                .exec(_ctx)
                .info_registrar_data.handle;
        if (keyset_data.sponsoring_registrar_handle == session_registrar_handle) {
            throw ObjectNotEligibleForTransfer();
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
        if ((keyset_states.find(Fred::Object_State::server_transfer_prohibited) != keyset_states.end()) ||
            (keyset_states.find(Fred::Object_State::delete_candidate) != keyset_states.end()))
        {
            throw ObjectStatusProhibitsOperation();
        }

        return Fred::TransferKeyset(
                   keyset_data.id,
                   session_registrar_handle,
                   _authinfopw,
                   _logd_request_id.isset() ? _logd_request_id.get_value() : Nullable< unsigned long long >())
               .exec(_ctx);
    }
    catch (const Fred::InfoKeysetByHandle::Exception &e) {
        if (e.is_set_unknown_handle()) {
            throw NonexistentHandle();
        }
        throw;
    }
    catch (const Fred::UnknownKeysetId&) {
        throw NonexistentHandle();
    }
    catch (const Fred::IncorrectAuthInfoPw&) {
        throw AuthorizationError();
    }
    catch (const Fred::NewRegistrarIsAlreadySponsoring&) {
        throw ObjectNotEligibleForTransfer();
    }
}

}
