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

#include "src/backend/public_request/get_registry_emails_of_registered_object.hh"

#include "src/backend/public_request/object_type.hh"
#include "src/libfred/object/object_type.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/domain/info_domain.hh"
#include "src/libfred/registrable_object/keyset/info_keyset.hh"
#include "src/libfred/registrable_object/nsset/info_nsset.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {

namespace {

std::set<unsigned long long> get_registry_contacts_of_registered_object(
        LibFred::OperationContext& _ctx,
        ObjectType _object_type,
        unsigned long long _object_id)
{

    std::set<unsigned long long> contacts;

    switch (_object_type)
    {
        case ObjectType::contact:
            {
                contacts.insert(_object_id);
            }
            return contacts;

        case ObjectType::nsset:
            {
                const auto tech_contacts = LibFred::InfoNssetById(_object_id).exec(_ctx).info_nsset_data.tech_contacts;

                for (const auto& tech_contact : tech_contacts)
                {
                    contacts.insert(tech_contact.id);
                }
            }
            return contacts;

        case ObjectType::domain:
            {
                const auto info_domain_data = LibFred::InfoDomainById(_object_id).exec(_ctx).info_domain_data;
                const auto registrant = info_domain_data.registrant;
                const auto admin_contacts = info_domain_data.admin_contacts;

                contacts.insert(registrant.id);
                for (const auto& admin_contact : admin_contacts)
                {
                    contacts.insert(admin_contact.id);
                }
            }
            return contacts;

        case ObjectType::keyset:
            {
                const auto tech_contacts = LibFred::InfoKeysetById(_object_id).exec(_ctx).info_keyset_data.tech_contacts;

                for (const auto& tech_contact : tech_contacts)
                {
                    contacts.insert(tech_contact.id);
                }
            }
            return contacts;
    }
    throw std::runtime_error("unexpected ObjectType");

}

} // namespace Fred::Backend::PublicRequest::{anonymous}

std::set<std::string> get_registry_emails_of_registered_object(
        LibFred::OperationContext& _ctx,
        ObjectType _object_type,
        unsigned long long _object_id)
{

    const auto contacts = get_registry_contacts_of_registered_object(_ctx, _object_type, _object_id);

    std::set<std::string> emails;

    for (const auto& contact : contacts)
    {
        const auto email = LibFred::InfoContactById(contact).exec(_ctx).info_contact_data.email.get_value_or("");
        if (email != "")
        {
            emails.insert(email);
        }
    }

    return emails;
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
