/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef POLL_REQUEST_GET_UPDATE_DOMAIN_DETAILS_LOCALIZED_H_38D1DA74972D48A78DAEF6DB61B14217
#define POLL_REQUEST_GET_UPDATE_DOMAIN_DETAILS_LOCALIZED_H_38D1DA74972D48A78DAEF6DB61B14217

#include "src/epp/domain/impl/info_domain_localized_output_data.h"
#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/session_data.h"

#include <string>

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
