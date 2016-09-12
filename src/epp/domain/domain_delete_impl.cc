#include "src/epp/domain/domain_delete_impl.h"

#include "src/epp/exception.h"

#include "src/fredlib/domain/domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/delete_domain.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/epp/impl/util.h"

namespace Epp {

namespace Domain {

unsigned long long domain_delete_impl(
    Fred::OperationContext& ctx,
    const std::string& fqdn,
    const unsigned long long registrar_id
) {

    const bool registrar_is_authenticated = registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    if(Fred::Domain::get_domain_registrability_by_domain_fqdn(ctx, fqdn) != Fred::Domain::DomainRegistrability::registered) {
        throw NonexistentHandle();
    }

    const Fred::InfoRegistrarData callers_registrar =
        Fred::InfoRegistrarById(registrar_id).set_lock().exec(ctx).info_registrar_data;
    const Fred::InfoDomainData domain_data_before_delete = Fred::InfoDomainByHandle(fqdn).set_lock().exec(ctx).info_domain_data;

    const bool is_sponsoring_registrar = (domain_data_before_delete.sponsoring_registrar_handle ==
                                          callers_registrar.handle);
    const bool is_system_registrar = callers_registrar.system.get_value_or(false);
    const bool is_operation_permitted = (is_system_registrar || is_sponsoring_registrar);

    if (!is_operation_permitted) {
        throw AuthorizationError();
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(domain_data_before_delete.id).exec(ctx);
    Fred::PerformObjectStateRequest(domain_data_before_delete.id).exec(ctx);

    const Fred::ObjectStatesInfo domain_states(Fred::GetObjectStates(domain_data_before_delete.id).exec(ctx));
    if (!is_system_registrar) {
        if (domain_states.presents(Fred::Object_State::server_update_prohibited) ||
            domain_states.presents(Fred::Object_State::server_delete_prohibited) ||
            domain_states.presents(Fred::Object_State::delete_candidate))
        {
            throw ObjectStatusProhibitsOperation();
        }
    }

    try {

        Fred::DeleteDomainByHandle(fqdn).exec(ctx);

        return domain_data_before_delete.historyid;

    } catch(const Fred::DeleteDomainByHandle::Exception& e) {

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
