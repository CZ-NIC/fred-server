#include "src/epp/domain/delete_domain.h"

#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/domain/delete_domain.h"
#include "src/fredlib/domain/domain.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/registrar_zone_access.h"
#include "src/fredlib/zone/zone.h"

#include <boost/date_time/gregorian/greg_date.hpp>

#include <string>

namespace Epp {
namespace Domain {

unsigned long long delete_domain(
        Fred::OperationContext& _ctx,
        const std::string& _domain_fqdn,
        const unsigned long long _registrar_id)
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
    }
    catch (const Fred::Zone::Exception& e) {
        if (e.is_set_unknown_zone_in_fqdn()) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }

    if (!Fred::is_zone_accessible_by_registrar(_registrar_id, zone_data.id, current_local_date, _ctx)) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error));
    }

    Fred::InfoDomainData domain_data_before_delete;
    try
    {
        domain_data_before_delete = Fred::InfoDomainByHandle(
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

    const bool is_sponsoring_registrar = (domain_data_before_delete.sponsoring_registrar_handle ==
                                          session_registrar.handle);
    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    const bool is_registrar_authorized = (is_sponsoring_registrar || is_system_registrar);

    if (!is_registrar_authorized) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::registrar_autor,
                                                         Reason::unauthorized_registrar)));
    }

    if (!is_system_registrar) {
        // do it before any object state related checks
        Fred::LockObjectStateRequestLock(domain_data_before_delete.id).exec(_ctx);
        Fred::PerformObjectStateRequest(domain_data_before_delete.id).exec(_ctx);

        const Fred::ObjectStatesInfo domain_states(Fred::GetObjectStates(domain_data_before_delete.id).exec(_ctx));
        if (domain_states.presents(Fred::Object_State::server_update_prohibited) ||
            domain_states.presents(Fred::Object_State::server_delete_prohibited) ||
            domain_states.presents(Fred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    try {

        Fred::DeleteDomainByHandle(_domain_fqdn).exec(_ctx);

        return domain_data_before_delete.historyid;

    }
    catch (const Fred::DeleteDomainByHandle::Exception& e) {

        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_domain_fqdn()) {
            throw;
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}

} // namespace Epp::Domain
} // namespace Epp
