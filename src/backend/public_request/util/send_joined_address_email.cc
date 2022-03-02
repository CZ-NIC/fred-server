/*
 * Copyright (C) 2018-2022  CZ.NIC, z. s. p. o.
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

#include "libhermes/libhermes.hh"

#include <boost/algorithm/string/trim.hpp>

#include <sstream>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Util {

unsigned long long send_joined_addresses_email(
        const std::string& _messenger_endpoint,
        const EmailData& data)
{
    LibHermes::Connection<LibHermes::Service::EmailMessenger> connection{
        LibHermes::Connection<LibHermes::Service::EmailMessenger>::ConnectionString{
            _messenger_endpoint}};

    for (const auto& recipient_email_address : data.recipient_email_addresses)
    {
        auto email =
                LibHermes::Email::make_minimal_email(
                        LibHermes::Email::Email::Recipient{boost::algorithm::trim_copy(recipient_email_address)},
                        LibHermes::Email::Email::SubjectTemplate{data.template_name_subject},
                        LibHermes::Email::Email::BodyTemplate{data.template_name_body});

        std::transform(
                data.template_parameters.begin(),
                data.template_parameters.end(),
                std::inserter(email.context, email.context.end()),
                [](const auto& item)
                {
                    return std::make_pair(LibHermes::StructKey{item.first}, LibHermes::StructValue{item.second});
                });

        std::transform(
                data.attachments.begin(),
                data.attachments.end(),
                std::back_inserter(email.attachments),
                [](const auto& item)
                {
                    return LibHermes::Email::Email::AttachmentUuid{item};
                });

        LibHermes::Email::EmailUid email_uid;
        try
        {
            email_uid =
                    LibHermes::Email::send(
                            connection,
                            email,
                            LibHermes::Email::Archive{false},
                            {});
        }
        catch (const std::exception& e)
        {
            //LIBLOG_WARNING("std::exception caught while sending email: {}", e.what());
            continue;
        }
        catch (...)
        {
            //LIBLOG_WARNING("exception caught while sending email");
            continue;
        }
        // throw FailedToSendMailToRecipient();
        // return email_uids
    }
}

} // namespace Fred::Backend::PublicRequest::Util
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
