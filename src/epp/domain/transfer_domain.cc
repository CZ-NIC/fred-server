#include "src/epp/domain/transfer_domain.h"

#include "src/epp/error.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/reason.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/domain/domain.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/transfer_domain.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/registrar_zone_access.h"
#include "src/fredlib/zone/zone.h"
#include "util/optional_value.h"

#include <boost/date_time/gregorian/greg_date.hpp>
#include <string>

namespace Epp {

namespace Domain {

unsigned long long transfer_domain(
        Fred::OperationContext& _ctx,
        const std::string& _domain_fqdn,
        const std::string& _authinfopw,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id)
{

    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    boost::gregorian::date current_local_date = boost::posix_time::microsec_clock::local_time().date();

    Fred::Zone::Data zone_data;
    try {
        zone_data = Fred::Zone::find_zone_in_fqdn(_ctx,
            Fred::Zone::rem_trailing_dot(_domain_fqdn));
    } catch (const Fred::Zone::Exception& e) {
        if(e.is_set_unknown_zone_in_fqdn()) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
        /* in the improbable case that exception is incorrectly set */
        throw;
    }

    if(!Fred::is_zone_accessible_by_registrar(_registrar_id, zone_data.id, current_local_date, _ctx)) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error));
    }

    Fred::InfoDomainData domain_data_before_transfer;
    try
    {
        domain_data_before_transfer = Fred::InfoDomainByHandle(
                Fred::Zone::rem_trailing_dot(_domain_fqdn)).set_lock().exec(_ctx).info_domain_data;
    }
    catch(const Fred::InfoDomainByHandle::Exception& ex)
    {
        if(ex.is_set_unknown_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }

    const Fred::InfoRegistrarData session_registrar =
        Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data;

    const bool is_sponsoring_registrar = (domain_data_before_transfer.sponsoring_registrar_handle ==
                                          session_registrar.handle);

    if(is_sponsoring_registrar) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(domain_data_before_transfer.id).exec(_ctx);
    Fred::PerformObjectStateRequest(domain_data_before_transfer.id).exec(_ctx);

    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    if (!is_system_registrar) {
        const Fred::ObjectStatesInfo domain_states(Fred::GetObjectStates(domain_data_before_transfer.id).exec(_ctx));

        if (domain_states.presents(Fred::Object_State::server_transfer_prohibited) ||
            domain_states.presents(Fred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    try {
        return
            Fred::TransferDomain(
                domain_data_before_transfer.id,
                session_registrar.handle,
                _authinfopw,
                _logd_request_id.isset() ? _logd_request_id.get_value() : Nullable<unsigned long long>()
            ).exec(_ctx);
    }
    catch (const Fred::UnknownDomainId&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    catch (const Fred::IncorrectAuthInfoPw&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::invalid_authorization_information));
    }
    catch (const Fred::NewRegistrarIsAlreadySponsoring&) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }
}

}

}
