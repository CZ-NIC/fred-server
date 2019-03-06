/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef POLL_REQUEST_GET_UPDATE_DOMAIN_DETAILS_LOCALIZED_HH_8CC13E809A194209B3BBAF5E4988F418
#define POLL_REQUEST_GET_UPDATE_DOMAIN_DETAILS_LOCALIZED_HH_8CC13E809A194209B3BBAF5E4988F418

#include "src/backend/epp/domain/info_domain_localized_output_data.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/session_data.hh"

namespace Epp {
namespace Poll {

struct PollRequestUpdateDomainLocalizedOutputData
{
    Epp::Domain::InfoDomainLocalizedOutputData old_data;
    Epp::Domain::InfoDomainLocalizedOutputData new_data;

    PollRequestUpdateDomainLocalizedOutputData(
        const Epp::Domain::InfoDomainLocalizedOutputData& _old_data,
        const Epp::Domain::InfoDomainLocalizedOutputData& _new_data)
    :
        old_data(_old_data),
        new_data(_new_data)
    {}
};

struct PollRequestUpdateDomainLocalizedResponse
{
    EppResponseSuccessLocalized epp_response;
    PollRequestUpdateDomainLocalizedOutputData data;

    PollRequestUpdateDomainLocalizedResponse(
        const EppResponseSuccessLocalized& _epp_response,
        const PollRequestUpdateDomainLocalizedOutputData& _data)
    :
        epp_response(_epp_response),
        data(_data)
    {}
};

PollRequestUpdateDomainLocalizedResponse poll_request_get_update_domain_details_localized(
    unsigned long long _message_id,
    const SessionData& _session_data);

} // namespace Epp::Poll
} // namespace Epp

#endif
