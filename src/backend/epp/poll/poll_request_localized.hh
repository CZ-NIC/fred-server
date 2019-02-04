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

#ifndef POLL_REQUEST_LOCALIZED_HH_D06712522E9B4963A7287274A85A2F9D
#define POLL_REQUEST_LOCALIZED_HH_D06712522E9B4963A7287274A85A2F9D

#include "src/backend/epp/session_lang.hh"
#include "util/decimal/decimal.hh"
#include "libfred/opcontext.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/poll/poll_request.hh"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>
#include <vector>

namespace Epp {
namespace Poll {

struct PollRequestLocalizedOutputData
{
    unsigned long long message_id;
    boost::posix_time::ptime creation_time;
    unsigned long long number_of_unseen_messages;
    PollRequestOutputData::Message message;

    PollRequestLocalizedOutputData(
        unsigned long long _message_id,
        const boost::posix_time::ptime& _creation_time,
        unsigned long long _number_of_unseen_messages,
        const PollRequestOutputData::Message& _message)
    :
        message_id(_message_id),
        creation_time(_creation_time),
        number_of_unseen_messages(_number_of_unseen_messages),
        message(_message)
    {}
};

struct PollRequestLocalizedResponse
{
    EppResponseSuccessLocalized epp_response;
    PollRequestLocalizedOutputData data;

    PollRequestLocalizedResponse(
        const EppResponseSuccessLocalized& _epp_response,
        const PollRequestLocalizedOutputData& _data)
    :
        epp_response(_epp_response),
        data(_data)
    {}
};

PollRequestLocalizedResponse poll_request_localized(
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle);

} // namespace Epp::Poll
} // namespace Epp

#endif
