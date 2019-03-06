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
#ifndef POLL_ACKNOWLEDGEMENT_LOCALIZED_HH_B52DB6A4EC8940218F81EBF5AC350C1D
#define POLL_ACKNOWLEDGEMENT_LOCALIZED_HH_B52DB6A4EC8940218F81EBF5AC350C1D

#include "src/backend/epp/session_lang.hh"
#include "src/backend/epp/epp_response_success_localized.hh"

#include <string>

namespace Epp {
namespace Poll {

struct PollAcknowledgementLocalizedOutputData
{
    unsigned long long number_of_unseen_messages;
    std::string oldest_unseen_message_id;

    PollAcknowledgementLocalizedOutputData(
        unsigned long long _number_of_unseen_messages,
        const std::string& _oldest_unseen_message_id)
    :
        number_of_unseen_messages(_number_of_unseen_messages),
        oldest_unseen_message_id(_oldest_unseen_message_id)
    {}
};

struct PollAcknowledgementLocalizedResponse
{
    EppResponseSuccessLocalized epp_response;
    PollAcknowledgementLocalizedOutputData data;

    PollAcknowledgementLocalizedResponse(
        const EppResponseSuccessLocalized& _epp_response,
        const PollAcknowledgementLocalizedOutputData& _data)
    :
        epp_response(_epp_response),
        data(_data)
    {}
};

PollAcknowledgementLocalizedResponse poll_acknowledgement_localized(
    const std::string& _message_id,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle);

} // namespace Epp::Poll
} // namespace Epp

#endif
