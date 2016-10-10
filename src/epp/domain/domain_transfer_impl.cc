#include "src/epp/domain/domain_transfer_impl.h"

#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"

#include "src/fredlib/domain/domain.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/transfer_domain.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/registrar/info_registrar.h"

namespace Epp {

namespace Domain {

unsigned long long domain_transfer_impl(
    Fred::OperationContext& _ctx,
    const std::string& _domain_fqdn,
    const std::string& _authinfopw,
    const unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
) {

    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    // TODO checkRegistrarZoneAccess

    try {
        if(Fred::Domain::get_domain_registrability_by_domain_fqdn(_ctx, _domain_fqdn) != Fred::Domain::DomainRegistrability::registered) {
            throw NonexistentHandle();
        }
    }
    catch (const Fred::Domain::ExceptionInvalidFqdn&) {
        throw NonexistentHandle();
    }
    catch (const NonexistentHandle&) {
        throw;
    }

    const std::string session_registrar_handle =
        Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data.handle;
    const Fred::InfoDomainData domain_data_before_transfer = Fred::InfoDomainByHandle(_domain_fqdn).set_lock().exec(_ctx).info_domain_data;

    const bool is_sponsoring_registrar = (domain_data_before_transfer.sponsoring_registrar_handle ==
                                          session_registrar_handle);

    if(is_sponsoring_registrar) {
        throw ObjectNotEligibleForTransfer();
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(domain_data_before_transfer.id).exec(_ctx);
    Fred::PerformObjectStateRequest(domain_data_before_transfer.id).exec(_ctx);

    const Fred::ObjectStatesInfo domain_states(Fred::GetObjectStates(domain_data_before_transfer.id).exec(_ctx));

    if (domain_states.presents(Fred::Object_State::server_transfer_prohibited) ||
        domain_states.presents(Fred::Object_State::delete_candidate))
    {
        throw ObjectStatusProhibitsOperation();
    }

    if(domain_data_before_transfer.authinfopw != _authinfopw) {
        throw AuthorizationInformationError();
    }

    try {
        return
            Fred::TransferDomain(
                domain_data_before_transfer.id,
                session_registrar_handle,
                _authinfopw,
                _logd_request_id.isset() ? _logd_request_id.get_value() : Nullable<unsigned long long>()
            ).exec(_ctx);

    } catch (const Fred::UnknownDomainId&) {
        throw NonexistentHandle();

    } catch (const Fred::IncorrectAuthInfoPw&) {
        throw AuthorizationInformationError();

    } catch (const Fred::NewRegistrarIsAlreadySponsoring&) {
        throw ObjectNotEligibleForTransfer();
    }
}

}

}
