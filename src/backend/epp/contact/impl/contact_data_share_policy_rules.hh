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
#ifndef CONTACT_DATA_SHARE_POLICY_RULES_HH_F1F64D7A27BD9690B617A87D5303B03A//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CONTACT_DATA_SHARE_POLICY_RULES_HH_F1F64D7A27BD9690B617A87D5303B03A

#include "src/backend/epp/contact/contact_data_share_policy_rules.hh"
#include "src/backend/epp/contact/impl/info_contact.hh"

#include <tuple>

namespace Epp {
namespace Contact {
namespace Impl {

struct ContactRegistrarRelationship
{
    struct SponsoringRegistrar;
    struct SponsoringRegistrarOfDomainWhereContactIs
    {
        struct DomainHolder;
        struct AdminContact;
    };
    struct AuthorizedRegistrar;
    struct SystemRegistrar;
    struct OtherRelationship;
};

class ContactDataSharePolicyRules final : public Epp::Contact::ContactDataSharePolicyRules
{
public:
    template <typename T>
    class Bool
    {
    public:
        constexpr explicit Bool(bool value = false) noexcept : value_{value} { }
        constexpr operator bool() const noexcept { return value_; }
        Bool& operator=(bool value) noexcept { value_ = value; return *this; }
    private:
        bool value_;
    };
    using Relationships = std::tuple<
            Bool<ContactRegistrarRelationship::SponsoringRegistrar>,
            Bool<ContactRegistrarRelationship::AuthorizedRegistrar>,
            Bool<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>,
            Bool<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>,
            Bool<ContactRegistrarRelationship::SystemRegistrar>,
            Bool<ContactRegistrarRelationship::OtherRelationship>>;
    explicit ContactDataSharePolicyRules(const Relationships& show_private_data_to);
    explicit ContactDataSharePolicyRules(InfoContact::DataSharePolicy data_share_policy);
    template <typename>
    bool show_private_data_to() const noexcept;
private:
    LibFred::InfoContactData& apply(
            LibFred::OperationContext& ctx,
            const Password& contact_authinfopw,
            const SessionData& session_data,
            LibFred::InfoContactData& contact_data) const override;
    Relationships show_private_data_to_;
};

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//CONTACT_DATA_SHARE_POLICY_RULES_HH_F1F64D7A27BD9690B617A87D5303B03A
