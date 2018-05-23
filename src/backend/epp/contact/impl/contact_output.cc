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

#include "src/backend/epp/contact/impl/contact_output.hh"

#include "src/backend/epp/contact/contact_disclose.hh"
#include "src/backend/epp/contact/status_value.hh"
#include "src/backend/epp/impl/disclose_policy.hh"
#include "src/libfred/object/object_id_handle_pair.hh"
#include "src/libfred/object/object_state.hh"
#include "src/util/db/nullable.hh"
#include "src/util/enum_conversion.hh"

#include <string>

namespace Epp {
namespace Contact {

namespace {

void insert_discloseflags(const LibFred::InfoContactData& src, ContactDisclose& dst)
{
    const bool meaning_of_present_discloseflag = dst.does_present_item_mean_to_disclose();
    if (src.disclosename == meaning_of_present_discloseflag) {
        dst.add<ContactDisclose::Item::name>();
    }
    if (src.discloseorganization == meaning_of_present_discloseflag) {
        dst.add<ContactDisclose::Item::organization>();
    }
    if (src.discloseaddress == meaning_of_present_discloseflag) {
        dst.add<ContactDisclose::Item::address>();
    }
    if (src.disclosetelephone == meaning_of_present_discloseflag) {
        dst.add<ContactDisclose::Item::telephone>();
    }
    if (src.disclosefax == meaning_of_present_discloseflag) {
        dst.add<ContactDisclose::Item::fax>();
    }
    if (src.discloseemail == meaning_of_present_discloseflag) {
        dst.add<ContactDisclose::Item::email>();
    }
    if (src.disclosevat == meaning_of_present_discloseflag) {
        dst.add<ContactDisclose::Item::vat>();
    }
    if (src.discloseident == meaning_of_present_discloseflag) {
        dst.add<ContactDisclose::Item::ident>();
    }
    if (src.disclosenotifyemail == meaning_of_present_discloseflag) {
        dst.add<ContactDisclose::Item::notify_email>();
    }
}

boost::optional<ContactDisclose> get_discloseflags(const LibFred::InfoContactData& src)
{
    ContactDisclose disclose(is_the_default_policy_to_disclose() ? ContactDisclose::Flag::hide
                                                                 : ContactDisclose::Flag::disclose);
    insert_discloseflags(src, disclose);
    const bool discloseflags_conform_to_the_default_policy = disclose.is_empty();
    return discloseflags_conform_to_the_default_policy ? boost::optional<ContactDisclose>()
                                                       : disclose;
}

boost::optional<LibFred::PersonalIdUnion> get_personal_id(
        const Nullable<std::string>& _value,
        const Nullable<std::string>& _type)
{
    if (_value.isnull() || _type.isnull()) {
        return boost::optional<LibFred::PersonalIdUnion>();
    }
    const std::string value = _value.get_value();
    const std::string type = _type.get_value();
    LibFred::PersonalIdUnion result = LibFred::PersonalIdUnion::get_OP(value);
    if (result.get_type() == type) {
        return result;
    }
    result = LibFred::PersonalIdUnion::get_PASS(value);
    if (result.get_type() == type) {
        return result;
    }
    result = LibFred::PersonalIdUnion::get_ICO(value);
    if (result.get_type() == type) {
        return result;
    }
    result = LibFred::PersonalIdUnion::get_MPSV(value);
    if (result.get_type() == type) {
        return result;
    }
    result = LibFred::PersonalIdUnion::get_BIRTHDAY(value);
    if (result.get_type() == type) {
        return result;
    }
    result = LibFred::PersonalIdUnion::get_RC(value);
    if (result.get_type() == type) {
        return boost::optional<LibFred::PersonalIdUnion>();
    }
    throw std::runtime_error("Invalid ident type.");
}

} // namespace Epp::Contact::{anonymous}

InfoContactOutputData::InfoContactOutputData(const boost::optional<ContactDisclose>& _disclose)
    : disclose(_disclose)
{
}

InfoContactOutputData get_info_contact_output(
    const LibFred::InfoContactData& _data,
    const std::vector<LibFred::ObjectStateData>& _object_state_data,
    bool _include_authinfo)
{

    // TODO: move get_discloseflags from info_contact.cc to this file (impl/ocntact_output.cc) and call this function from info_contact.cc (remove duplicit implementation)
    InfoContactOutputData ret(get_discloseflags(_data));

    ret.handle = _data.handle;
    ret.roid = _data.roid;
    ret.sponsoring_registrar_handle = _data.sponsoring_registrar_handle;
    ret.creating_registrar_handle = _data.create_registrar_handle;
    ret.last_update_registrar_handle = _data.update_registrar_handle;

    for (const auto& object_state : _object_state_data)
    {

        // XXX HACK: Tickets #10053, #20574 - temporary hack until changed xml schemas are released upon poor registrars
        if ((Conversion::Enums::from_db_handle<LibFred::Object_State>(object_state.state_name) == LibFred::Object_State::contact_failed_manual_verification) ||
            (Conversion::Enums::from_db_handle<LibFred::Object_State>(object_state.state_name) == LibFred::Object_State::contact_in_manual_verification) ||
            (Conversion::Enums::from_db_handle<LibFred::Object_State>(object_state.state_name) == LibFred::Object_State::contact_passed_manual_verification) ||
            (Conversion::Enums::from_db_handle<LibFred::Object_State>(object_state.state_name) == LibFred::Object_State::server_blocked))
        {
            continue; // do not propagate any of these states
        }

        if (object_state.is_external)
        {
            ret.states.insert(Conversion::Enums::from_fred_object_state<StatusValue>(
                    Conversion::Enums::from_db_handle<LibFred::Object_State>(
                            object_state.state_name)));
        }
    }

    ret.crdate = _data.creation_time;
    ret.last_update = _data.update_time;
    ret.last_transfer = _data.transfer_time;
    ret.name = _data.name;
    ret.organization = _data.organization;
    ret.street1 = _data.place.isnull()
                    ? Nullable<std::string>()
                    : _data.place.get_value().street1;
    ret.street2 =
            _data.place.isnull() || !_data.place.get_value().street2.isset()
                    ? Nullable<std::string>()
                    : _data.place.get_value().street2.get_value();
    ret.street3 =
            _data.place.isnull() || !_data.place.get_value().street3.isset()
                    ? Nullable<std::string>()
                    : _data.place.get_value().street3.get_value();
    ret.city =
            _data.place.isnull()
                    ? Nullable<std::string>()
                    : _data.place.get_value().city;
    ret.state_or_province =
            _data.place.isnull() || !_data.place.get_value().stateorprovince.isset()
                    ? Nullable<std::string>()
                    : _data.place.get_value().stateorprovince.get_value();
    ret.postal_code =
            _data.place.isnull()
                    ? Nullable<std::string>()
                    : _data.place.get_value().postalcode;
    ret.country_code =
            _data.place.isnull()
                    ? Nullable<std::string>()
                    : _data.place.get_value().country;
    const LibFred::ContactAddressList::const_iterator addresses_itr =
            _data.addresses.find(LibFred::ContactAddressType::MAILING);
    if (addresses_itr != _data.addresses.end())
    {
        ContactData::Address mailing_address;
        mailing_address.street1 = addresses_itr->second.street1;
        if (addresses_itr->second.street2.isset())
        {
            mailing_address.street2 = addresses_itr->second.street2.get_value();
        }
        if (addresses_itr->second.street3.isset())
        {
            mailing_address.street3 = addresses_itr->second.street3.get_value();
        }
        mailing_address.city = addresses_itr->second.city;
        if (addresses_itr->second.stateorprovince.isset())
        {
            mailing_address.state_or_province = addresses_itr->second.stateorprovince.get_value();
        }
        mailing_address.postal_code = addresses_itr->second.postalcode;
        mailing_address.country_code = addresses_itr->second.country;
        ret.mailing_address = mailing_address;
    }
    ret.telephone = _data.telephone;
    ret.fax = _data.fax;
    ret.email = _data.email;
    ret.notify_email = _data.notifyemail;
    ret.VAT = _data.vat;
    ret.personal_id = get_personal_id(_data.ssn, _data.ssntype);

    ret.authinfopw = _include_authinfo ? _data.authinfopw : boost::optional<std::string>();

    return ret;
}

} // namespace Epp::Contact
} // namespace Epp
