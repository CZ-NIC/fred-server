/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/backend/epp/domain/delete_domain.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/impl/util.hh"
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/domain.hh"
#include "libfred/registrable_object/domain/domain_name.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/registrar_zone_access.hh"
#include "libfred/zone/zone.hh"
#include "libfred/poll/create_poll_message.hh"
#include "libfred/poll/message_type.hh"

#include <boost/date_time/gregorian/greg_date.hpp>

#include <string>

namespace Epp {
namespace Domain {

unsigned long long delete_domain(
        LibFred::OperationContext& _ctx,
        const std::string& _domain_fqdn,
        const DeleteDomainConfigData& _delete_domain_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    LibFred::Zone::Data zone_data;
    try
    {
        zone_data = LibFred::Zone::find_zone_in_fqdn(
                _ctx,
                LibFred::Zone::rem_trailing_dot(_domain_fqdn));
    }
    catch (const LibFred::Zone::Exception& e)
    {
        if (e.is_set_unknown_zone_in_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }

    // start of db transaction, utc timestamp without timezone, will be timestamp of domain creation crdate
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
            static_cast<std::string>(_ctx.get_conn().exec(
                    "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));

    // warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time =
        boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    if (!LibFred::is_zone_accessible_by_registrar(_session_data.registrar_id, zone_data.id, current_local_date, _ctx))
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error));
    }

    LibFred::InfoDomainData domain_data_before_delete;
    try
    {
        domain_data_before_delete =
                LibFred::InfoDomainByFqdn(_domain_fqdn)
                        .set_lock()
                        .exec(_ctx)
                        .info_domain_data;
    }
    catch (const LibFred::InfoDomainByFqdn::Exception& ex)
    {
        if (ex.is_set_unknown_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }

    const LibFred::InfoRegistrarData session_registrar =
        LibFred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data;

    const bool is_sponsoring_registrar = (domain_data_before_delete.sponsoring_registrar_handle ==
                                          session_registrar.handle);
    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    const bool is_registrar_authorized = (is_sponsoring_registrar || is_system_registrar);

    if (!is_registrar_authorized)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::registrar_autor,
                                                         Reason::unauthorized_registrar)));
    }

    if (!is_system_registrar)
    {
        const LibFred::ObjectStatesInfo domain_states(LibFred::GetObjectStates(domain_data_before_delete.id).exec(
                        _ctx));
        if (domain_states.presents(LibFred::Object_State::server_update_prohibited) ||
            domain_states.presents(LibFred::Object_State::server_delete_prohibited) ||
            domain_states.presents(LibFred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    try
    {
        LibFred::DeleteDomainByFqdn(_domain_fqdn).exec(_ctx);
        if (!is_sponsoring_registrar)
        {
            LibFred::Poll::CreatePollMessage<::LibFred::Poll::MessageType::delete_domain>()
                .exec(_ctx, domain_data_before_delete.historyid);
        }
        return domain_data_before_delete.historyid;
    }
    catch (const LibFred::DeleteDomainByFqdn::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_domain_fqdn())
        {
            throw;
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}


} // namespace Epp::Domain
} // namespace Epp
