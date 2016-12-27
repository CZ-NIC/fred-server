#include "src/epp/contact/transfer_contact.h"

#include "src/epp/error.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/exception_aggregate_param_errors.h"
#include "src/epp/impl/reason.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/transfer_contact.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/poll/create_transfer_contact_poll_message.h"
#include "src/fredlib/poll/message_types.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {
namespace Contact {

unsigned long long transfer_contact(
        Fred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const std::string& _authinfopw,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id)
{
    static const unsigned long long invalid_registrar_id = 0;
    if (_registrar_id == invalid_registrar_id) {
        //throw AuthErrorServerClosingConnection();
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    if (Fred::Contact::get_handle_registrability(_ctx, _contact_handle) != Fred::ContactHandleState::Registrability::registered) {
        //throw NonexistentHandle();
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }

    // TODO optimize out
    const Fred::InfoContactData contact_data =
        Fred::InfoContactByHandle(_contact_handle)
            .set_lock()
            .exec(_ctx).info_contact_data;

    const std::string session_registrar_handle =
        Fred::InfoRegistrarById(_registrar_id)
            .set_lock()
            .exec(_ctx).info_registrar_data.handle;

    if (contact_data.sponsoring_registrar_handle == session_registrar_handle) {
        //throw ObjectNotEligibleForTransfer();
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(contact_data.id).exec(_ctx);
    Fred::PerformObjectStateRequest(contact_data.id).exec(_ctx);

    // FIXME use Fred::Object_State instead
    if (Fred::ObjectHasState(contact_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(_ctx) ||
        Fred::ObjectHasState(contact_data.id, Fred::ObjectState::DELETE_CANDIDATE).exec(_ctx))
    {
        throw ObjectStatusProhibitsOperation();
    }

    if (contact_data.authinfopw != _authinfopw) {
        //throw AuthorizationInformationError();
        throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
    }

    try {
        const unsigned long long post_transfer_history_id = 
            Fred::TransferContact(
                contact_data.id,
                session_registrar_handle,
                _authinfopw,
                _logd_request_id.isset() ? _logd_request_id.get_value() : Nullable<unsigned long long>()
            ).exec(_ctx);

        // FIXME use
        //Fred::Poll::CreateEppActionPollMessage(post_transfer_history_id,
        //                                       Fred::Poll::contact,
        //                                       Fred::Poll::TRANSFER_CONTACT).exec(_ctx);
        Fred::Poll::CreateTransferContactPollMessage(post_transfer_history_id).exec(_ctx);

        return post_transfer_history_id;

    }
    catch (const Fred::UnknownContactId&) {
        //throw NonexistentHandle();
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    catch (const Fred::IncorrectAuthInfoPw&) {
        //throw AuthorizationInformationError();
        throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
    }
    catch (const Fred::NewRegistrarIsAlreadySponsoring&) {
        //throw ObjectNotEligibleForTransfer();
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }

}

} // namespace Epp::Contact
} // namespace Epp
