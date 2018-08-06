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

#include "src/backend/epp/contact/impl/cznic/create_contact_check.hh"

#include <stdexcept>

namespace Epp {
namespace Contact {
namespace Impl {
namespace CzNic {

namespace {

enum class Public
{
    no,
    yes
};

template <typename T>
Public compute_publishability(
        const Hideable<T>& hideable_data,
        CreateContactCheck::Data default_publishability)
{
    if (hideable_data.is_publishability_specified())
    {
        if (hideable_data.is_public())
        {
            return Public::yes;
        }
        if (hideable_data.is_private())
        {
            return Public::no;
        }
        throw std::runtime_error("data is neither public nor private");
    }
    switch (default_publishability)
    {
        case CreateContactCheck::Data::show:
            return Public::yes;
        case CreateContactCheck::Data::hide:
            return Public::no;
        case CreateContactCheck::Data::publishability_not_specified:
            throw std::runtime_error("publishability must be specified");
    }
    throw std::runtime_error("unknown default publishability");
}

template <typename> struct Constraint;

template <typename C, typename T>
bool is_public(const Hideable<T>& hideable_data, CreateContactCheck::Data default_publishability)
{
    const auto publishability = compute_publishability(hideable_data, default_publishability);
    switch (publishability)
    {
        case Public::no:
        case Public::yes:
            Constraint<C>::apply(publishability);
            return publishability == Public::yes;
    }
    throw std::runtime_error("unknown publishability");
}

struct PublicIsMandatory;
struct AnyIsAvailable;

struct ExceptionDiscloseflagRulesViolation:
        Epp::Contact::CreateOperationCheck::DiscloseflagRulesViolation,
        std::runtime_error
{
    ExceptionDiscloseflagRulesViolation():std::runtime_error("discloseflag rules violation") { }
};

template <>
struct Constraint<PublicIsMandatory>
{
    static void apply(Public publishability)
    {
        switch (publishability)
        {
            case Public::no:
                throw ExceptionDiscloseflagRulesViolation();
            case Public::yes:
                return;
        }
        throw std::runtime_error("unknown publishability");
    }
};

template <>
struct Constraint<AnyIsAvailable>
{
    static void apply(Public publishability)
    {
        switch (publishability)
        {
            case Public::no:
            case Public::yes:
                return;
        }
        throw std::runtime_error("unknown publishability");
    }
};

}//namespace Epp::Contact::Impl::CzNic::{anonymous}

CreateContactCheck::CreateContactCheck()
    : default_disclose_name_(Data::publishability_not_specified),
      default_disclose_organization_(Data::publishability_not_specified),
      default_disclose_address_(Data::publishability_not_specified),
      default_disclose_telephone_(Data::publishability_not_specified),
      default_disclose_fax_(Data::publishability_not_specified),
      default_disclose_email_(Data::publishability_not_specified),
      default_disclose_vat_(Data::publishability_not_specified),
      default_disclose_ident_(Data::publishability_not_specified),
      default_disclose_notify_email_(Data::publishability_not_specified)
{ }

CreateContactCheck::CreateContactCheck(
        Data default_disclose_name,
        Data default_disclose_organization,
        Data default_disclose_address,
        Data default_disclose_telephone,
        Data default_disclose_fax,
        Data default_disclose_email,
        Data default_disclose_vat,
        Data default_disclose_ident,
        Data default_disclose_notify_email)
    : default_disclose_name_(default_disclose_name),
      default_disclose_organization_(default_disclose_organization),
      default_disclose_address_(default_disclose_address),
      default_disclose_telephone_(default_disclose_telephone),
      default_disclose_fax_(default_disclose_fax),
      default_disclose_email_(default_disclose_email),
      default_disclose_vat_(default_disclose_vat),
      default_disclose_ident_(default_disclose_ident),
      default_disclose_notify_email_(default_disclose_notify_email)
{ }

LibFred::CreateContact& CreateContactCheck::operator()(
        LibFred::OperationContext& ctx,
        const CreateContactInputData& contact_data,
        const SessionData&,
        LibFred::CreateContact& create_op)const
{
    try
    {
        create_op.set_disclosename(
                is_public<PublicIsMandatory>(contact_data.name, default_disclose_name_));
        create_op.set_discloseorganization(
                is_public<PublicIsMandatory>(contact_data.organization, default_disclose_organization_));
        create_op.set_discloseaddress(
                is_public<PublicIsMandatory>(contact_data.address, default_disclose_address_));
        create_op.set_disclosetelephone(
                is_public<AnyIsAvailable>(contact_data.telephone, default_disclose_telephone_));
        create_op.set_disclosefax(
                is_public<AnyIsAvailable>(contact_data.fax, default_disclose_fax_));
        create_op.set_discloseemail(
                is_public<AnyIsAvailable>(contact_data.email, default_disclose_email_));
        create_op.set_disclosevat(
                is_public<AnyIsAvailable>(contact_data.vat, default_disclose_vat_));
        create_op.set_discloseident(
                is_public<AnyIsAvailable>(contact_data.ident, default_disclose_ident_));
        create_op.set_disclosenotifyemail(
                is_public<AnyIsAvailable>(contact_data.notify_email, default_disclose_notify_email_));
    }
    catch (const std::exception& e)
    {
        ctx.get_log().info(std::string("in ") + __PRETTY_FUNCTION__ + " exception caught: " + e.what());
        throw;
    }
    return create_op;
}

}//namespace Epp::Contact::Impl::CzNic
}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp
