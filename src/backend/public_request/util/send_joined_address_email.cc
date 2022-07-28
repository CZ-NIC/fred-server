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

#include "util/log/logger.hh"

#include "libhermes/libhermes.hh"

#include <boost/algorithm/string/trim.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>

#include <tuple>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Util {

namespace {

std::map<LibHermes::Email::RecipientEmail, std::set<LibHermes::Email::RecipientUuid>> to_libhermes_recipients(const std::set<EmailData::Recipient>& _recipients)
{
    std::map<LibHermes::Email::RecipientEmail, std::set<LibHermes::Email::RecipientUuid>> result;
    for (const auto& recipient : _recipients)
    {
        if (recipient.uuid != boost::none)
        {
            result[LibHermes::Email::RecipientEmail{recipient.email}].insert(LibHermes::Email::RecipientUuid{*recipient.uuid});
        }
        else
        {
            result[LibHermes::Email::RecipientEmail{recipient.email}]; // create the record if does not exist yet
        }
    }
    return result;
}

LibHermes::Reference::Type to_libhermes_reference_type(ObjectType _object_type)
{
    switch (_object_type)
    {
        case ObjectType::contact: return LibHermes::Reference::Type{"contact"};
        case ObjectType::domain: return LibHermes::Reference::Type{"domain"};
        case ObjectType::nsset: return LibHermes::Reference::Type{"nsset"};
        case ObjectType::keyset: return LibHermes::Reference::Type{"keyset"};
    }
    throw std::logic_error("unexpected value of LibFred::Object_Type");
}

} // namespace Fred::Backend::PublicRequest::Util::{anonymous}

EmailData::EmailData(
        std::set<Recipient> _recipients,
        std::string _type,
        std::string _template_name_subject,
        std::string _template_name_body,
        LibHermes::Struct _template_parameters,
        ObjectType _object_type, 
        boost::uuids::uuid _object_uuid,
        boost::uuids::uuid _public_request_uuid,
        std::vector<boost::uuids::uuid> _attachments)
    : recipients(std::move(_recipients)),
      type(std::move(_type)),
      template_name_subject(std::move(_template_name_subject)),
      template_name_body(std::move(_template_name_body)),
      template_parameters(std::move(_template_parameters)),
      object_type(std::move(_object_type)),
      object_uuid(std::move(_object_uuid)),
      public_request_uuid(std::move(_public_request_uuid)),
      attachments(std::move(_attachments))
{
}

bool EmailData::Recipient::operator<(const EmailData::Recipient& _other) const
{
    return std::make_tuple(email, uuid) < std::make_tuple(_other.email, _other.uuid);
}

void send_joined_addresses_email(
        const std::string& _messenger_endpoint,
        bool _archive,
        const EmailData& data)
{
    LibHermes::Connection<LibHermes::Service::EmailMessenger> connection{
        LibHermes::Connection<LibHermes::Service::EmailMessenger>::ConnectionString{
            _messenger_endpoint}};

    auto email =
            LibHermes::Email::make_minimal_email(
                    to_libhermes_recipients(data.recipients),
                    LibHermes::Email::SubjectTemplate{data.template_name_subject},
                    LibHermes::Email::BodyTemplate{data.template_name_body});
    email.context = data.template_parameters;
    email.type = LibHermes::Email::Type{data.type};

    std::transform(
            data.attachments.begin(),
            data.attachments.end(),
            std::back_inserter(email.attachments),
            [](const auto& item)
            {
                return LibHermes::Email::AttachmentUuid{item};
            });

    try
    {
        LibHermes::Email::batch_send(
                connection,
                email,
                LibHermes::Email::Archive{_archive},
                {LibHermes::Reference{
                        to_libhermes_reference_type(data.object_type),
                        LibHermes::Reference::Value{boost::uuids::to_string(data.object_uuid)}},
                 LibHermes::Reference{
                        LibHermes::Reference::Type{"public-request"},
                        LibHermes::Reference::Value{boost::uuids::to_string(data.public_request_uuid)}}});
    }
    catch (const LibHermes::Email::SendFailed& e)
    {
        LOGGER.info(boost::str(boost::format("gRPC exception caught while sending email: gRPC error code: %1% error message: %2% grpc_message_json: %3%") %
                               e.error_code() % e.error_message() % e.grpc_message_json()));
        throw FailedToSendMailToRecipient();
    }
    catch (const std::exception& e)
    {
        LOGGER.info(boost::str(boost::format("std::exception caught while sending email: %1%") % e.what()));
        throw FailedToSendMailToRecipient();
    }
    catch (...)
    {
        LOGGER.info("exception caught while sending email");
        throw FailedToSendMailToRecipient();
    }
}

} // namespace Fred::Backend::PublicRequest::Util
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
