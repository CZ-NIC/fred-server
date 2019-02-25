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
#ifndef POLL_REQUEST_GET_UPDATE_NSSET_DETAILS_LOCALIZED_HH_CD7B39E536924A2BACD9B58C99AFDAE6
#define POLL_REQUEST_GET_UPDATE_NSSET_DETAILS_LOCALIZED_HH_CD7B39E536924A2BACD9B58C99AFDAE6

#include "src/backend/epp/nsset/info_nsset_localized_output_data.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/session_data.hh"

namespace Epp {
namespace Poll {

struct PollRequestUpdateNssetLocalizedOutputData
{
    Epp::Nsset::InfoNssetLocalizedOutputData old_data;
    Epp::Nsset::InfoNssetLocalizedOutputData new_data;

    PollRequestUpdateNssetLocalizedOutputData(
        const Epp::Nsset::InfoNssetLocalizedOutputData& _old_data,
        const Epp::Nsset::InfoNssetLocalizedOutputData& _new_data)
    :
        old_data(_old_data),
        new_data(_new_data)
    {}
};

struct PollRequestUpdateNssetLocalizedResponse
{
    EppResponseSuccessLocalized epp_response;
    PollRequestUpdateNssetLocalizedOutputData data;

    PollRequestUpdateNssetLocalizedResponse(
        const EppResponseSuccessLocalized& _epp_response,
        const PollRequestUpdateNssetLocalizedOutputData& _data)
    :
        epp_response(_epp_response),
        data(_data)
    {}
};

PollRequestUpdateNssetLocalizedResponse poll_request_get_update_nsset_details_localized(
    unsigned long long _message_id,
    const SessionData& _session_data);

} // namespace Epp::Poll
} // namespace Epp

#endif
