/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/contact/create_contact.hh"
#include "src/backend/epp/contact/contact_change.hh"
#include "src/backend/epp/contact/impl/get_personal_id_union.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/reason.hh"
#include "src/backend/epp/impl/util.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrar/info_registrar.hh"
#include "util/optional_value.hh"

#include <boost/optional.hpp>

#include <stdexcept>
#include <string>

namespace Epp {
namespace Contact {

namespace {

template <typename T>
Optional<bool> hideable_to_discloseflag(const Hideable<T>& hideable)
{
    if (hideable.is_publishability_specified())
    {
        if (hideable.is_public())
        {
            return true;
        }
        if (hideable.is_private())
        {
            return false;
        }
        throw std::logic_error("unexpected privacy policy");
    }
    return Optional<bool>();
}

template <typename T>
Optional<T> to_optional(const boost::optional<T>& src)
{
    return static_cast<bool>(src) ? Optional<T>(*src)
                                  : Optional<T>();
}

template <typename T>
Optional<T> to_optional(const T&);

template<>
Optional<std::string> to_optional<std::string>(const std::string& src)
{
    return src.empty() ? Optional<std::string>()
                       : Optional<std::string>(src);
}

boost::optional<LibFred::PersonalIdUnion> get_optional_personal_id_union(const boost::optional<ContactIdent>& ident)
{
    if (static_cast<bool>(ident))
    {
        return Impl::get_personal_id_union(*ident);
    }
    return boost::none;
}

} // namespace Epp::Contact::{anonymous}

CreateContactResult::CreateContactResult(
        unsigned long long _contact_id,
        unsigned long long _create_history_id,
        const boost::posix_time::ptime& _contact_crdate)
    : id(_contact_id),
      create_history_id(_create_history_id),
      crdate(_contact_crdate)
{ }

CreateContactResult create_contact(
        LibFred::OperationContext& ctx,
        const std::string& contact_handle,
        const CreateContactInputData& contact_data,
        const CreateContactConfigData& create_contact_config_data,
        const SessionData& session_data)
{
    if (!is_session_registrar_valid(session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    const bool handle_is_valid = LibFred::Contact::get_handle_syntax_validity(ctx, contact_handle) ==
                                 LibFred::ContactHandleState::SyntaxValidity::valid;
    if (!handle_is_valid)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_syntax_error)
                                        .add_extended_error(EppExtendedError::of_scalar_parameter(
                                                Param::contact_handle,
                                                Reason::bad_format_contact_handle)));
    }

    {
        const LibFred::ContactHandleState::Registrability::Enum contact_registrability =
            LibFred::Contact::get_handle_registrability(ctx, contact_handle);

        const bool contact_is_registered = contact_registrability ==
                                           LibFred::ContactHandleState::Registrability::registered;
        if (contact_is_registered)
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_exists));
        }

        EppResultFailure parameter_value_policy_errors(EppResultCode::parameter_value_policy_error);

        const bool contact_is_in_protection_period = contact_registrability ==
                                                     LibFred::ContactHandleState::Registrability::in_protection_period;
        if (contact_is_in_protection_period)
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_scalar_parameter(
                            Param::contact_handle,
                            Reason::protected_period));
        }

        if (!is_country_code_valid(ctx, contact_data.address->country_code))
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_scalar_parameter(
                            Param::contact_cc,
                            Reason::country_notexist));
        }

        if (static_cast<bool>(contact_data.mailing_address))
        {
            if (!is_country_code_valid(ctx, contact_data.mailing_address->country_code))
            {
                throw EppResponseFailure(parameter_value_policy_errors);
            }
        }

        if (!parameter_value_policy_errors.empty())
        {
            throw EppResponseFailure(parameter_value_policy_errors);
        }
    }

    try
    {
        LibFred::Contact::PlaceAddress place;
        place.street1 = contact_data.address->street[0];
        place.street2 = to_optional(contact_data.address->street[1]);
        place.street3 = to_optional(contact_data.address->street[2]);
        place.city = contact_data.address->city;
        place.stateorprovince = to_optional(contact_data.address->state_or_province);
        place.postalcode = contact_data.address->postal_code;
        place.country = contact_data.address->country_code;

        const auto ident = get_optional_personal_id_union(*contact_data.ident);
        Optional<LibFred::ContactAddressList> addresses;
        if (static_cast<bool>(contact_data.mailing_address))
        {
            LibFred::ContactAddress mailing_address;
            mailing_address.street1 = contact_data.mailing_address->street[0];
            mailing_address.street2 = to_optional(contact_data.mailing_address->street[1]);
            mailing_address.street3 = to_optional(contact_data.mailing_address->street[2]);
            mailing_address.city = contact_data.mailing_address->city;
            mailing_address.stateorprovince = to_optional(contact_data.mailing_address->state_or_province);
            mailing_address.postalcode = contact_data.mailing_address->postal_code;
            mailing_address.country = contact_data.mailing_address->country_code;
            LibFred::ContactAddressList address_list;
            address_list.insert(std::make_pair(LibFred::ContactAddressType::MAILING, mailing_address));
            addresses = address_list;
        }
        LibFred::CreateContact create_contact_op(
            contact_handle,
            LibFred::InfoRegistrarById(session_data.registrar_id).exec(ctx).info_registrar_data.handle,
            to_optional(contact_data.authinfopw),
            to_optional(*contact_data.name),
            to_optional(*contact_data.organization),
            place,
            to_optional(*contact_data.telephone),
            to_optional(*contact_data.fax),
            to_optional(*contact_data.email),
            to_optional(*contact_data.notify_email),
            to_optional(*contact_data.vat),
            static_cast<bool>(ident) ? ident->get_type() : Optional<std::string>(),
            static_cast<bool>(ident) ? ident->get() : Optional<std::string>(),
            addresses,
            hideable_to_discloseflag(contact_data.name),
            hideable_to_discloseflag(contact_data.organization),
            hideable_to_discloseflag(contact_data.address),
            hideable_to_discloseflag(contact_data.telephone),
            hideable_to_discloseflag(contact_data.fax),
            hideable_to_discloseflag(contact_data.email),
            hideable_to_discloseflag(contact_data.vat),
            hideable_to_discloseflag(contact_data.ident),
            hideable_to_discloseflag(contact_data.notify_email),
            Optional<Nullable<bool>>(),
            session_data.logd_request_id);
        create_contact_config_data.get_data_filter()(
                ctx,
                contact_data,
                session_data,
                create_contact_op);
        const LibFred::CreateContact::Result create_data = create_contact_op.exec(ctx, "UTC");

        return CreateContactResult(
            create_data.create_object_result.object_id,
            create_data.create_object_result.history_id,
            create_data.creation_time);
    }
    catch (const LibFred::CreateContact::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_forbidden_company_name_setting() ||
            e.is_set_unknown_registrar_handle() ||
            e.is_set_unknown_ssntype())
        {
            throw;
        }

        if (e.is_set_invalid_contact_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_exists));
        }

        if (e.is_set_unknown_country())
        {
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
