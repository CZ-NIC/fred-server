/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef POLL_REQUEST_GET_UPDATE_CONTACT_DETAILS_LOCALIZED_HH_A6C364EF5DFD42C99D25EB99BFF3B9CF
#define POLL_REQUEST_GET_UPDATE_CONTACT_DETAILS_LOCALIZED_HH_A6C364EF5DFD42C99D25EB99BFF3B9CF

#include "src/backend/epp/contact/info_contact_localized_output_data.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/session_data.hh"

namespace Epp {
namespace Poll {

struct PollRequestUpdateContactLocalizedOutputData
{
    Epp::Contact::InfoContactLocalizedOutputData old_data;
    Epp::Contact::InfoContactLocalizedOutputData new_data;


    PollRequestUpdateContactLocalizedOutputData(
            const Epp::Contact::InfoContactLocalizedOutputData& _old_data,
            const Epp::Contact::InfoContactLocalizedOutputData& _new_data)
        : old_data(_old_data),
          new_data(_new_data)
    {
    }

};

struct PollRequestUpdateContactLocalizedResponse
{
    EppResponseSuccessLocalized epp_response;
    PollRequestUpdateContactLocalizedOutputData data;


    PollRequestUpdateContactLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response,
            const PollRequestUpdateContactLocalizedOutputData& _data)
        : epp_response(_epp_response),
          data(_data)
    {
    }

};

PollRequestUpdateContactLocalizedResponse poll_request_get_update_contact_details_localized(
        unsigned long long _message_id,
        const SessionData& _session_data);


} // namespace Epp::Poll
} // namespace Epp

#endif
