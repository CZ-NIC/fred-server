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

#include "src/backend/epp/contact/impl/cznic/update_contact_data_filter.hh"
#include "src/backend/epp/contact/util.hh"

#include "libfred/object/object_states_info.hh"

#include <stdexcept>
#include <type_traits>

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

struct ExceptionDiscloseflagRulesViolation:
        Epp::Contact::UpdateContactDataFilter::DiscloseflagRulesViolation,
        std::runtime_error
{
    ExceptionDiscloseflagRulesViolation() : std::runtime_error("discloseflag rules violation") { }
};

bool is_change_operation(const Deletable<std::string>& op)
{
    return op != UpdateOperation::Action::no_operation;
}

bool is_change_operation(const Updateable<std::string>& op)
{
    return op != UpdateOperation::Action::no_operation;
}

bool is_change_operation(const StreetTraits::Rows<Deletable<std::string>>& street)
{
    for (const auto& op : street)
    {
        if (is_change_operation(op))
        {
            return true;
        }
    }
    return false;
}

bool is_fake_change_operation(const Deletable<std::string>& op, const std::string& value)
{
    if (op == UpdateOperation::Action::no_operation)
    {
        return true;
    }
    if (op == UpdateOperation::Action::delete_value)
    {
        return trim(value).empty();
    }
    if (op == UpdateOperation::Action::set_value)
    {
        const auto trimmed_new_value = trim(*op);
        const auto trimmed_old_value = trim(value);
        return trimmed_new_value == trimmed_old_value;
    }
    throw std::runtime_error("unknown update operation");
}

bool is_fake_change_operation(const Deletable<std::string>& op, const Nullable<std::string>& value)
{
    if (op == UpdateOperation::Action::no_operation)
    {
        return true;
    }
    if (op == UpdateOperation::Action::delete_value)
    {
        return value.isnull() || trim(value.get_value()).empty();
    }
    if (op == UpdateOperation::Action::set_value)
    {
        const auto trimmed_new_value = trim(*op);
        const auto trimmed_old_value = value.isnull() ? std::string() : trim(value.get_value());
        return trimmed_new_value == trimmed_old_value;
    }
    throw std::runtime_error("unknown update operation");
}

bool is_fake_change_operation(const Deletable<std::string>& op, const Optional<std::string>& value)
{
    if (op == UpdateOperation::Action::no_operation)
    {
        return true;
    }
    if (op == UpdateOperation::Action::delete_value)
    {
        return !value.isset() || trim(value.get_value()).empty();
    }
    if (op == UpdateOperation::Action::set_value)
    {
        const auto trimmed_new_value = trim(*op);
        const auto trimmed_old_value = value.isset() ? trim(value.get_value()) : std::string();
        return trimmed_new_value == trimmed_old_value;
    }
    throw std::runtime_error("unknown update operation");
}

bool is_fake_change_operation(const Updateable<std::string>& op, const Nullable<std::string>& value)
{
    if (op == UpdateOperation::Action::no_operation)
    {
        return true;
    }
    if (op == UpdateOperation::Action::set_value)
    {
        const auto trimmed_new_value = trim(*op);
        const auto trimmed_old_value = value.isnull() ? std::string() : trim(value.get_value());
        return trimmed_new_value == trimmed_old_value;
    }
    throw std::runtime_error("unknown update operation");
}

bool is_fake_change_operation(const Updateable<std::string>& op, const std::string& value)
{
    if (op == UpdateOperation::Action::no_operation)
    {
        return true;
    }
    if (op == UpdateOperation::Action::set_value)
    {
        const auto trimmed_new_value = trim(*op);
        const auto trimmed_old_value = trim(value);
        return trimmed_new_value == trimmed_old_value;
    }
    throw std::runtime_error("unknown update operation");
}

bool is_fake_change_operation(
        const StreetTraits::Rows<Deletable<std::string>>& street,
        const LibFred::Contact::PlaceAddress& old_address)
{
    static_assert(std::tuple_size<std::remove_reference_t<decltype(street)>>::value == 3, "must be exactly 3 streets");
    return is_fake_change_operation(street[0], old_address.street1) &&
           is_fake_change_operation(street[1], old_address.street2) &&
           is_fake_change_operation(street[2], old_address.street3);
}

