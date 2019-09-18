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
#include "src/backend/epp/contact/impl/set_unused/update_contact_data_filter.hh"

#include <stdexcept>

namespace Epp {
namespace Contact {
namespace Impl {
namespace SetUnused {

namespace {

enum class Public
{
    no,
    yes
};

Public compute_publishability(
        bool is_public,
        const boost::optional<PrivacyPolicy>& publishability,
        UpdateContactDataFilter::Operation default_operation)
{
    if (static_cast<bool>(publishability))
    {
        switch (*publishability)
        {
            case PrivacyPolicy::show:
                return Public::yes;
            case PrivacyPolicy::hide:
                return Public::no;
        }
        throw std::runtime_error("unknown publishability");
    }
    switch (default_operation)
    {
        case UpdateContactDataFilter::Operation::set_to_show:
            return Public::yes;
        case UpdateContactDataFilter::Operation::set_to_hide:
            return Public::no;
        case UpdateContactDataFilter::Operation::do_not_change:
            return is_public ? Public::yes : Public::no;
    }
    throw std::runtime_error("unknown publishability");
}

template <typename U>
Public get_privacy(
        U& updater,
        bool is_public,
        const boost::optional<PrivacyPolicy>& publishability,
        UpdateContactDataFilter::Operation default_operation,
        U&(LibFred::UpdateContact<U>::*privacy_setter)(bool))
{
    switch (compute_publishability(is_public, publishability, default_operation))
    {
        case Public::yes:
            (updater.*privacy_setter)(true);
            return Public::yes;
        case Public::no:
            (updater.*privacy_setter)(false);
            return Public::no;
    }
    throw std::runtime_error("unknown publishability");
}

}//namespace Epp::Contact::Impl::SetUnused::{anonymous}

UpdateContactDataFilter::UpdateContactDataFilter()
    : default_disclose_name_(Operation::do_not_change),
      default_disclose_organization_(Operation::do_not_change),
      default_disclose_address_(Operation::do_not_change),
      default_disclose_telephone_(Operation::do_not_change),
      default_disclose_fax_(Operation::do_not_change),
      default_disclose_email_(Operation::do_not_change),
      default_disclose_vat_(Operation::do_not_change),
      default_disclose_ident_(Operation::do_not_change),
      default_disclose_notify_email_(Operation::do_not_change)
{ }

UpdateContactDataFilter::UpdateContactDataFilter(
        Operation default_disclose_name,
        Operation default_disclose_organization,
        Operation default_disclose_address,
        Operation default_disclose_telephone,
        Operation default_disclose_fax,
        Operation default_disclose_email,
        Operation default_disclose_vat,
        Operation default_disclose_ident,
        Operation default_disclose_notify_email)
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

LibFred::UpdateContactByHandle& UpdateContactDataFilter::operator()(
        LibFred::OperationContext&,
        const LibFred::InfoContactData& old_data,
        const ContactChange& change,
        const SessionData&,
        LibFred::UpdateContactByHandle& update_op)const
{
    if (change.disclose == UpdateOperation::Action::set_value)
    {
        get_privacy(
                update_op,
                old_data.disclosename,
                (*change.disclose).name,
                default_disclose_name_,
                &LibFred::UpdateContactByHandle::set_disclosename);
        get_privacy(
                update_op,
                old_data.discloseorganization,
                (*change.disclose).organization,
                default_disclose_organization_,
                &LibFred::UpdateContactByHandle::set_discloseorganization);
        get_privacy(
                update_op,
                old_data.discloseaddress,
                (*change.disclose).address,
                default_disclose_address_,
                &LibFred::UpdateContactByHandle::set_discloseaddress);
        get_privacy(
                update_op,
                old_data.disclosetelephone,
                (*change.disclose).telephone,
                default_disclose_telephone_,
                &LibFred::UpdateContactByHandle::set_disclosetelephone);
        get_privacy(
                update_op,
                old_data.disclosefax,
                (*change.disclose).fax,
                default_disclose_fax_,
                &LibFred::UpdateContactByHandle::set_disclosefax);
        get_privacy(
                update_op,
                old_data.discloseemail,
                (*change.disclose).email,
                default_disclose_email_,
                &LibFred::UpdateContactByHandle::set_discloseemail);
        get_privacy(
                update_op,
                old_data.disclosevat,
                (*change.disclose).vat,
                default_disclose_vat_,
                &LibFred::UpdateContactByHandle::set_disclosevat);
        get_privacy(
                update_op,
                old_data.discloseident,
                (*change.disclose).ident,
                default_disclose_ident_,
                &LibFred::UpdateContactByHandle::set_discloseident);
        get_privacy(
                update_op,
                old_data.disclosenotifyemail,
                (*change.disclose).notify_email,
                default_disclose_notify_email_,
                &LibFred::UpdateContactByHandle::set_disclosenotifyemail);
    }
    return update_op;
}

}//namespace Epp::Contact::Impl::SetUnused
}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp
