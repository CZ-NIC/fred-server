/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/epp/domain/info_domain.h"

#include "src/epp/domain/status_value.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/info_domain_data.h"
#include "src/fredlib/object/object_id_handle_pair.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/zone/zone.h"
#include "util/db/nullable.h"
#include "util/enum_conversion.h"

#include <string>
#include <vector>

namespace Epp {
namespace Domain {

InfoDomainOutputData info_domain(
        Fred::OperationContext& _ctx,
        const std::string& _fqdn,
        const InfoDomainConfigData& _info_domain_config_data,
        const SessionData& _session_data)
{
    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    InfoDomainOutputData info_domain_output_data;

    try
    {
        const Fred::InfoDomainData info_domain_data = Fred::InfoDomainByHandle(
                Fred::Zone::rem_trailing_dot(_fqdn)).exec(_ctx, "UTC").info_domain_data;

        info_domain_output_data.roid = info_domain_data.roid;
        info_domain_output_data.fqdn = info_domain_data.fqdn;
        info_domain_output_data.registrant = info_domain_data.registrant.handle;
        info_domain_output_data.nsset = info_domain_data.nsset.isnull()
                                                ? Nullable<std::string>()
                                                : Nullable<std::string>(info_domain_data.nsset.get_value().handle);
        info_domain_output_data.keyset = info_domain_data.keyset.isnull()
                                                 ? Nullable<std::string>()
                                                 : Nullable<std::string>(info_domain_data.keyset.get_value().handle);

        {

            typedef std::vector<Fred::ObjectStateData> ObjectStatesData;

            ObjectStatesData domain_states_data = Fred::GetObjectStates(info_domain_data.id).exec(_ctx);
            for (ObjectStatesData::const_iterator object_state_ptr = domain_states_data.begin();
                 object_state_ptr != domain_states_data.end(); ++object_state_ptr)
            {
                if (object_state_ptr->is_external)
                {
                    info_domain_output_data.states.insert(Conversion::Enums::from_fred_object_state<StatusValue>(
                            Conversion::Enums::from_db_handle<Fred::Object_State>(
                                    object_state_ptr->state_name)));
                }
            }
        }

        info_domain_output_data.sponsoring_registrar_handle = info_domain_data.sponsoring_registrar_handle;
        info_domain_output_data.creating_registrar_handle = info_domain_data.create_registrar_handle;
        info_domain_output_data.last_update_registrar_handle = info_domain_data.update_registrar_handle;

        info_domain_output_data.crdate = info_domain_data.creation_time;
        info_domain_output_data.last_update = info_domain_data.update_time;
        info_domain_output_data.last_transfer = info_domain_data.transfer_time;
        info_domain_output_data.exdate = info_domain_data.expiration_date;

        // show object authinfopw only to sponsoring registrar
        const std::string session_registrar_handle =
                Fred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data.handle;
        const bool authinfopw_has_to_be_hidden = info_domain_data.sponsoring_registrar_handle !=
                                                 session_registrar_handle;
        info_domain_output_data.authinfopw =
                authinfopw_has_to_be_hidden ? boost::optional<std::string>() : info_domain_data.authinfopw;

        for (
            std::vector<Fred::ObjectIdHandlePair>::const_iterator object_id_handle_pair =
                    info_domain_data.admin_contacts.begin();
            object_id_handle_pair != info_domain_data.admin_contacts.end();
            ++object_id_handle_pair)
        {
            info_domain_output_data.admin.insert(object_id_handle_pair->handle);
        }

        info_domain_output_data.ext_enum_domain_validation =
                info_domain_data.enum_domain_validation.isnull()
                        ? Nullable<EnumValidationExtension>()
                        : EnumValidationExtension(
                                  info_domain_data.enum_domain_validation.get_value().validation_expiration,
                                  info_domain_data.enum_domain_validation.get_value().publish);
    }
    catch (const Fred::InfoDomainByHandle::Exception& e)
    {

        if (e.is_set_unknown_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        // in the improbable case that exception is incorrectly set
        throw;

    }
}


} // namesapce Epp::Domain
} // namespace Epp
