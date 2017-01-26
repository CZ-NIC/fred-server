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

#ifndef POLL_ACKNOWLEDGEMENT_LOCALIZED_H_B65E41A7524C4C0097F69AAF8EC1459F
#define POLL_ACKNOWLEDGEMENT_LOCALIZED_H_B65E41A7524C4C0097F69AAF8EC1459F

#include "src/epp/impl/session_lang.h"
#include "src/epp/impl/epp_response_success_localized.h"

#include <string>

namespace Epp {
namespace Poll {

struct PollAcknowledgementLocalizedOutputData
{
    unsigned long long count;
    std::string next_message_id;

    PollAcknowledgementLocalizedOutputData(
        unsigned long long _count,
        const std::string _next_message_id)
    :
        count(_count),
        next_message_id(_next_message_id)
    {}
};

struct PollAcknowledgementLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const PollAcknowledgementLocalizedOutputData poll_acknowledgement_localized_output_data;

    PollAcknowledgementLocalizedResponse(
        const EppResponseSuccessLocalized& _epp_response_success_localized,
        const PollAcknowledgementLocalizedOutputData& _poll_acknowledgement_localized_output_data)
    :
        epp_response_success_localized(_epp_response_success_localized),
        poll_acknowledgement_localized_output_data(_poll_acknowledgement_localized_output_data)
    {}
};

PollAcknowledgementLocalizedResponse poll_acknowledgement_localized(
    unsigned long long _message_id,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle);

} // namespace Epp::Poll
} // namespace Epp

#endif
