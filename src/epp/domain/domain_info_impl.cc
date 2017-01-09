#include "src/epp/domain/domain_info_impl.h"

#include "src/epp/exception.h"
#include "src/fredlib/domain/info_domain_data.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object/object_id_handle_pair.h"
#include "util/db/nullable.h"
#include "util/enum_conversion.h"

#include <iterator>
#include <vector>
#include <algorithm>

namespace Epp {

namespace Domain {

DomainInfoOutputData domain_info_impl(
    Fred::OperationContext& _ctx,
    const std::string& _domain_fqdn,
    unsigned long long _registrar_id
) {

    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    DomainInfoOutputData domain_info_output_data;

    try {

        const Fred::InfoDomainData info_domain_data = Fred::InfoDomainByHandle(_domain_fqdn).exec(_ctx, "UTC").info_domain_data;

        domain_info_output_data.roid = info_domain_data.roid;
        domain_info_output_data.fqdn = info_domain_data.fqdn;
        domain_info_output_data.registrant = info_domain_data.registrant.handle;
        domain_info_output_data.nsset = info_domain_data.nsset.isnull()
            ? Nullable<std::string>()
            : Nullable<std::string>(info_domain_data.nsset.get_value().handle);
        domain_info_output_data.keyset = info_domain_data.keyset.isnull()
            ? Nullable<std::string>()
            : Nullable<std::string>(info_domain_data.keyset.get_value().handle);

        {
            typedef std::vector<Fred::ObjectStateData> ObjectStatesData;
            ObjectStatesData domain_states_data = Fred::GetObjectStates(info_domain_data.id).exec(_ctx);
            for (ObjectStatesData::const_iterator object_state_ptr = domain_states_data.begin();
                 object_state_ptr != domain_states_data.end(); ++object_state_ptr)
            {
                if (object_state_ptr->is_external) {
                    domain_info_output_data.states.insert(Conversion::Enums::from_db_handle<Fred::Object_State>(object_state_ptr->state_name));
                }
            }
        }

        domain_info_output_data.sponsoring_registrar_handle = info_domain_data.sponsoring_registrar_handle;
        domain_info_output_data.creating_registrar_handle = info_domain_data.create_registrar_handle;
        domain_info_output_data.last_update_registrar_handle = info_domain_data.update_registrar_handle;

        domain_info_output_data.crdate = info_domain_data.creation_time;
        domain_info_output_data.last_update = info_domain_data.update_time;
        domain_info_output_data.last_transfer = info_domain_data.transfer_time;
        domain_info_output_data.exdate = info_domain_data.expiration_date;

        domain_info_output_data.auth_info_pw = info_domain_data.authinfopw;

        for (
            std::vector<Fred::ObjectIdHandlePair>::const_iterator object_id_handle_pair = info_domain_data.admin_contacts.begin();
            object_id_handle_pair != info_domain_data.admin_contacts.end();
            ++object_id_handle_pair
        ) {
            domain_info_output_data.admin.insert(object_id_handle_pair->handle);
        }

        domain_info_output_data.ext_enum_domain_validation =
            info_domain_data.enum_domain_validation.isnull() ? Nullable<Epp::ENUMValidationExtension>()
                                                             : Epp::ENUMValidationExtension(
                                                                 info_domain_data.enum_domain_validation.get_value().validation_expiration,
                                                                 info_domain_data.enum_domain_validation.get_value().publish
                                                               );


    } catch (const Fred::InfoDomainByHandle::Exception& e) {

        if(e.is_set_unknown_fqdn()) {
            throw NonexistentHandle();
        };

        /* in the improbable case that exception is incorrectly set */
        throw;

    }

    return domain_info_output_data;
}

}

}
