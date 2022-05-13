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
#include "src/backend/public_request/get_valid_registry_emails_of_registered_object.hh"

#include "libfred/contact_verification/django_email_format.hh"
#include "libfred/object/object_type.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "src/backend/public_request/object_type.hh"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <set>
#include <vector>
#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {

namespace {

std::set<unsigned long long> get_registry_contacts_of_registered_object(
        LibFred::OperationContext& _ctx,
        ObjectType _object_type,
        unsigned long long _object_id)
{
    std::set<unsigned long long> object_contacts;

    switch (_object_type)
    {
        case ObjectType::contact:
            {
                object_contacts.insert(_object_id);
            }
            return object_contacts;

        case ObjectType::nsset:
            {
                const auto tech_contacts = LibFred::InfoNssetById(_object_id).exec(_ctx).info_nsset_data.tech_contacts;

                for (const auto& contact : tech_contacts)
                {
                    object_contacts.insert(contact.id);
                }
            }
            return object_contacts;

        case ObjectType::domain:
            {
                const auto info_domain_data = LibFred::InfoDomainById(_object_id).exec(_ctx).info_domain_data;
                const auto registrant = info_domain_data.registrant;
                const auto admin_contacts = info_domain_data.admin_contacts;

                object_contacts.insert(registrant.id);
                for (const auto& contact : admin_contacts)
                {
                    object_contacts.insert(contact.id);
                }
            }
            return object_contacts;

        case ObjectType::keyset:
            {
                const auto tech_contacts = LibFred::InfoKeysetById(_object_id).exec(_ctx).info_keyset_data.tech_contacts;

                for (const auto& contact : tech_contacts)
                {
                    object_contacts.insert(contact.id);
                }
            }
            return object_contacts;
    }
    throw std::runtime_error("unexpected ObjectType");
}

} // namespace Fred::Backend::PublicRequest::{anonymous}

std::set<Util::EmailData::Recipient> get_valid_registry_emails_of_registered_object(
        LibFred::OperationContext& _ctx,
        ObjectType _object_type,
        unsigned long long _object_id)
{

    const auto object_contacts = get_registry_contacts_of_registered_object(_ctx, _object_type, _object_id);

    std::set<Util::EmailData::Recipient> recipients_with_valid_email;

    for (const auto contact_id : object_contacts)
    {
        const auto info_contact_data =
                LibFred::InfoContactById(contact_id).exec(_ctx).info_contact_data;

        std::vector<std::string> emails;
        boost::split(emails, info_contact_data.email.get_value_or_default(), boost::is_any_of(","));
        for (auto& email : emails)
        {
            const bool email_format_is_valid = DjangoEmailFormat().check(email);
            if (email_format_is_valid)
            {
                recipients_with_valid_email.push_back(Util::EmailData::Recipient{email, get_raw_value_from(info_contact_data.uuid)});
            }
        }
    }

    return recipients_with_valid_email;
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
