#include "src/epp/keyset/transfer_keyset.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/epp/reason.h"
#include "src/epp/util.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/transfer_keyset.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/poll/create_epp_action_poll_message_impl.h"
#include "src/fredlib/poll/message_types.h"
#include "src/fredlib/registrar/info_registrar.h"

#include <string>

namespace Epp {
namespace Keyset {

unsigned long long transfer_keyset(
        Fred::OperationContext& _ctx,
        const std::string& _keyset_handle,
        const std::string& _authinfopw,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id)
{
    static const unsigned long long invalid_registrar_id = 0;
    if (_registrar_id == invalid_registrar_id)
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    try
    {
        const Fred::InfoKeysetData keyset_data_before_transfer =
                Fred::InfoKeysetByHandle(_keyset_handle)
                        .set_lock()
                        .exec(_ctx)
                        .info_keyset_data;

        const Fred::InfoRegistrarData session_registrar =
                Fred::InfoRegistrarById(_registrar_id)
                        .exec(_ctx)
                        .info_registrar_data;

        const bool is_sponsoring_registrar = (keyset_data_before_transfer.sponsoring_registrar_handle ==
                                              session_registrar.handle);

        if (is_sponsoring_registrar)
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
        }

        const bool is_system_registrar = session_registrar.system.get_value_or(false);
        if (!is_system_registrar)
        {
            // do it before any object state related checks
            Fred::LockObjectStateRequestLock(keyset_data_before_transfer.id).exec(_ctx);
            Fred::PerformObjectStateRequest(keyset_data_before_transfer.id).exec(_ctx);

            const Fred::ObjectStatesInfo keyset_states_before_transfer(Fred::GetObjectStates(
                            keyset_data_before_transfer.id).exec(_ctx));

            if (keyset_states_before_transfer.presents(Fred::Object_State::server_transfer_prohibited) ||
                keyset_states_before_transfer.presents(Fred::Object_State::delete_candidate))
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
            }
        }

        if (keyset_data_before_transfer.authinfopw != _authinfopw)
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
        }

        const unsigned long long post_transfer_history_id =
                Fred::TransferKeyset(
                        keyset_data_before_transfer.id,
                        session_registrar.handle,
                        _authinfopw,
                        _logd_request_id.isset() ? _logd_request_id.get_value() : Nullable<unsigned long long>())
                        .exec(_ctx);

        Fred::Poll::CreateEppActionPollMessage(
                post_transfer_history_id,
                Fred::Poll::keyset,
                Fred::Poll::TRANSFER_KEYSET)
                .exec(_ctx);

        return post_transfer_history_id;

    }
    catch (const Fred::InfoKeysetByHandle::Exception& e)
    {
        if (e.is_set_unknown_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
        throw;
    }
    catch (const Fred::UnknownKeysetId&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    catch (const Fred::IncorrectAuthInfoPw&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
    }
    catch (const Fred::NewRegistrarIsAlreadySponsoring&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }
}


} // namespace Epp::Keyset
} // namespace Epp
