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

#include "src/backend/epp/contact/impl/contact_output.hh"

#include "src/backend/epp/contact/status_value.hh"
#include "libfred/object/object_id_handle_pair.hh"
#include "libfred/object/object_state.hh"
#include "util/enum_conversion.hh"

#include <string>

namespace Epp {
namespace Contact {

namespace {

template <typename T>
Hideable<T> to_hideable(const T& value, bool to_disclose)
{
    return to_disclose ? make_public_data(value)
                       : make_private_data(value);
}

template <typename T>
boost::optional<T> nullable_to_optional(const Nullable<T>& nullable)
{
    if (nullable.isnull())
    {
        return boost::none;
    }
    return nullable.get_value();
}

template <typename T>
HideableOptional<T> nullable_to_hideable_optional(const Nullable<T>& nullable, bool to_disclose)
{
    return to_hideable(nullable_to_optional(nullable), to_disclose);
}

template <typename T>
boost::optional<T> optional_to_optional(const Optional<T>& optional)
{
    if (optional.isset())
    {
        return  optional.get_value();
    }
    return boost::none;
}

boost::optional<std::string> string_to_optional(const std::string& value)
{
    if (value.empty())
    {
        return boost::none;
    }
    return value;
}

boost::optional<ContactData::Address> nullable_to_optional(const Nullable<LibFred::Contact::PlaceAddress>& nullable)
{
    if (nullable.isnull())
    {
        return boost::none;
    }
    ContactData::Address retval;
    retval.street[0] = string_to_optional(nullable.get_value().street1);
    retval.street[1] = optional_to_optional(nullable.get_value().street2);
    retval.street[2] = optional_to_optional(nullable.get_value().street3);
    retval.city = string_to_optional(nullable.get_value().city);
    retval.state_or_province = optional_to_optional(nullable.get_value().stateorprovince);
    retval.postal_code = string_to_optional(nullable.get_value().postalcode);
    retval.country_code = string_to_optional(nullable.get_value().country);
    return retval;
}

HideableOptional<ContactData::Address> nullable_to_hideable_optional(
        const Nullable<LibFred::Contact::PlaceAddress>& nullable,
        bool to_disclose)
{
    return to_hideable(nullable_to_optional(nullable), to_disclose);
}

boost::optional<ContactIdent> get_contact_ident(
        const Nullable<std::string>& _value,
        const Nullable<std::string>& _type)
{
    if (_value.isnull() || _type.isnull())
    {
        return boost::none;
    }
    const std::string value = _value.get_value();
    const std::string type = _type.get_value();
    if (LibFred::PersonalIdUnion::get_OP(value).get_type() == type)
    {
        return make_contact_ident<ContactIdentType::Op>(value);
    }
    if (LibFred::PersonalIdUnion::get_PASS(value).get_type() == type)
    {
        return make_contact_ident<ContactIdentType::Pass>(value);
    }
    if (LibFred::PersonalIdUnion::get_ICO(value).get_type() == type)
    {
        return make_contact_ident<ContactIdentType::Ico>(value);
    }
    if (LibFred::PersonalIdUnion::get_MPSV(value).get_type() == type)
    {
        return make_contact_ident<ContactIdentType::Mpsv>(value);
    }
    if (LibFred::PersonalIdUnion::get_BIRTHDAY(value).get_type() == type)
    {
        return make_contact_ident<ContactIdentType::Birthday>(value);
    }
    if (LibFred::PersonalIdUnion::get_RC(value).get_type() == type)
    {
        return boost::none;
    }
    throw std::runtime_error("Invalid ident type.");
}

InfoContactOutputData get_info_contact_output(
        const LibFred::InfoContactData& _data,
        const std::vector<LibFred::ObjectStateData>& _object_state_data)
{
    InfoContactOutputData retval;

    retval.handle = _data.handle;
    retval.roid = _data.roid;
    retval.sponsoring_registrar_handle = _data.sponsoring_registrar_handle;
    retval.creating_registrar_handle = _data.create_registrar_handle;
    retval.last_update_registrar_handle = nullable_to_optional(_data.update_registrar_handle);

    for (const auto& object_state : _object_state_data)
    {
        if (object_state.is_external)
        {
            const auto object_state_enum = Conversion::Enums::from_db_handle<LibFred::Object_State>(object_state.state_name);
            switch (object_state_enum)
            {
            // XXX HACK: Tickets #10053, #20574 - temporary hack until changed xml schemas are released upon poor registrars
                case LibFred::Object_State::contact_failed_manual_verification:
                case LibFred::Object_State::contact_in_manual_verification:
                case LibFred::Object_State::contact_passed_manual_verification:
                case LibFred::Object_State::server_blocked:
                    break;
                default:
                    retval.states.insert(Conversion::Enums::from_fred_object_state<StatusValue>(object_state_enum));
                    break;
            }
        }
    }

    retval.crdate = _data.creation_time;
    retval.last_update = nullable_to_optional(_data.update_time);
    retval.last_transfer = nullable_to_optional(_data.transfer_time);
    retval.name = nullable_to_hideable_optional(_data.name, _data.disclosename);
    retval.organization = nullable_to_hideable_optional(_data.organization, _data.discloseorganization);
    retval.address = nullable_to_hideable_optional(_data.place, _data.discloseaddress);
    const LibFred::ContactAddressList::const_iterator addresses_itr =
            _data.addresses.find(LibFred::ContactAddressType::MAILING);
    const bool has_mailing_address = addresses_itr != _data.addresses.end();
    if (has_mailing_address)
    {
        ContactData::Address mailing_address;
        mailing_address.street[0] = string_to_optional(addresses_itr->second.street1);
        mailing_address.street[1] = optional_to_optional(addresses_itr->second.street2);
        mailing_address.street[2] = optional_to_optional(addresses_itr->second.street3);
        mailing_address.city = string_to_optional(addresses_itr->second.city);
        mailing_address.state_or_province = optional_to_optional(addresses_itr->second.stateorprovince);
        mailing_address.postal_code = string_to_optional(addresses_itr->second.postalcode);
        mailing_address.country_code = string_to_optional(addresses_itr->second.country);
        retval.mailing_address = mailing_address;
    }
    else
    {
        retval.mailing_address = boost::none;
    }
    retval.telephone = nullable_to_hideable_optional(_data.telephone, _data.disclosetelephone);
    retval.fax = nullable_to_hideable_optional(_data.fax, _data.disclosefax);
    retval.email = nullable_to_hideable_optional(_data.email, _data.discloseemail);
    retval.notify_email = nullable_to_hideable_optional(_data.notifyemail, _data.disclosenotifyemail);
    retval.VAT = nullable_to_hideable_optional(_data.vat, _data.disclosevat);
    retval.personal_id = to_hideable(get_contact_ident(_data.ssn, _data.ssntype), _data.discloseident);

    return retval;
}

}//namespace Epp::Contact::{anonymous}

InfoContactOutputData get_info_contact_output(
        LibFred::OperationContext& _ctx,
        LibFred::InfoContactData _data,
        const Password& _authinfopw,
        const ContactDataSharePolicyRules& _contact_data_share_policy_rules,
        const SessionData& _session_data,
        const std::vector<LibFred::ObjectStateData>& _object_state_data)
{
    _contact_data_share_policy_rules.apply(
                _ctx,
                _authinfopw,
                _session_data,
                _data);
    return get_info_contact_output(_data, _object_state_data);
}

InfoContactOutputData get_info_contact_output(
        const LibFred::InfoContactData& _data,
        const std::vector<LibFred::ObjectStateData>& _object_state_data,
        bool _include_authinfo [[gnu::unused]])
{
    return get_info_contact_output(_data, _object_state_data);
}

}//namespace Epp::Contact
}//namespace Epp
