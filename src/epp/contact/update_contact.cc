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

#include "src/epp/contact/update_contact.h"

#include "src/epp/contact/contact_change.h"
#include "src/epp/contact/util.h"
#include "src/epp/impl/disclose_policy.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/object/object_states_info.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/registrar/info_registrar.h"

#include <boost/mpl/assert.hpp>
#include <boost/variant.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Contact {

namespace {

void set_ContactUpdate_member(
    const boost::optional< Nullable<std::string> >& _input,
    Fred::UpdateContactByHandle& update_object,
    Fred::UpdateContactByHandle& (Fred::UpdateContactByHandle::*setter)(const std::string&))
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(_input))
    {
        (update_object.*setter)(ContactChange::get_value(_input));
    }
    else if (ContactChange::does_value_mean<ContactChange::Value::to_delete>(_input))
    {
        const std::string empty_string;
        (update_object.*setter)(empty_string);
    }
}

void set_ContactUpdate_member(
    const boost::optional< Nullable<std::string> >& _input,
    Fred::UpdateContactByHandle& update_object,
    Fred::UpdateContactByHandle& (Fred::UpdateContactByHandle::*setter)(const Nullable<std::string>&))
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(_input))
    {
        (update_object.*setter)(ContactChange::get_value(_input));
    }
    else if (ContactChange::does_value_mean<ContactChange::Value::to_delete>(_input))
    {
        const Nullable<std::string> value_meaning_to_delete;
        (update_object.*setter)(value_meaning_to_delete);
    }
}

bool has_data_changed(const boost::optional< Nullable<std::string> >& change,
                      const std::string& current_value)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        return ContactChange::get_value(change) != current_value;
    }
    if (ContactChange::does_value_mean<ContactChange::Value::to_delete>(change))
    {
        const bool current_value_means_deleted_value = current_value.empty();
        return !current_value_means_deleted_value;
    }
    return false;
}

bool has_data_changed(const boost::optional< Nullable<std::string> >& change,
                      const Nullable<std::string>& current_value)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        const std::string no_value;
        return ContactChange::get_value(change) != current_value.get_value_or(no_value);
    }
    if (ContactChange::does_value_mean<ContactChange::Value::to_delete>(change))
    {
        const bool current_value_means_deleted_value = current_value.isnull() ||
                                                       current_value.get_value().empty();
        return !current_value_means_deleted_value;
    }
    return false;
}

bool has_data_changed(const boost::optional< Nullable<std::string> >& change,
                      const Optional<std::string>& current_value)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        const std::string no_value;
        return ContactChange::get_value(change) != current_value.get_value_or(no_value);
    }
    if (ContactChange::does_value_mean< ContactChange::Value::to_delete >(change))
    {
        const bool current_value_means_deleted_value = !current_value.isset() ||
                                                       current_value.get_value().empty();
        return !current_value_means_deleted_value;
    }
    return false;
}

bool has_data_changed(const boost::optional<std::string>& change,
                      const std::string& current_value)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        return ContactChange::get_value(change) != current_value;
    }
    return false;
}

bool has_streets_changed(const std::vector< boost::optional< Nullable<std::string> > >& change,
                      const Fred::Contact::PlaceAddress& current_value)
{
    switch (change.size())
    {
        case 0:
            return false;
        case 1:
            return has_data_changed(change[0], current_value.street1);
        case 2:
            return has_data_changed(change[0], current_value.street1) ||
                   has_data_changed(change[1], current_value.street2);
        case 3:
            return has_data_changed(change[0], current_value.street1) ||
                   has_data_changed(change[1], current_value.street2) ||
                   has_data_changed(change[2], current_value.street3);
    }
    throw std::runtime_error("Too many streets.");
}

bool has_data(const boost::optional< Nullable<std::string> >& change)
{
    return ContactChange::does_value_mean<ContactChange::Value::to_set>(change);
}

bool has_data(const boost::optional<std::string>& change)
{
    return ContactChange::does_value_mean<ContactChange::Value::to_set>(change);
}

bool has_streets(const std::vector< boost::optional< Nullable<std::string> > >& change)
{
    bool result = false;
    switch (change.size())
    {
        case 3:  result |= has_data(change[2]);
        case 2:  result |= has_data(change[1]);
        case 1:  result |= has_data(change[0]);
        case 0:  return result;
        default: throw std::runtime_error("Too many streets.");
    }
}

bool has_place_changed(const ContactChange& changed_data,
                       const Nullable<Fred::Contact::PlaceAddress>& current_place)
{
    if (current_place.isnull())
    {
        return has_data(changed_data.city)              ||
               has_data(changed_data.state_or_province) ||
               has_data(changed_data.postal_code)       ||
               has_data(changed_data.country_code)      ||
               has_streets(changed_data.streets);
    }
    const Fred::Contact::PlaceAddress src = current_place.get_value();
    return has_data_changed(changed_data.city, src.city) ||
           has_data_changed(changed_data.state_or_province, src.stateorprovince) ||
           has_data_changed(changed_data.postal_code, src.postalcode) ||
           has_data_changed(changed_data.country_code, src.country) ||
           has_streets_changed(changed_data.streets, src);
}

void set_data(const boost::optional< Nullable<std::string> >& change, std::string& data)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        data = ContactChange::get_value(change);
    }
}

void set_data(const boost::optional<std::string>& change, std::string& data)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        data = ContactChange::get_value(change);
    }
}

void set_data(const boost::optional< Nullable<std::string> >& change, Optional<std::string>& data)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        data = ContactChange::get_value(change);
    }
}

bool should_address_be_disclosed(
    Fred::OperationContext& ctx,
    const Fred::InfoContactData& current_contact_data,
    const ContactChange& change)
{
    //don't touch organization => current value has to be checked
    if (ContactChange::does_value_mean< ContactChange::Value::not_to_touch >(change.organization))
    {
        const bool contact_is_organization = !current_contact_data.organization.isnull() &&
                                             !current_contact_data.organization.get_value().empty();
        if (contact_is_organization)
        {
            return true;
        }
    }
    //change organization => new value has to be checked
    else
    {
        const bool contact_will_be_organization =
                ContactChange::does_value_mean<ContactChange::Value::to_set>(change.organization);
        if (contact_will_be_organization)
        {
            return true;
        }
    }

    const bool contact_will_lose_identification =
        has_data_changed(change.email, current_contact_data.email) ||
        has_data_changed(change.telephone, current_contact_data.telephone) ||
        has_data_changed(change.name, current_contact_data.name) ||
        has_data_changed(change.organization, current_contact_data.organization) ||
        has_place_changed(change, current_contact_data.place);
    if (contact_will_lose_identification)
    {
        return true;
    }

    const Fred::ObjectStatesInfo contact_states(Fred::GetObjectStates(current_contact_data.id).exec(ctx));
    const bool contact_is_identified = contact_states.presents(Fred::Object_State::identified_contact) ||
                                       contact_states.presents(Fred::Object_State::validated_contact);
    return !contact_is_identified;
}

template <ContactDisclose::Item::Enum ITEM>
void set_ContactUpdate_discloseflag(Fred::UpdateContactByHandle& update_op, bool value);

template < >
void set_ContactUpdate_discloseflag<ContactDisclose::Item::name>(Fred::UpdateContactByHandle& update_op, bool value)
{
    update_op.set_disclosename(value);
}

template < >
void set_ContactUpdate_discloseflag<ContactDisclose::Item::organization>(Fred::UpdateContactByHandle& update_op, bool value)
{
    update_op.set_discloseorganization(value);
}

template < >
void set_ContactUpdate_discloseflag<ContactDisclose::Item::address>(Fred::UpdateContactByHandle& update_op, bool value)
{
    update_op.set_discloseaddress(value);
}

template < >
void set_ContactUpdate_discloseflag<ContactDisclose::Item::telephone>(Fred::UpdateContactByHandle& update_op, bool value)
{
    update_op.set_disclosetelephone(value);
}

template < >
void set_ContactUpdate_discloseflag<ContactDisclose::Item::fax>(Fred::UpdateContactByHandle& update_op, bool value)
{
    update_op.set_disclosefax(value);
}

template < >
void set_ContactUpdate_discloseflag<ContactDisclose::Item::email>(Fred::UpdateContactByHandle& update_op, bool value)
{
    update_op.set_discloseemail(value);
}

template < >
void set_ContactUpdate_discloseflag<ContactDisclose::Item::vat>(Fred::UpdateContactByHandle& update_op, bool value)
{
    update_op.set_disclosevat(value);
}

template < >
void set_ContactUpdate_discloseflag<ContactDisclose::Item::ident>(Fred::UpdateContactByHandle& update_op, bool value)
{
    update_op.set_discloseident(value);
}

template < >
void set_ContactUpdate_discloseflag<ContactDisclose::Item::notify_email>(Fred::UpdateContactByHandle& update_op, bool value)
{
    update_op.set_disclosenotifyemail(value);
}

template < ContactDisclose::Item::Enum ITEM >
void set_ContactUpdate_discloseflag(const ContactDisclose& disclose, Fred::UpdateContactByHandle& update_op)
{
    BOOST_MPL_ASSERT_MSG(ITEM != ContactDisclose::Item::address,
                         discloseflag_address_has_its_own_method,
                         (ContactDisclose::Item::Enum));
    const bool to_disclose = disclose.should_be_disclosed< ITEM >(is_the_default_policy_to_disclose());
    set_ContactUpdate_discloseflag<ITEM>(update_op, to_disclose);
}

void set_ContactUpdate_discloseflag_address(
    Fred::OperationContext& ctx,
    const ContactChange& change,
    const Fred::InfoContactData& contact_data_before_update,
    Fred::UpdateContactByHandle& update_op)
{
    bool address_has_to_be_disclosed =
        change.disclose->should_be_disclosed<ContactDisclose::Item::address>(is_the_default_policy_to_disclose());

    if (!address_has_to_be_disclosed)
    {
        static const bool address_has_to_be_hidden = true;
        const bool address_can_be_hidden = !should_address_be_disclosed(ctx,
                                                                        contact_data_before_update,
                                                                        change);
        if (address_has_to_be_hidden && !address_can_be_hidden)
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
        address_has_to_be_disclosed = !address_has_to_be_hidden || !address_can_be_hidden;
    }

    update_op.set_discloseaddress(address_has_to_be_disclosed);
}

Fred::InfoContactData info_contact_by_handle(const std::string& handle, Fred::OperationContext& ctx)
{
    try
    {
        // TODO admin_contact_verification_modification AdminContactVerificationObjectStates::conditionally_cancel_final_states( ) relies on this exclusive lock
        return Fred::InfoContactByHandle(handle).set_lock().exec(ctx).info_contact_data;
    }
    catch (const Fred::InfoContactByHandle::Exception& e)
    {
        e.is_set_unknown_contact_handle()
            ? throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist))
            : throw;
    }
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

} // namespace Epp::Contact::{anonymous}

unsigned long long update_contact(
        Fred::OperationContext& ctx,
        const std::string& contact_handle,
        const ContactChange& change,
        const UpdateContactConfigData& update_contact_config_data,
        const SessionData& session_data)
{
    if (!is_session_registrar_valid(session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    const bool contact_is_registered = Fred::Contact::get_handle_registrability(ctx, contact_handle) ==
                                       Fred::ContactHandleState::Registrability::registered;
    if (!contact_is_registered)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }

    const Fred::InfoContactData contact_data_before_update = info_contact_by_handle(contact_handle, ctx);

    const Fred::InfoRegistrarData session_registrar =
            Fred::InfoRegistrarById(session_data.registrar_id)
                    .exec(ctx)
                    .info_registrar_data;

    const bool is_sponsoring_registrar = (contact_data_before_update.sponsoring_registrar_handle ==
                                          session_registrar.handle);
    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    const bool operation_is_permitted = (is_sponsoring_registrar || is_system_registrar);

    if (!operation_is_permitted)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::registrar_autor,
                                                         Reason::unauthorized_registrar)));
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(contact_data_before_update.id).exec(ctx);
    Fred::PerformObjectStateRequest(contact_data_before_update.id).exec(ctx);

    if (!is_system_registrar)
    {
        const Fred::ObjectStatesInfo contact_states_before_update(Fred::GetObjectStates(contact_data_before_update.id).exec(ctx));
        if (contact_states_before_update.presents(Fred::Object_State::server_update_prohibited) ||
            contact_states_before_update.presents(Fred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    const ContactChange trimmed_change = trim(change);

    // when deleting or not-changing, no check of data is needed
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(trimmed_change.country_code))
    {
        if (!is_country_code_valid(ctx, ContactChange::get_value(trimmed_change.country_code)))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error)
                                             .add_extended_error(
                                                     EppExtendedError::of_scalar_parameter(
                                                             Param::contact_cc,
                                                             Reason::country_notexist)));
        }
    }

    // update itself
    {
        Fred::UpdateContactByHandle update(contact_handle, session_registrar.handle);

        set_ContactUpdate_member(trimmed_change.name, update, &Fred::UpdateContactByHandle::set_name);
        set_ContactUpdate_member(trimmed_change.organization, update, &Fred::UpdateContactByHandle::set_organization);
        set_ContactUpdate_member(trimmed_change.telephone, update, &Fred::UpdateContactByHandle::set_telephone);
        set_ContactUpdate_member(trimmed_change.fax, update, &Fred::UpdateContactByHandle::set_fax);
        set_ContactUpdate_member(trimmed_change.email, update, &Fred::UpdateContactByHandle::set_email);
        set_ContactUpdate_member(trimmed_change.notify_email, update, &Fred::UpdateContactByHandle::set_notifyemail);
        set_ContactUpdate_member(trimmed_change.vat, update, &Fred::UpdateContactByHandle::set_vat);
        set_ContactUpdate_member(trimmed_change.authinfopw, update, &Fred::UpdateContactByHandle::set_authinfo);

        {
            if (ContactChange::does_value_mean<ContactChange::Value::to_set>(trimmed_change.ident))
            {
                const ContactIdent ident = ContactChange::get_value(trimmed_change.ident);
                update.set_personal_id(boost::apply_visitor(
                        GetPersonalIdUnionFromContactIdent(),
                        ident));
            }
            else if (ContactChange::does_value_mean<ContactChange::Value::to_delete>(trimmed_change.ident))
            {
                update.set_personal_id(Nullable<Fred::PersonalIdUnion>());
            }
        }

        {
            if (ContactChange::does_value_mean<ContactChange::Value::to_set>(trimmed_change.mailing_address))
            {
                const ContactChange::Address src = ContactChange::get_value(trimmed_change.mailing_address);
                Fred::ContactAddress dst;
                if (src.street1 != boost::none)
                {
                    dst.street1 = *src.street1;
                }
                if ((src.street2 != boost::none) && !src.street2->empty())
                {
                    dst.street2 = *src.street2;
                }
                if ((src.street3 != boost::none) && !src.street3->empty())
                {
                    dst.street3 = *src.street3;
                }
                if (src.city != boost::none)
                {
                    dst.city = *src.city;
                }
                if ((src.state_or_province != boost::none) && !src.state_or_province->empty())
                {
                    dst.stateorprovince = *src.state_or_province;
                }
                if (src.postal_code != boost::none)
                {
                    dst.postalcode = *src.postal_code;
                }
                if (src.country_code != boost::none)
                {
                    dst.country = *src.country_code;
                }
                update.set_address<Fred::ContactAddressType::MAILING>(dst);
            }
            else if (ContactChange::does_value_mean<ContactChange::Value::to_delete>(trimmed_change.mailing_address))
            {
                update.reset_address<Fred::ContactAddressType::MAILING>();
            }
        }

        if (trimmed_change.disclose != boost::none)
        {
            trimmed_change.disclose->check_validity();
            set_ContactUpdate_discloseflag_address(ctx, trimmed_change, contact_data_before_update, update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::name>(*trimmed_change.disclose, update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::organization>(*trimmed_change.disclose, update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::telephone>(*trimmed_change.disclose, update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::fax>(*trimmed_change.disclose, update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::email>(*trimmed_change.disclose, update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::vat>(*trimmed_change.disclose, update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::ident>(*trimmed_change.disclose, update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::notify_email>(*trimmed_change.disclose, update);
        }
        else
        {
            const bool address_was_hidden = !contact_data_before_update.discloseaddress;
            if (address_was_hidden)
            {
                const bool address_has_to_be_disclosed = should_address_be_disclosed(ctx,
                                                                                     contact_data_before_update,
                                                                                     trimmed_change);
                if (address_has_to_be_disclosed)
                {
                    update.set_discloseaddress(true);
                }
            }
        }

        if (has_place_changed(trimmed_change, contact_data_before_update.place))
        {
            Fred::Contact::PlaceAddress new_place;
            switch (trimmed_change.streets.size())
            {
                case 3: set_data(trimmed_change.streets[2], new_place.street3);
                case 2: set_data(trimmed_change.streets[1], new_place.street2);
                case 1: set_data(trimmed_change.streets[0], new_place.street1);
                case 0: break;
                default: throw std::runtime_error("Too many streets.");
            }
            set_data(trimmed_change.city, new_place.city);
            set_data(trimmed_change.state_or_province, new_place.stateorprovince);
            set_data(trimmed_change.postal_code, new_place.postalcode);
            set_data(trimmed_change.country_code, new_place.country);
            update.set_place(new_place);
        }

        if (session_data.logd_request_id.isset())
        {
            update.set_logd_request_id(session_data.logd_request_id.get_value());
        }

        try
        {
            const unsigned long long new_history_id = update.exec(ctx);
            return new_history_id;
        }
        catch (const Fred::UpdateContactByHandle::ExceptionType& e)
        {
            // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
            if (e.is_set_forbidden_company_name_setting() ||
                e.is_set_unknown_registrar_handle() ||
                e.is_set_unknown_ssntype())
            {
                throw;
            }

            if (e.is_set_unknown_contact_handle())
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
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
}

} // namespace Epp::Contact
} // namespace Epp
