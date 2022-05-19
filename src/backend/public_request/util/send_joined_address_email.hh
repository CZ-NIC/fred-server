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

#include "src/backend/public_request/object_type.hh"

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
        bool operator<(const Recipient& _recipient) const;
    };

    EmailData(
            const std::set<Recipient>& _recipients,
            const std::string& _type,
            const std::string& _template_name_subject,
            const std::string& _template_name_body,
            const LibHermes::Struct& _template_parameters,
            ObjectType object_type, 
            boost::uuids::uuid _object_uuid,
            const std::vector<boost::uuids::uuid>& _attachments);

    const std::set<Recipient> recipients;
    const std::string type;
    const std::string template_name_subject;
    const std::string template_name_body;
    const LibHermes::Struct template_parameters;
    const ObjectType object_type;
    const boost::uuids::uuid object_uuid;
    const std::vector<boost::uuids::uuid> attachments;
};

void send_joined_addresses_email(
        const std::string& _messenger_endpoint,
        bool _archive,
        const EmailData& data);

} // namespace Fred::Backend::PublicRequest::Util
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
