/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/public_request/util/send_joined_address_email.hh"

#include <boost/algorithm/string/trim.hpp>

#include <sstream>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Util {

unsigned long long send_joined_addresses_email(
        std::shared_ptr<LibFred::Mailer::Manager> mailer,
        const EmailData& data)
{
    std::ostringstream recipients;
    for (const auto& email: data.recipient_email_addresses)
    {
        recipients << boost::trim_copy(email) << ' ';
    }

    try
    {
        return mailer->sendEmail(
                "",
                recipients.str(),
                "",
                data.template_name,
                data.template_parameters,
                LibFred::Mailer::Handles(),
                data.attachments);
    }
    catch (const LibFred::Mailer::NOT_SEND&)
    {
        throw FailedToSendMailToRecipient();
    }
}

} // namespace Fred::Backend::PublicRequest::Util
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
