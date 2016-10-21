#include "src/epp/domain/domain_delete_impl.h"

#include "src/epp/exception.h"

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

namespace Epp {

namespace Domain {

unsigned long long domain_delete_impl(
    Fred::OperationContext& _ctx,
    const std::string& _domain_fqdn,
    const unsigned long long _registrar_id
) {

    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    boost::gregorian::date current_local_date = boost::posix_time::microsec_clock::local_time().date();

    const Fred::Zone::Data zone_data = Fred::Zone::find_zone_in_fqdn(_ctx,
            Fred::Zone::rem_trailing_dot(_domain_fqdn));

    if (!Fred::registrar_zone_access(_registrar_id, zone_data.id, current_local_date, _ctx)) {
        throw AuthorizationError();
    }

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

    const Fred::InfoRegistrarData session_registrar =
        Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data;
    const Fred::InfoDomainData domain_data_before_delete = Fred::InfoDomainByHandle(_domain_fqdn).set_lock().exec(_ctx).info_domain_data;

    const bool is_sponsoring_registrar = (domain_data_before_delete.sponsoring_registrar_handle ==
                                          session_registrar.handle);
    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    const bool is_operation_permitted = (is_sponsoring_registrar || is_system_registrar);

    if (!is_operation_permitted) {
        throw AuthorizationError();
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
            throw ObjectStatusProhibitsOperation();
        }
    }

    try {

        Fred::DeleteDomainByHandle(_domain_fqdn).exec(_ctx);

        return domain_data_before_delete.historyid;

    } catch (const Fred::DeleteDomainByHandle::Exception& e) {

        /* general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority */
        if(e.is_set_unknown_domain_fqdn()) {
            throw;
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}

}
