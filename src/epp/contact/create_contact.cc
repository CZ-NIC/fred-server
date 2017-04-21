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

#include "src/epp/contact/contact_change.h"
#include "src/epp/contact/create_contact.h"
#include "src/epp/impl/disclose_policy.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/optional_value.h"

#include <boost/optional.hpp>

#include <string>

namespace Epp {
namespace Contact {

namespace {

template <ContactDisclose::Item::Enum ITEM>
bool should_item_be_disclosed(const boost::optional<ContactDisclose>& _disclose)
{
    const bool use_the_default_policy = !_disclose.is_initialized();
    if (use_the_default_policy) {
        return is_the_default_policy_to_disclose();
    }
    return _disclose->should_be_disclosed< ITEM >(is_the_default_policy_to_disclose());
}

Optional<std::string> to_db_handle(const Nullable<ContactChange::IdentType::Enum>& src)
{
    if (src.isnull()) {
        return Optional< std::string >();
    }
    switch (src.get_value())
    {
        case ContactChange::IdentType::op:       return Fred::PersonalIdUnion::get_OP("").get_type();
        case ContactChange::IdentType::pass:     return Fred::PersonalIdUnion::get_PASS("").get_type();
        case ContactChange::IdentType::ico:      return Fred::PersonalIdUnion::get_ICO("").get_type();
        case ContactChange::IdentType::mpsv:     return Fred::PersonalIdUnion::get_MPSV("").get_type();
        case ContactChange::IdentType::birthday: return Fred::PersonalIdUnion::get_BIRTHDAY("").get_type();
    }
    throw std::runtime_error("Invalid Epp::Contact::ContactChange::IdentType::Enum value.");
}

} // namespace Epp::{anonymous}

CreateContactResult create_contact(
        Fred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const CreateContactInputData& _contact_data,
        const CreateContactConfigData& _create_contact_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data)) {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    const bool handle_is_valid = Fred::Contact::get_handle_syntax_validity(_contact_handle) ==
                                 Fred::ContactHandleState::SyntaxValidity::valid;
    if (!handle_is_valid) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_syntax_error)
                                        .add_extended_error(EppExtendedError::of_scalar_parameter(
                                                Param::contact_handle,
                                                Reason::bad_format_contact_handle)));
    }

    {
        const Fred::ContactHandleState::Registrability::Enum contact_registrability =
            Fred::Contact::get_handle_registrability(_ctx, _contact_handle);

        const bool contact_is_registered = contact_registrability ==
                                           Fred::ContactHandleState::Registrability::registered;
        if (contact_is_registered) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_exists));
        }

        EppResultFailure parameter_value_policy_errors(EppResultCode::parameter_value_policy_error);

        const bool contact_is_in_protection_period = contact_registrability ==
                                                     Fred::ContactHandleState::Registrability::in_protection_period;
        if (contact_is_in_protection_period) {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_scalar_parameter(
                            Param::contact_handle,
                            Reason::protected_period));
        }

        if (!is_country_code_valid(_ctx, _contact_data.country_code)) {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_scalar_parameter(
                            Param::contact_cc,
                            Reason::country_notexist));
        }

        if (!parameter_value_policy_errors.empty()) {
            throw EppResponseFailure(parameter_value_policy_errors);
        }
    }

    try {
        Fred::Contact::PlaceAddress place;
        switch (_contact_data.streets.size())
        {
            case 3: place.street3 = _contact_data.streets[2];
            case 2: place.street2 = _contact_data.streets[1];
            case 1: place.street1 = _contact_data.streets[0];
            case 0: break;
            default: throw std::runtime_error("Too many streets.");
        }
        place.city            = _contact_data.city;
        place.stateorprovince = _contact_data.state_or_province;
        place.postalcode      = _contact_data.postal_code;
        place.country         = _contact_data.country_code;

        if (_contact_data.disclose.is_initialized()) {
            _contact_data.disclose->check_validity();
        }

        const Fred::CreateContact create_contact_op(
            _contact_handle,
            Fred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data.handle,
            _contact_data.authinfopw ? Optional<std::string>(*_contact_data.authinfopw) : Optional<std::string>() ,
            _contact_data.name,
            _contact_data.organization,
            place,
            _contact_data.telephone,
            _contact_data.fax,
            _contact_data.email,
            _contact_data.notify_email,
            _contact_data.vat,
            to_db_handle(_contact_data.identtype),
            _contact_data.ident,
            // will be implemented in #13744
            Optional< Fred::ContactAddressList >(),
            should_item_be_disclosed< ContactDisclose::Item::name         >(_contact_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::organization >(_contact_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::address      >(_contact_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::telephone    >(_contact_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::fax          >(_contact_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::email        >(_contact_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::vat          >(_contact_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::ident        >(_contact_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::notify_email >(_contact_data.disclose),
            Optional< Nullable< bool > >(),
            _session_data.logd_request_id);

        const Fred::CreateContact::Result create_data = create_contact_op.exec(_ctx, "UTC");

        return CreateContactResult(
            create_data.create_object_result.object_id,
            create_data.create_object_result.history_id,
            create_data.creation_time
        );

    }
    catch (const Fred::CreateContact::Exception& e) {

        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (
            e.is_set_forbidden_company_name_setting() ||
            e.is_set_unknown_registrar_handle() ||
            e.is_set_unknown_ssntype()
        ) {
            throw;
        }

        if ( e.is_set_invalid_contact_handle() /* wrong exception name */ ) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_exists));
        }

        if ( e.is_set_unknown_country() ) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error)
                    .add_extended_error(EppExtendedError::of_scalar_parameter(
                            Param::contact_cc,
                            Reason::country_notexist)));
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}

} // namespace Epp::Contact
} // namespace Epp
