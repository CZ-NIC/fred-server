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

#include "src/epp/impl/disclose_policy.h"
#include "src/epp/contact/info_contact.h"

#include "src/admin/contact/verification/contact_states/enum.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"
#include "src/epp/object_state.h"
#include "src/fredlib/contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/db/nullable.h"

#include <boost/foreach.hpp>

#include <string>

namespace Epp {
namespace Contact {

namespace {

void insert_discloseflags(const Fred::InfoContactData& src, ContactDisclose& dst)
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

boost::optional<ContactDisclose> get_discloseflags(const Fred::InfoContactData& src)
{
    ContactDisclose disclose(is_the_default_policy_to_disclose() ? ContactDisclose::Flag::hide
                                                                 : ContactDisclose::Flag::disclose);
    insert_discloseflags(src, disclose);
    const bool discloseflags_conform_to_the_default_policy = disclose.is_empty();
    return discloseflags_conform_to_the_default_policy ? boost::optional<ContactDisclose>()
                                                       : disclose;
}

boost::optional<Fred::PersonalIdUnion> get_personal_id(
        const Nullable<std::string>& _value,
        const Nullable<std::string>& _type)
{
    if (_value.isnull() || _type.isnull()) {
        return boost::optional<Fred::PersonalIdUnion>();
    }
    const std::string value = _value.get_value();
    const std::string type = _type.get_value();
    Fred::PersonalIdUnion result = Fred::PersonalIdUnion::get_OP(value);
    if (result.get_type() == type) {
        return result;
    }
    result = Fred::PersonalIdUnion::get_PASS(value);
    if (result.get_type() == type) {
        return result;
    }
    result = Fred::PersonalIdUnion::get_ICO(value);
    if (result.get_type() == type) {
        return result;
    }
    result = Fred::PersonalIdUnion::get_MPSV(value);
    if (result.get_type() == type) {
        return result;
    }
    result = Fred::PersonalIdUnion::get_BIRTHDAY(value);
    if (result.get_type() == type) {
        return result;
    }
    result = Fred::PersonalIdUnion::get_RC(value);
    if (result.get_type() == type) {
        return boost::optional<Fred::PersonalIdUnion>();
    }
    throw std::runtime_error("Invalid ident type.");
}

}//namespace Epp::{anonymous}

InfoContactOutputData::InfoContactOutputData(const boost::optional<ContactDisclose>& _disclose)
:   disclose(_disclose)
{ }

InfoContactOutputData info_contact(
        Fred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const InfoContactConfigData& _info_contact_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data)) {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    try {
        const Fred::InfoContactData info_contact_data = Fred::InfoContactByHandle(_contact_handle).exec(_ctx, "UTC").info_contact_data;

        InfoContactOutputData info_contact_output_data(get_discloseflags(info_contact_data));
        info_contact_output_data.handle = info_contact_data.handle;
        info_contact_output_data.roid = info_contact_data.roid;
        info_contact_output_data.sponsoring_registrar_handle = info_contact_data.sponsoring_registrar_handle;
        info_contact_output_data.creating_registrar_handle = info_contact_data.create_registrar_handle;
        info_contact_output_data.last_update_registrar_handle = info_contact_data.update_registrar_handle;
        {
            typedef std::vector<Fred::ObjectStateData> ObjectStatesData;

            ObjectStatesData domain_states_data = Fred::GetObjectStates(info_contact_data.id).exec(_ctx);
            for (ObjectStatesData::const_iterator object_state = domain_states_data.begin();
                 object_state != domain_states_data.end(); ++object_state)
            {

                // XXX HACK: Ticket #10053 - temporary hack until changed xml schemas are released upon poor registrars
                if (object_state->state_name == "contactFailedManualVerification" ||
                    object_state->state_name == "contactInManualVerification" ||
                    object_state->state_name == "contactPassedManualVerification")
                {
                    continue; // do not propagate admin contact verification states
                }

                if (object_state->is_external)
                {
                    info_contact_output_data.states.insert(Conversion::Enums::from_fred_object_state<Epp::Object_State>(
                            Conversion::Enums::from_db_handle<Fred::Object_State>(
                                    object_state->state_name)));
                }
            }
        }

        info_contact_output_data.crdate = info_contact_data.creation_time;
        info_contact_output_data.last_update = info_contact_data.update_time;
        info_contact_output_data.last_transfer = info_contact_data.transfer_time;
        info_contact_output_data.name = info_contact_data.name;
        info_contact_output_data.organization = info_contact_data.organization;
        info_contact_output_data.street1 = info_contact_data.place.isnull()
                                      ? Nullable<std::string>()
                                      : info_contact_data.place.get_value().street1;
        info_contact_output_data.street2 = info_contact_data.place.isnull()
                                      ? Nullable<std::string>()
                                      : !info_contact_data.place.get_value().street2.isset()
                                                ? Nullable<std::string>()
                                                : info_contact_data.place.get_value().street2.get_value();
        info_contact_output_data.street3 = info_contact_data.place.isnull()
                                      ? Nullable<std::string>()
                                      : !info_contact_data.place.get_value().street3.isset()
                                                ? Nullable<std::string>()
                                                : info_contact_data.place.get_value().street3.get_value();
        info_contact_output_data.city = info_contact_data.place.isnull()
                                   ? Nullable<std::string>()
                                   : info_contact_data.place.get_value().city;
        info_contact_output_data.state_or_province = info_contact_data.place.isnull()
                                                ? Nullable<std::string>()
                                                : info_contact_data.place.get_value().stateorprovince.isset()
                                                          ? info_contact_data.place.get_value().stateorprovince.get_value()
                                                          : Nullable<std::string>();
        info_contact_output_data.postal_code = info_contact_data.place.isnull()
                                          ? Nullable<std::string>()
                                          : info_contact_data.place.get_value().postalcode;
        info_contact_output_data.country_code = info_contact_data.place.isnull()
                                           ? Nullable<std::string>()
                                           : info_contact_data.place.get_value().country;
        info_contact_output_data.telephone = info_contact_data.telephone;
        info_contact_output_data.fax = info_contact_data.fax;
        info_contact_output_data.email = info_contact_data.email;
        info_contact_output_data.notify_email = info_contact_data.notifyemail;
        info_contact_output_data.VAT = info_contact_data.vat;
        info_contact_output_data.personal_id = get_personal_id(info_contact_data.ssn, info_contact_data.ssntype);

        // show object authinfopw only to sponsoring registrar
        const std::string session_registrar_handle =
            Fred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data.handle;

        const bool authinfopw_has_to_be_hidden = info_contact_data.sponsoring_registrar_handle !=
                                                 session_registrar_handle;
        info_contact_output_data.authinfopw = authinfopw_has_to_be_hidden ? boost::optional<std::string>() : info_contact_data.authinfopw;

        return info_contact_output_data;

    }
    catch (const Fred::InfoContactByHandle::Exception& e) {

        if (e.is_set_unknown_contact_handle()) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}

} // namespace Epp::Contact
} // namespace Epp
