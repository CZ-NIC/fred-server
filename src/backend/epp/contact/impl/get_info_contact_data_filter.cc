/*
 * Copyright (C) 2021  CZ.NIC, z. s. p. o.
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

#include "src/backend/epp/contact/impl/get_info_contact_data_filter.hh"
#include "src/backend/epp/contact/impl/info_contact.hh"
#include "src/backend/epp/contact/impl/info_contact_data_filter.hh"

#include <stdexcept>

namespace Epp {
namespace Contact {

namespace Impl {

namespace {

InfoContact::DataSharePolicy to_data_share_policy(const std::string& value)
{
    if (has_value<InfoContact::DataSharePolicy::cznic_specific>(value))
    {
        return InfoContact::DataSharePolicy::cznic_specific;
    }
    if (has_value<InfoContact::DataSharePolicy::show_all>(value))
    {
        return InfoContact::DataSharePolicy::show_all;
    }
    throw std::runtime_error("unable convert string to value of InfoContact::DataSharePolicy type");
}

template <typename T>
bool show_private_data_to(const std::string& relationship_name, InfoContactDataFilter::Relationships& enabled_registrars) noexcept
{
    if (has_value<T>(relationship_name))
    {
        std::get<InfoContactDataFilter::Bool<T>>(enabled_registrars) = true;
        return true;
    }
    return false;
}

}//namespace Epp::Contact::Impl::{anonymous}

std::shared_ptr<Epp::Contact::InfoContactDataFilter> get_info_contact_data_filter(const ConfigDataFilter& filter)
{
    const auto data_share_policy_str = filter.get_value<Impl::InfoContact::DataSharePolicy>();
    if (data_share_policy_str.empty())
    {
        Impl::InfoContactDataFilter::Relationships enabled_registrars{};
        const auto on_registrars_role = [&](const std::string& relationship_name)
        {
            if (show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>(
                        relationship_name, enabled_registrars) ||
                show_private_data_to<ContactRegistrarRelationship::AuthorizedRegistrar>(
                        relationship_name, enabled_registrars) ||
                show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>(
                        relationship_name, enabled_registrars) ||
                show_private_data_to<ContactRegistrarRelationship::OtherRelationship>(
                        relationship_name, enabled_registrars) ||
                show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrar>(
                        relationship_name, enabled_registrars) ||
                show_private_data_to<ContactRegistrarRelationship::SystemRegistrar>(
                        relationship_name, enabled_registrars))
            {
                return;
            }
            struct InvalidRelationshipName : std::runtime_error
            {
                InvalidRelationshipName() : std::runtime_error{"Invalid relationship name"} { }
            };
            throw InvalidRelationshipName{};
        };
        filter.iterate_multiple_value<Impl::InfoContact::ShowPrivateDataTo>(on_registrars_role);
        return std::make_shared<Impl::InfoContactDataFilter>(enabled_registrars);
    }
    return std::make_shared<Impl::InfoContactDataFilter>(to_data_share_policy(data_share_policy_str));
}

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp
