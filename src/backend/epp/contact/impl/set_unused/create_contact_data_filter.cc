/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/contact/impl/set_unused/create_contact_data_filter.hh"

#include <stdexcept>

namespace Epp {
namespace Contact {
namespace Impl {
namespace SetUnused {

namespace {

template <typename T>
bool is_public(const Hideable<T>& hideable_data, CreateContactDataFilter::Data default_publishability)
{
    if (hideable_data.is_publishability_specified())
    {
        if (hideable_data.is_public())
        {
            return true;
        }
        if (hideable_data.is_private())
        {
            return false;
        }
        throw std::runtime_error("data is neither public nor private");
    }
    switch (default_publishability)
    {
        case CreateContactDataFilter::Data::show:
            return true;
        case CreateContactDataFilter::Data::hide:
            return false;
        case CreateContactDataFilter::Data::publishability_not_specified:
            throw std::runtime_error("publishability must be specified");
    }
    throw std::runtime_error("unknown default publishability");
}

struct ExceptionDiscloseflagRulesViolation:
        Epp::Contact::CreateContactDataFilter::DiscloseflagRulesViolation,
        std::runtime_error
{
    ExceptionDiscloseflagRulesViolation() : std::runtime_error("discloseflag rules violation") { }
};

}//namespace Epp::Contact::Impl::SetUnused::{anonymous}

CreateContactDataFilter::CreateContactDataFilter()
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

CreateContactDataFilter::CreateContactDataFilter(
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

LibFred::CreateContact& CreateContactDataFilter::operator()(
        LibFred::OperationContext& ctx,
        const CreateContactInputData& contact_data,
        const SessionData&,
        LibFred::CreateContact& create_op)const
{
    try
    {
        create_op.set_disclosename(is_public(contact_data.name, default_disclose_name_));
        create_op.set_discloseorganization(is_public(contact_data.organization, default_disclose_organization_));
        create_op.set_discloseaddress(is_public(contact_data.address, default_disclose_address_));
        create_op.set_disclosetelephone(is_public(contact_data.telephone, default_disclose_telephone_));
        create_op.set_disclosefax(is_public(contact_data.fax, default_disclose_fax_));
        create_op.set_discloseemail(is_public(contact_data.email, default_disclose_email_));
        create_op.set_disclosevat(is_public(contact_data.vat, default_disclose_vat_));
        create_op.set_discloseident(is_public(contact_data.ident, default_disclose_ident_));
        create_op.set_disclosenotifyemail(is_public(contact_data.notify_email, default_disclose_notify_email_));
    }
    catch (const std::exception& e)
    {
        ctx.get_log().info(std::string("in ") + __PRETTY_FUNCTION__ + " exception caught: " + e.what());
        throw;
    }
    return create_op;
}

}//namespace Epp::Contact::Impl::SetUnused
}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp
