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
#include <boost/variant.hpp>

#include <string>

namespace Epp {
namespace Contact {

namespace {

template <ContactDisclose::Item::Enum item>
bool should_item_be_disclosed(const boost::optional<ContactDisclose>& disclose)
{
    const bool use_the_default_policy = !disclose;
    if (use_the_default_policy)
    {
        return is_the_default_policy_to_disclose();
    }
    return disclose->should_be_disclosed<item>(is_the_default_policy_to_disclose());
}

template <typename T>
Optional<T> to_optional(const boost::optional<T>& src)
{
    return src ? Optional<T>(*src)
               : Optional<T>();
}

Optional<std::string> to_optional(const std::string& src)
{
    return src.empty() ? Optional<std::string>()
                       : Optional<std::string>(src);
}

struct GetPersonalIdUnionFromContactIdent:boost::static_visitor<Fred::PersonalIdUnion>
{
    Fred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Op>& src)const
    {
        return Fred::PersonalIdUnion::get_OP(src.value);
    }
    Fred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Pass>& src)const
    {
        return Fred::PersonalIdUnion::get_PASS(src.value);
    }
    Fred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Ico>& src)const
    {
        return Fred::PersonalIdUnion::get_ICO(src.value);
    }
    Fred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Mpsv>& src)const
    {
        return Fred::PersonalIdUnion::get_MPSV(src.value);
    }
    Fred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Birthday>& src)const
    {
        return Fred::PersonalIdUnion::get_BIRTHDAY(src.value);
    }
};

boost::optional<Fred::PersonalIdUnion> get_ident(const boost::optional<ContactIdent>& ident)
{
    return ident ? boost::apply_visitor(GetPersonalIdUnionFromContactIdent(), *ident)
                 : boost::optional<Fred::PersonalIdUnion>();
}

}//namespace Epp::Contact::{anonymous}

CreateContactResult::CreateContactResult(
        unsigned long long _contact_id,
        unsigned long long _create_history_id,
        const boost::posix_time::ptime& _contact_crdate)
    : id(_contact_id),
      create_history_id(_create_history_id),
      crdate(_contact_crdate)
{ }

CreateContactResult create_contact(
        Fred::OperationContext& ctx,
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

    const bool handle_is_valid = Fred::Contact::get_handle_syntax_validity(ctx, contact_handle) ==
                                 Fred::ContactHandleState::SyntaxValidity::valid;
    if (!handle_is_valid)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_syntax_error)
                                        .add_extended_error(EppExtendedError::of_scalar_parameter(
                                                Param::contact_handle,
                                                Reason::bad_format_contact_handle)));
    }

    {
        const Fred::ContactHandleState::Registrability::Enum contact_registrability =
            Fred::Contact::get_handle_registrability(ctx, contact_handle);

        const bool contact_is_registered = contact_registrability ==
                                           Fred::ContactHandleState::Registrability::registered;
        if (contact_is_registered)
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_exists));
        }

        EppResultFailure parameter_value_policy_errors(EppResultCode::parameter_value_policy_error);

        const bool contact_is_in_protection_period = contact_registrability ==
                                                     Fred::ContactHandleState::Registrability::in_protection_period;
        if (contact_is_in_protection_period)
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_scalar_parameter(
                            Param::contact_handle,
                            Reason::protected_period));
        }

        if (!is_country_code_valid(ctx, contact_data.country_code))
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_scalar_parameter(
                            Param::contact_cc,
                            Reason::country_notexist));
        }

        if (!parameter_value_policy_errors.empty())
        {
            throw EppResponseFailure(parameter_value_policy_errors);
        }
    }

    try
    {
        Fred::Contact::PlaceAddress place;
        switch (contact_data.streets.size())
        {
            case 3: place.street3 = contact_data.streets[2];
            case 2: place.street2 = contact_data.streets[1];
            case 1: place.street1 = contact_data.streets[0];
            case 0: break;
            default: throw std::runtime_error("Too many streets.");
        }
        place.city = contact_data.city;
        place.stateorprovince = to_optional(contact_data.state_or_province);
        place.postalcode = contact_data.postal_code;
        place.country = contact_data.country_code;

        if (contact_data.disclose)
        {
            contact_data.disclose->check_validity();
        }

        const boost::optional<Fred::PersonalIdUnion> ident = get_ident(contact_data.ident);
        Optional<Fred::ContactAddressList> addresses;
        if (static_cast<bool>(contact_data.mailing_address))
        {
            Fred::ContactAddress mailing_address;
            mailing_address.street1 = contact_data.mailing_address->street1;
            mailing_address.street2 = to_optional(contact_data.mailing_address->street2);
            mailing_address.street3 = to_optional(contact_data.mailing_address->street3);
            mailing_address.city = contact_data.mailing_address->city;
            mailing_address.stateorprovince = to_optional(contact_data.mailing_address->state_or_province);
            mailing_address.postalcode = contact_data.mailing_address->postal_code;
            mailing_address.country = contact_data.mailing_address->country_code;
            Fred::ContactAddressList address_list;
            address_list.insert(std::make_pair(Fred::ContactAddressType::MAILING, mailing_address));
            addresses = address_list;
        }
        const Fred::CreateContact create_contact_op(
            contact_handle,
            Fred::InfoRegistrarById(session_data.registrar_id).exec(ctx).info_registrar_data.handle,
            to_optional(contact_data.authinfopw),
            to_optional(contact_data.name),
            to_optional(contact_data.organization),
            place,
            to_optional(contact_data.telephone),
            to_optional(contact_data.fax),
            to_optional(contact_data.email),
            to_optional(contact_data.notify_email),
            to_optional(contact_data.vat),
            ident ? ident->get_type() : Optional<std::string>(),
            ident ? ident->get() : Optional<std::string>(),
            addresses,
            should_item_be_disclosed<ContactDisclose::Item::name>(contact_data.disclose),
            should_item_be_disclosed<ContactDisclose::Item::organization>(contact_data.disclose),
            should_item_be_disclosed<ContactDisclose::Item::address>(contact_data.disclose),
            should_item_be_disclosed<ContactDisclose::Item::telephone>(contact_data.disclose),
            should_item_be_disclosed<ContactDisclose::Item::fax>(contact_data.disclose),
            should_item_be_disclosed<ContactDisclose::Item::email>(contact_data.disclose),
            should_item_be_disclosed<ContactDisclose::Item::vat>(contact_data.disclose),
            should_item_be_disclosed<ContactDisclose::Item::ident>(contact_data.disclose),
            should_item_be_disclosed<ContactDisclose::Item::notify_email>(contact_data.disclose),
            Optional< Nullable<bool> >(),
            session_data.logd_request_id);
        const Fred::CreateContact::Result create_data = create_contact_op.exec(ctx, "UTC");

        return CreateContactResult(
            create_data.create_object_result.object_id,
            create_data.create_object_result.history_id,
            create_data.creation_time);
    }
    catch (const Fred::CreateContact::Exception& e)
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
