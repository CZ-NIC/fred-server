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

#include "src/epp/domain/transfer_domain.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/domain/domain.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/transfer_domain.h"
#include "src/fredlib/exception.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object/object_states_info.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/poll/create_poll_message.h"
#include "src/fredlib/poll/message_type.h"
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
        const std::string& _fqdn,
        const std::string& _authinfopw,
        const TransferDomainConfigData& _transfer_domain_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data)) {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    // start of db transaction, utc timestamp without timezone, will be timestamp of domain creation crdate
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
        static_cast<std::string>(_ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));

    // warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    Fred::Zone::Data zone_data;
    try {
        zone_data = Fred::Zone::find_zone_in_fqdn(_ctx,
            Fred::Zone::rem_trailing_dot(_fqdn));
    } catch (const Fred::Zone::Exception& e) {
        if (e.is_set_unknown_zone_in_fqdn()) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
        /* in the improbable case that exception is incorrectly set */
        throw;
    }

    if (!Fred::is_zone_accessible_by_registrar(_session_data.registrar_id, zone_data.id, current_local_date, _ctx)) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error));
    }

    Fred::InfoDomainData domain_data_before_transfer;
    try
    {
        domain_data_before_transfer = Fred::InfoDomainByHandle(Fred::Zone::rem_trailing_dot(_fqdn))
                                              .set_lock().exec(_ctx).info_domain_data;
    }
    catch(const Fred::InfoDomainByHandle::Exception& ex)
    {
        if (ex.is_set_unknown_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }

    const Fred::InfoRegistrarData session_registrar =
        Fred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data;

    const bool is_sponsoring_registrar = (domain_data_before_transfer.sponsoring_registrar_handle ==
                                          session_registrar.handle);
    if (is_sponsoring_registrar) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_is_not_eligible_for_transfer));
    }

    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    if (!is_system_registrar)
    {
        const Fred::ObjectStatesInfo domain_states_before_transfer(Fred::GetObjectStates(domain_data_before_transfer.id).exec(_ctx));

        if (domain_states_before_transfer.presents(Fred::Object_State::server_transfer_prohibited) ||
            domain_states_before_transfer.presents(Fred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    try {
        const unsigned long long post_transfer_history_id =
            Fred::TransferDomain(
                domain_data_before_transfer.id,
                session_registrar.handle,
                _authinfopw,
                _session_data.logd_request_id.isset() ? _session_data.logd_request_id.get_value() : Nullable<unsigned long long>()
            ).exec(_ctx);

        Fred::Poll::CreatePollMessage<Fred::Poll::MessageType::transfer_domain>()
                .exec(_ctx, post_transfer_history_id);

        return post_transfer_history_id;

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
