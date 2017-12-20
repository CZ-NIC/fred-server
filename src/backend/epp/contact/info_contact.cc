/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/backend/epp/impl/disclose_policy.hh"
#include "src/backend/epp/contact/info_contact.hh"

#include "src/backend/admin/contact/verification/contact_states/enum.hh"
#include "src/backend/epp/contact/status_value.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/impl/util.hh"
#include "src/libfred/registrable_object/contact.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/object_state/get_object_states.hh"
#include "src/libfred/registrar.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/util/db/nullable.hh"

#include <boost/foreach.hpp>

#include <string>

namespace Epp {
namespace Contact {

namespace {

void insert_discloseflags(const LibFred::InfoContactData& src, ContactDisclose& dst)
{
    const bool meaning_of_present_discloseflag = dst.does_present_item_mean_to_disclose();
    if (src.disclosename == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::name >();
    }
    if (src.discloseorganization == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::organization >();
    }
    if (src.discloseaddress == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::address >();
    }
    if (src.disclosetelephone == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::telephone >();
    }
    if (src.disclosefax == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::fax >();
    }
    if (src.discloseemail == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::email >();
    }
    if (src.disclosevat == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::vat >();
    }
    if (src.discloseident == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::ident >();
    }
    if (src.disclosenotifyemail == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::notify_email >();
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

} // namespace Epp::{anonymous}

InfoContactOutputData::InfoContactOutputData(const boost::optional<ContactDisclose>& _disclose)
:   disclose(_disclose)
{ }

InfoContactOutputData info_contact(
        LibFred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const InfoContactConfigData& _info_contact_config_data,
        const SessionData& _session_data)
{
    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    try
    {
        const LibFred::InfoContactData info_contact_data = LibFred::InfoContactByHandle(_contact_handle).exec(_ctx, "UTC").info_contact_data;

        InfoContactOutputData info_contact_output_data(get_discloseflags(info_contact_data));
        info_contact_output_data.handle = info_contact_data.handle;
        info_contact_output_data.roid = info_contact_data.roid;
        info_contact_output_data.sponsoring_registrar_handle = info_contact_data.sponsoring_registrar_handle;
        info_contact_output_data.creating_registrar_handle = info_contact_data.create_registrar_handle;
        info_contact_output_data.last_update_registrar_handle = info_contact_data.update_registrar_handle;
        {
            typedef std::vector<LibFred::ObjectStateData> ObjectStatesData;

            ObjectStatesData domain_states_data = LibFred::GetObjectStates(info_contact_data.id).exec(_ctx);
            for (ObjectStatesData::const_iterator object_state = domain_states_data.begin();
                 object_state != domain_states_data.end(); ++object_state)
            {

                // XXX HACK: Tickets #10053, #20574 - temporary hack until changed xml schemas are released upon poor registrars
                if (object_state->state_name == "contactFailedManualVerification" ||
                    object_state->state_name == "contactInManualVerification" ||
                    object_state->state_name == "contactPassedManualVerification" ||
                    object_state->state_name == "serverBlocked")
                {
                    continue; // do not propagate any of these states
                }

                if (object_state->is_external)
                {
                    info_contact_output_data.states.insert(Conversion::Enums::from_fred_object_state<StatusValue>(
                            Conversion::Enums::from_db_handle<LibFred::Object_State>(
                                    object_state->state_name)));
                }
            }
        }

        info_contact_output_data.crdate = info_contact_data.creation_time;
        info_contact_output_data.last_update = info_contact_data.update_time;
        info_contact_output_data.last_transfer = info_contact_data.transfer_time;
        info_contact_output_data.name = info_contact_data.name;
        info_contact_output_data.organization = info_contact_data.organization;
        info_contact_output_data.street1 =
                info_contact_data.place.isnull()
                        ? Nullable<std::string>()
                        : info_contact_data.place.get_value().street1;
        info_contact_output_data.street2 =
                info_contact_data.place.isnull() || !info_contact_data.place.get_value().street2.isset()
                        ? Nullable<std::string>()
                        : info_contact_data.place.get_value().street2.get_value();
        info_contact_output_data.street3 =
                info_contact_data.place.isnull() || !info_contact_data.place.get_value().street3.isset()
                        ? Nullable<std::string>()
                        : info_contact_data.place.get_value().street3.get_value();
        info_contact_output_data.city =
                info_contact_data.place.isnull()
                        ? Nullable<std::string>()
                        : info_contact_data.place.get_value().city;
        info_contact_output_data.state_or_province =
                info_contact_data.place.isnull() || !info_contact_data.place.get_value().stateorprovince.isset()
                        ? Nullable<std::string>()
                        : info_contact_data.place.get_value().stateorprovince.get_value();
        info_contact_output_data.postal_code =
                info_contact_data.place.isnull()
                        ? Nullable<std::string>()
                        : info_contact_data.place.get_value().postalcode;
        info_contact_output_data.country_code =
                info_contact_data.place.isnull()
                        ? Nullable<std::string>()
                        : info_contact_data.place.get_value().country;
        const LibFred::ContactAddressList::const_iterator addresses_itr =
                info_contact_data.addresses.find(LibFred::ContactAddressType::MAILING);
        if (addresses_itr != info_contact_data.addresses.end())
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
            info_contact_output_data.mailing_address = mailing_address;
        }
        info_contact_output_data.telephone = info_contact_data.telephone;
        info_contact_output_data.fax = info_contact_data.fax;
        info_contact_output_data.email = info_contact_data.email;
        info_contact_output_data.notify_email = info_contact_data.notifyemail;
        info_contact_output_data.VAT = info_contact_data.vat;
        info_contact_output_data.personal_id = get_personal_id(info_contact_data.ssn, info_contact_data.ssntype);

        // show object authinfopw only to sponsoring registrar
        const std::string session_registrar_handle =
            LibFred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data.handle;

        const bool authinfopw_has_to_be_hidden = info_contact_data.sponsoring_registrar_handle !=
                                                 session_registrar_handle;
        info_contact_output_data.authinfopw = authinfopw_has_to_be_hidden
                                                      ? boost::optional<std::string>()
                                                      : info_contact_data.authinfopw;

        return info_contact_output_data;

    }
    catch (const LibFred::InfoContactByHandle::Exception& e) {

        if (e.is_set_unknown_contact_handle()) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}

} // namespace Epp::Contact
} // namespace Epp
