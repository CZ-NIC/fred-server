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

#include "src/backend/epp/contact/impl/info_contact_data_filter.hh"

#include "libfred/registrar/info_registrar.hh"

#include "util/log/logger.hh"

#include <stdexcept>

namespace Epp {
namespace Contact {
namespace Impl {

namespace {

template <typename T>
constexpr auto enable() noexcept
{
    return InfoContactDataFilter::Bool<T>{true};
}

template <typename T>
constexpr auto disable() noexcept
{
    return InfoContactDataFilter::Bool<T>{false};
}

InfoContactDataFilter::Relationships enabled_relationships(InfoContact::DataSharePolicy data_share_policy)
{
    switch (data_share_policy)
    {
        case InfoContact::DataSharePolicy::cznic_specific:
            return InfoContactDataFilter::Relationships{
                    enable<ContactRegistrarRelationship::SponsoringRegistrar>(),
                    enable<ContactRegistrarRelationship::AuthorizedRegistrar>(),
                    enable<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>(),
                    enable<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>(),
                    disable<ContactRegistrarRelationship::OtherRelationship>()};
        case InfoContact::DataSharePolicy::show_all:
            return InfoContactDataFilter::Relationships{
                    enable<ContactRegistrarRelationship::SponsoringRegistrar>(),
                    enable<ContactRegistrarRelationship::AuthorizedRegistrar>(),
                    enable<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>(),
                    enable<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>(),
                    enable<ContactRegistrarRelationship::OtherRelationship>()};
    }
    throw std::runtime_error{"unexpected DataSharePolicy value"};
}

template <typename T>
constexpr bool does_present(const InfoContactDataFilter::Relationships& relationships) noexcept
{
    return std::get<InfoContactDataFilter::Bool<T>>(relationships);
}

bool is_visibility_restricted(const InfoContactDataFilter::Relationships& show_private_data_to)
{
    return !does_present<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>(show_private_data_to) ||
           !does_present<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>(show_private_data_to) ||
           !does_present<ContactRegistrarRelationship::OtherRelationship>(show_private_data_to) ||
           !does_present<ContactRegistrarRelationship::SponsoringRegistrar>(show_private_data_to);
}

bool is_authorized_registrar(
        const boost::optional<std::string>& contact_authinfopw,
        const LibFred::InfoContactData& contact_data)
{
    if (contact_authinfopw != boost::none)
    {
        if (contact_data.authinfopw != *contact_authinfopw)
        {
            struct AuthinfoDoesNotMatch : InfoContactDataFilter::InvalidAuthorizationInformation, std::exception
            {
                const char* what() const noexcept override { return "authinfo does not match"; }
            };
            throw AuthinfoDoesNotMatch{};
        }
        return true;
    }
    return false;
}

bool registrar_is_contacts_sponsoring_registrar(
        LibFred::OperationContext& ctx,
        unsigned long long registrar_id,
        const std::string& contact_sponsoring_registrar_handle)
{
    const auto contact_sponsoring_registrar_id = LibFred::InfoRegistrarByHandle{contact_sponsoring_registrar_handle}
            .exec(ctx).info_registrar_data.id;
    return registrar_id  == contact_sponsoring_registrar_id;
}

bool contact_is_registrars_domain_holder(
        LibFred::OperationContext& ctx,
        unsigned long long registrar_id,
        unsigned long long contact_id)
{
    return 0 < ctx.get_conn().exec_params(
            "SELECT 0 "
            "FROM object d_o "
            "JOIN domain d ON d.id = d_o.id AND "
                             "d_o.clid = $1::BIGINT AND "
                             "d.registrant = $2::BIGINT "
            "LIMIT 1",
            Database::query_param_list(registrar_id)(contact_id)).size();
}

bool contact_is_registrars_domain_admin_contact(
        LibFred::OperationContext& ctx,
        unsigned long long registrar_id,
        unsigned long long contact_id)
{
    return 0 < ctx.get_conn().exec_params(
            "SELECT 0 "
            "FROM object d_o "
            "JOIN domain d ON d.id = d_o.id AND "
                             "d_o.clid = $1::BIGINT "
            "JOIN domain_contact_map dcm ON dcm.domainid = d.id AND "
                                           "dcm.contactid = $2::BIGINT AND "
                                           "dcm.role = 1 " // 1 means "admin contact", there are currently no other roles.
            "LIMIT 1",
            Database::query_param_list(registrar_id)(contact_id)).size();
}

template <typename T>
void set_null(Nullable<T>& value)
{
    value = Nullable<T>{};
}

void hide_private_data(LibFred::InfoContactData& contact_data)
{
    if (!contact_data.discloseaddress)
    {
        contact_data.addresses.clear();
        set_null(contact_data.place);
    }
    if (!contact_data.discloseemail)
    {
        set_null(contact_data.email);
    }
    if (!contact_data.disclosefax)
    {
        set_null(contact_data.fax);
    }
    if (!contact_data.discloseident)
    {
        set_null(contact_data.ssn);
        set_null(contact_data.ssntype);
    }
    if (!contact_data.disclosename)
    {
        set_null(contact_data.name);
    }
    if (!contact_data.disclosenotifyemail)
    {
        set_null(contact_data.notifyemail);
    }
    if (!contact_data.discloseorganization)
    {
        set_null(contact_data.organization);
    }
    if (!contact_data.disclosetelephone)
    {
        set_null(contact_data.telephone);
    }
    if (!contact_data.disclosevat)
    {
        set_null(contact_data.vat);
    }
}

template <typename> std::string relationship_name();

template <>
std::string relationship_name<ContactRegistrarRelationship::AuthorizedRegistrar>()
{
    return "AuthorizedRegistrar";
}

template <>
std::string relationship_name<ContactRegistrarRelationship::SponsoringRegistrar>()
{
    return "SponsoringRegistrar";
}

template <>
std::string relationship_name<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>()
{
    return "SponsoringRegistrarOfDomainWhereContactIs::AdminContact";
}

template <>
std::string relationship_name<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>()
{
    return "SponsoringRegistrarOfDomainWhereContactIs::DomainHolder";
}

template <>
std::string relationship_name<ContactRegistrarRelationship::OtherRelationship>()
{
    return "OtherRelationship";
}

template <typename ...Relationships> struct Append;

template <typename First, typename ...Tail>
struct Append<First, Tail...>
{
    template <typename ...Ts>
    static std::string& to_string(const std::tuple<InfoContactDataFilter::Bool<Ts>...>& relationships, std::string& str)
    {
        if (std::get<InfoContactDataFilter::Bool<First>>(relationships))
        {
            if (!str.empty())
            {
                str += ", ";
            }
            str += relationship_name<First>();
        }
        return Append<Tail...>::to_string(relationships, str);
    }
};

template <>
struct Append<>
{
    template <typename ...Ts>
    static std::string& to_string(const std::tuple<InfoContactDataFilter::Bool<Ts>...>&, std::string& str)
    {
        return str;
    }
};

template <typename ...Relationships>
std::string to_string(const std::tuple<InfoContactDataFilter::Bool<Relationships>...>& relationships)
{
    std::string result;
    Append<Relationships...>::to_string(relationships, result);
    return result;
}

}//namespace Epp::Contact::Impl::CzNic::{anonymous}

InfoContactDataFilter::InfoContactDataFilter(const Relationships& show_private_data_to)
    : show_private_data_to_{show_private_data_to}
{
    LOGGER.info("InfoContactDataFilter(" + to_string(show_private_data_to_) + ")");
}

InfoContactDataFilter::InfoContactDataFilter(InfoContact::DataSharePolicy data_share_policy)
    : InfoContactDataFilter{enabled_relationships(data_share_policy)}
{ }

template <typename T>
bool InfoContactDataFilter::show_private_data_to() const noexcept
{
    return does_present<T>(show_private_data_to_);
}

LibFred::InfoContactData& InfoContactDataFilter::operator()(
        LibFred::OperationContext& ctx,
        const boost::optional<std::string>& contact_authinfopw,
        const SessionData& session_data,
        LibFred::InfoContactData& contact_data) const
{
    try
    {
        bool show_private_data = false;
        if (this->show_private_data_to<ContactRegistrarRelationship::AuthorizedRegistrar>() && is_authorized_registrar(contact_authinfopw, contact_data))
        {
            show_private_data = true;
        }
        const bool is_sponsoring_registrar = registrar_is_contacts_sponsoring_registrar(
                    ctx,
                    session_data.registrar_id,
                    contact_data.sponsoring_registrar_handle);
        if (!is_sponsoring_registrar)
        {
            contact_data.authinfopw.clear();
        }
        if (!show_private_data && is_visibility_restricted(show_private_data_to_))
        {
            const auto is_domain_holder = [&]()
                {
                    return contact_is_registrars_domain_holder(ctx, session_data.registrar_id, contact_data.id);
                };
            const auto is_admin_contact = [&]()
                {
                    return contact_is_registrars_domain_admin_contact(ctx, session_data.registrar_id, contact_data.id);
                };
            show_private_data =
                    (this->show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrar>() &&
                     is_sponsoring_registrar) ||
                    (this->show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>() &&
                     is_domain_holder()) ||
                    (this->show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>() &&
                     is_admin_contact());
            if (!show_private_data && this->show_private_data_to<ContactRegistrarRelationship::OtherRelationship>())
            {
                show_private_data =
                        (this->show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrar>() ||
                         !is_sponsoring_registrar) &&
                        (this->show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>() ||
                         !is_domain_holder()) &&
                        (this->show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>() ||
                         !is_admin_contact());
            }
            if (!show_private_data)
            {
                LOGGER.info("do not show private data of contact " + contact_data.handle + " to registrar " + std::to_string(session_data.registrar_id));
                hide_private_data(contact_data);
            }
        }
    }
    catch (const std::exception& e)
    {
        ctx.get_log().info(std::string{"in "} + __PRETTY_FUNCTION__ + " exception caught: " + e.what());
        throw;
    }
    return contact_data;
}

template bool InfoContactDataFilter::show_private_data_to<ContactRegistrarRelationship::AuthorizedRegistrar>() const noexcept;
template bool InfoContactDataFilter::show_private_data_to<ContactRegistrarRelationship::OtherRelationship>() const noexcept;
template bool InfoContactDataFilter::show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrar>() const noexcept;
template bool InfoContactDataFilter::show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>() const noexcept;
template bool InfoContactDataFilter::show_private_data_to<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>() const noexcept;

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp
