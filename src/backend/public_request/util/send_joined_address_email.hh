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
#ifndef SEND_JOINED_ADDRESS_EMAIL_HH_AE7242E0D3EE4527ADDE0B27DE242FE7
#define SEND_JOINED_ADDRESS_EMAIL_HH_AE7242E0D3EE4527ADDE0B27DE242FE7

#include "libhermes/struct.hh"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>

#include <boost/optional/optional.hpp>
#include <boost/uuid/uuid.hpp>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Util {

struct FailedToSendMailToRecipient : std::exception
{
    const char* what() const noexcept
    {
        return "failed to send mail to recipient";
    }
};

struct EmailData
{
    struct Recipient
    {
        std::string email;
        boost::optional<boost::uuids::uuid> uuid;
    };

    EmailData(
            const std::vector<Recipient>& _recipients,
            const std::string& _type,
            const std::string& _template_name_subject,
            const std::string& _template_name_body,
            const LibHermes::Struct& _template_parameters,
            const std::vector<boost::uuids::uuid>& _attachments)
        : recipients(_recipients),
          type(_type),
          template_name_subject(_template_name_subject),
          template_name_body(_template_name_body),
          template_parameters(_template_parameters),
          attachments(_attachments)
    {
    }
    const std::vector<Recipient> recipients;
    const std::string type;
    const std::string template_name_subject;
    const std::string template_name_body;
    const LibHermes::Struct template_parameters;
    const std::vector<boost::uuids::uuid> attachments;
};

unsigned long long send_joined_addresses_email(
        const std::string& _messenger_endpoint,
        bool _archive,
        const EmailData& data);

} // namespace Fred::Backend::PublicRequest::Util
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