bool is_fake_change_operation(
        const ContactChange::MainAddress& change,
        const Nullable<LibFred::Contact::PlaceAddress>& old_address)
{
    if (old_address.isnull())
    {
        const Nullable<std::string> old_value;
        for (const auto& op : change.street)
        {
            if (!is_fake_change_operation(op, old_value))
            {
                return false;
            }
        }
        return is_fake_change_operation(change.city, old_value) &&
               is_fake_change_operation(change.state_or_province, old_value) &&
               is_fake_change_operation(change.postal_code, old_value) &&
               is_fake_change_operation(change.country_code, old_value);
    }
    return is_fake_change_operation(change.street, old_address.get_value()) &&
           is_fake_change_operation(change.city, old_address.get_value().city) &&
           is_fake_change_operation(change.state_or_province, old_address.get_value().stateorprovince) &&
           is_fake_change_operation(change.postal_code, old_address.get_value().postalcode) &&
           is_fake_change_operation(change.country_code, old_address.get_value().country);
}

bool is_sufficient_authenticity_level_to_private_address(
        LibFred::OperationContext& ctx,
        const LibFred::InfoContactData& old_data,
        const ContactChange& change)
{
    const LibFred::ObjectStatesInfo contact_states(LibFred::GetObjectStates(old_data.id).exec(ctx));
    const bool contact_data_authenticity_is_sufficient =
            contact_states.presents(LibFred::Object_State::identified_contact) ||
            contact_states.presents(LibFred::Object_State::validated_contact);
    if (!contact_data_authenticity_is_sufficient)
    {
        return false;
    }
    const bool change_planned =
            is_change_operation(change.name) ||
            is_change_operation(change.organization) ||
            is_change_operation(change.address.street) ||
            is_change_operation(change.address.city) ||
            is_change_operation(change.address.state_or_province) ||
            is_change_operation(change.address.postal_code) ||
            is_change_operation(change.address.country_code) ||
            is_change_operation(change.telephone) ||
            is_change_operation(change.email);
    if (change_planned)
    {
        const bool change_is_fake_only =
                is_fake_change_operation(change.name, old_data.name) &&
                is_fake_change_operation(change.organization, old_data.organization) &&
                is_fake_change_operation(change.address, old_data.place) &&
                is_fake_change_operation(change.telephone, old_data.telephone) &&
                is_fake_change_operation(change.email, old_data.email);
        if (!change_is_fake_only)
        {
            return false;
        }
    }
    const bool contact_is_organization = !old_data.organization.get_value_or("").empty();
    const bool address_can_be_hidden = !contact_is_organization;
    return address_can_be_hidden;
}

}//namespace Epp::Contact::Impl::CzNic::{anonymous}

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
        LibFred::OperationContext& ctx,
        const LibFred::InfoContactData& old_data,
        const ContactChange& change,
        const SessionData& session_data,
        LibFred::UpdateContactByHandle& update_op)const
{
    if (change.disclose == UpdateOperation::Action::set_value)
    {
        const auto disclose_name = get_privacy(
                update_op,
                old_data.disclosename,
                (*change.disclose).name,
                default_disclose_name_,
                &LibFred::UpdateContactByHandle::set_disclosename);
        if (disclose_name == Public::no)
        {
            throw ExceptionDiscloseflagRulesViolation();
        }
        const auto disclose_organization = get_privacy(
                update_op,
                old_data.discloseorganization,
                (*change.disclose).organization,
                default_disclose_organization_,
                &LibFred::UpdateContactByHandle::set_discloseorganization);
        if (disclose_organization == Public::no)
        {
            throw ExceptionDiscloseflagRulesViolation();
        }
        const auto disclose_address = get_privacy(
                update_op,
                old_data.discloseaddress,
                (*change.disclose).address,
                default_disclose_address_,
                &LibFred::UpdateContactByHandle::set_discloseaddress);
        if (disclose_address == Public::no)
        {
            if (!is_sufficient_authenticity_level_to_private_address(ctx, old_data, change))
            {
                if (static_cast<bool>((*change.disclose).address) ||
                    (default_disclose_address_ != Operation::do_not_change))
                {
                    throw ExceptionDiscloseflagRulesViolation();
                }
                update_op.set_discloseaddress(true);
            }
        }
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
        return update_op;
    }
    const bool address_should_stay_private = !old_data.discloseaddress;
    if (address_should_stay_private)
    {
        const bool address_can_stay_private = is_sufficient_authenticity_level_to_private_address(ctx, old_data, change);
        if (!address_can_stay_private)
        {
            update_op.set_discloseaddress(true);
        }
    }
    return update_op;
}

}//namespace Epp::Contact::Impl::CzNic
}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp
