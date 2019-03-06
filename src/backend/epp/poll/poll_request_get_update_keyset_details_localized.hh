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
#ifndef POLL_REQUEST_GET_UPDATE_KEYSET_DETAILS_LOCALIZED_HH_DCDF70B930C54C8D85020F64B173D53A
#define POLL_REQUEST_GET_UPDATE_KEYSET_DETAILS_LOCALIZED_HH_DCDF70B930C54C8D85020F64B173D53A

#include "src/backend/epp/keyset/info_keyset_localized_output_data.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/session_data.hh"

namespace Epp {
namespace Poll {

struct PollRequestUpdateKeysetLocalizedOutputData
{
    Epp::Keyset::InfoKeysetLocalizedOutputData old_data;
    Epp::Keyset::InfoKeysetLocalizedOutputData new_data;

    PollRequestUpdateKeysetLocalizedOutputData(
        const Epp::Keyset::InfoKeysetLocalizedOutputData& _old_data,
        const Epp::Keyset::InfoKeysetLocalizedOutputData& _new_data)
    :
        old_data(_old_data),
        new_data(_new_data)
    {}
};

struct PollRequestUpdateKeysetLocalizedResponse
{
    EppResponseSuccessLocalized epp_response;
    PollRequestUpdateKeysetLocalizedOutputData data;

    PollRequestUpdateKeysetLocalizedResponse(
        const EppResponseSuccessLocalized& _epp_response,
        const PollRequestUpdateKeysetLocalizedOutputData& _data)
    :
        epp_response(_epp_response),
        data(_data)
    {}
};

PollRequestUpdateKeysetLocalizedResponse poll_request_get_update_keyset_details_localized(
    unsigned long long _message_id,
    const SessionData& _session_data);

} // namespace Epp::Poll
} // namespace Epp

#endif
