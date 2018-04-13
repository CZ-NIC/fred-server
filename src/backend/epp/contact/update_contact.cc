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

#include "src/backend/epp/contact/update_contact.hh"

#include "src/backend/epp/contact/contact_change.hh"
#include "src/backend/epp/contact/util.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/impl/disclose_policy.hh"
#include "src/backend/epp/impl/util.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/libfred/object_state/lock_object_state_request_lock.hh"
#include "src/libfred/object_state/perform_object_state_request.hh"
#include "src/libfred/registrable_object/contact/check_contact.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/contact/update_contact.hh"
#include "src/libfred/registrar/info_registrar.hh"

#include <boost/mpl/assert.hpp>
#include <boost/variant.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Contact {

namespace {

void set_ContactUpdate_member(
        const boost::optional<Nullable<std::string> >& _input,
        LibFred::UpdateContactByHandle& update_object,
        LibFred::UpdateContactByHandle& (LibFred::UpdateContactByHandle::* setter) (const std::string&))
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
        const boost::optional<Nullable<std::string> >& _input,
        LibFred::UpdateContactByHandle& update_object,
        LibFred::UpdateContactByHandle& (LibFred::UpdateContactByHandle::* setter) (const Nullable<std::string>&))
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


bool has_data_changed(
        const boost::optional<Nullable<std::string> >& change,
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


bool has_data_changed(
        const boost::optional<Nullable<std::string> >& change,
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


bool has_data_changed(
        const boost::optional<Nullable<std::string> >& change,
        const Optional<std::string>& current_value)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        const std::string no_value;
        return ContactChange::get_value(change) != current_value.get_value_or(no_value);
    }
    if (ContactChange::does_value_mean<ContactChange::Value::to_delete>(change))
    {
        const bool current_value_means_deleted_value = !current_value.isset() ||
                                                       current_value.get_value().empty();
        return !current_value_means_deleted_value;
    }
    return false;
}


bool has_data_changed(
        const boost::optional<std::string>& change,
        const std::string& current_value)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        return ContactChange::get_value(change) != current_value;
    }
    return false;
}


bool has_streets(const boost::optional<std::vector<std::string>>& change)
{
    return ContactChange::does_value_mean<ContactChange::Value::to_set>(change);
}


bool has_streets_changed(const boost::optional<std::vector<std::string>>& change,
                         const LibFred::Contact::PlaceAddress& current_value)
{
    if (!has_streets(change))
    {
        return false;
    }
    switch (change->size())
    {
        case 0:
            return !(current_value.street1.empty() &&
                     current_value.street2.get_value_or_default().empty() &&
                     current_value.street3.get_value_or_default().empty());
        case 1:
            return !((current_value.street1 == (*change)[0]) &&
                     current_value.street2.get_value_or_default().empty() &&
                     current_value.street3.get_value_or_default().empty());
        case 2:
            return !((current_value.street1 == (*change)[0]) &&
                     (current_value.street2.get_value_or_default() == (*change)[1]) &&
                     current_value.street3.get_value_or_default().empty());
        case 3:
            return !((current_value.street1 == (*change)[0]) &&
                     (current_value.street2.get_value_or_default() == (*change)[1]) &&
                     (current_value.street3.get_value_or_default() == (*change)[2]));
    }
    throw std::runtime_error("Too many streets.");
}


bool has_data(const boost::optional<Nullable<std::string> >& change)
{
    return ContactChange::does_value_mean<ContactChange::Value::to_set>(change);
}


bool has_data(const boost::optional<std::string>& change)
{
    return ContactChange::does_value_mean<ContactChange::Value::to_set>(change);
}


bool has_place_changed(const ContactChange& changed_data,
                       const Nullable<LibFred::Contact::PlaceAddress>& current_place)
{
    if (current_place.isnull())
    {
        return has_data(changed_data.city)              ||
               has_data(changed_data.state_or_province) ||
               has_data(changed_data.postal_code)       ||
               has_data(changed_data.country_code)      ||
               has_streets(changed_data.streets);
    }
    const LibFred::Contact::PlaceAddress src = current_place.get_value();
    return has_data_changed(
            changed_data.city,
            src.city) ||
           has_data_changed(
            changed_data.state_or_province,
            src.stateorprovince) ||
           has_data_changed(
            changed_data.postal_code,
            src.postalcode) ||
           has_data_changed(
            changed_data.country_code,
            src.country) ||
           has_streets_changed(
            changed_data.streets,
            src);
}


void set_data(
        const boost::optional<Nullable<std::string> >& change,
        std::string& data)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        data = ContactChange::get_value(change);
    }
}


void set_data(
        const boost::optional<std::string>& change,
        std::string& data)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        data = ContactChange::get_value(change);
    }
}


void set_data(
        const boost::optional<Nullable<std::string> >& change,
        Optional<std::string>& data)
{
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(change))
    {
        data = ContactChange::get_value(change);
    }
}


bool should_address_be_disclosed(
        LibFred::OperationContext& ctx,
        const LibFred::InfoContactData& current_contact_data,
        const ContactChange& change)
{
    // don't touch organization => current value has to be checked
    if (ContactChange::does_value_mean<ContactChange::Value::not_to_touch>(change.organization))
    {
        const bool contact_is_organization = !current_contact_data.organization.isnull() &&
                                             !current_contact_data.organization.get_value().empty();
        if (contact_is_organization)
        {
            return true;
        }
    }
    // change organization => new value has to be checked
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
        has_data_changed(
                change.email,
                current_contact_data.email) ||
        has_data_changed(
                change.telephone,
                current_contact_data.telephone) ||
        has_data_changed(
                change.name,
                current_contact_data.name) ||
        has_data_changed(
                change.organization,
                current_contact_data.organization) ||
        has_place_changed(
                change,
                current_contact_data.place);
    if (contact_will_lose_identification)
    {
        return true;
    }

    const LibFred::ObjectStatesInfo contact_states(LibFred::GetObjectStates(current_contact_data.id).exec(ctx));
    const bool contact_is_identified = contact_states.presents(LibFred::Object_State::identified_contact) ||
                                       contact_states.presents(LibFred::Object_State::validated_contact);
    return !contact_is_identified;
}


template <ContactDisclose::Item::Enum ITEM>
void set_ContactUpdate_discloseflag(
        LibFred::UpdateContactByHandle& update_op,
        bool value);


template <>
void set_ContactUpdate_discloseflag<ContactDisclose::Item::name>(
        LibFred::UpdateContactByHandle& update_op,
        bool value)
{
    // update_op.set_disclosename(value);
}


template <>
void set_ContactUpdate_discloseflag<ContactDisclose::Item::organization>(
        LibFred::UpdateContactByHandle& update_op,
        bool value)
{
    // update_op.set_discloseorganization(value);
}


template <>
void set_ContactUpdate_discloseflag<ContactDisclose::Item::address>(
        LibFred::UpdateContactByHandle& update_op,
        bool value)
{
    update_op.set_discloseaddress(value);
}


template <>
void set_ContactUpdate_discloseflag<ContactDisclose::Item::telephone>(
        LibFred::UpdateContactByHandle& update_op,
        bool value)
{
    update_op.set_disclosetelephone(value);
}


template <>
void set_ContactUpdate_discloseflag<ContactDisclose::Item::fax>(
        LibFred::UpdateContactByHandle& update_op,
        bool value)
{
    update_op.set_disclosefax(value);
}


template <>
void set_ContactUpdate_discloseflag<ContactDisclose::Item::email>(
        LibFred::UpdateContactByHandle& update_op,
        bool value)
{
    update_op.set_discloseemail(value);
}


template <>
void set_ContactUpdate_discloseflag<ContactDisclose::Item::vat>(
        LibFred::UpdateContactByHandle& update_op,
        bool value)
{
    update_op.set_disclosevat(value);
}


template <>
void set_ContactUpdate_discloseflag<ContactDisclose::Item::ident>(
        LibFred::UpdateContactByHandle& update_op,
        bool value)
{
    update_op.set_discloseident(value);
}


template <>
void set_ContactUpdate_discloseflag<ContactDisclose::Item::notify_email>(
        LibFred::UpdateContactByHandle& update_op,
        bool value)
{
    update_op.set_disclosenotifyemail(value);
}


template <ContactDisclose::Item::Enum ITEM>
void set_ContactUpdate_discloseflag(
        const ContactDisclose& disclose,
        LibFred::UpdateContactByHandle& update_op)
{
    BOOST_MPL_ASSERT_MSG(
            ITEM != ContactDisclose::Item::address,
            discloseflag_address_has_its_own_method,
            (ContactDisclose::Item::Enum));
    const bool to_disclose = disclose.should_be_disclosed<ITEM>(is_the_default_policy_to_disclose());
    set_ContactUpdate_discloseflag<ITEM>(
            update_op,
            to_disclose);
}


void set_ContactUpdate_discloseflag_address(
        LibFred::OperationContext& ctx,
        const ContactChange& change,
        const LibFred::InfoContactData& contact_data_before_update,
        LibFred::UpdateContactByHandle& update_op)
{
    bool address_has_to_be_disclosed =
        change.disclose->should_be_disclosed<ContactDisclose::Item::address>(
                is_the_default_policy_to_disclose());

    if (!address_has_to_be_disclosed)
    {
        static const bool address_has_to_be_hidden = true;
        const bool address_can_be_hidden = !should_address_be_disclosed(
                ctx,
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


LibFred::InfoContactData info_contact_by_handle(
        const std::string& handle,
        LibFred::OperationContext& ctx)
{
    try
    {
        // TODO admin_contact_verification_modification Fred::Backend::Admin::Contact::Verification::ContactStates::conditionally_cancel_final_states( ) relies on this exclusive lock
        return LibFred::InfoContactByHandle(handle).set_lock().exec(ctx).info_contact_data;
    }
    catch (const LibFred::InfoContactByHandle::Exception& e)
    {
        e.is_set_unknown_contact_handle()
        ? throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist))
        : throw;
    }
}


struct GetPersonalIdUnionFromContactIdent
    : boost::static_visitor<LibFred::PersonalIdUnion>
{
    LibFred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Op>& src) const
    {
        return LibFred::PersonalIdUnion::get_OP(src.value);
    }

    LibFred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Pass>& src) const
    {
        return LibFred::PersonalIdUnion::get_PASS(src.value);
    }

    LibFred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Ico>& src) const
    {
        return LibFred::PersonalIdUnion::get_ICO(src.value);
    }

    LibFred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Mpsv>& src) const
    {
        return LibFred::PersonalIdUnion::get_MPSV(src.value);
    }

    LibFred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Birthday>& src) const
    {
        return LibFred::PersonalIdUnion::get_BIRTHDAY(src.value);
    }

};

} // namespace Epp::Contact::{anonymous}

unsigned long long update_contact(
        LibFred::OperationContext& ctx,
        const std::string& contact_handle,
        const ContactChange& change,
        const UpdateContactConfigData& update_contact_config_data,
        const SessionData& session_data)
{
    if (!is_session_registrar_valid(session_data))
    {
        throw EppResponseFailure(
                EppResultFailure(
                        EppResultCode::
                        authentication_error_server_closing_connection));
    }

    const LibFred::InfoContactData contact_data_before_update = info_contact_by_handle(
            contact_handle,
            ctx);

    const LibFred::InfoRegistrarData session_registrar =
        LibFred::InfoRegistrarById(session_data.registrar_id)
        .exec(ctx)
        .info_registrar_data;

    const bool is_sponsoring_registrar = (contact_data_before_update.sponsoring_registrar_handle ==
                                          session_registrar.handle);
    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    const bool operation_is_permitted = (is_sponsoring_registrar || is_system_registrar);

    if (!operation_is_permitted)
    {
        throw EppResponseFailure(
                EppResultFailure(EppResultCode::authorization_error)
                .add_extended_error(
                        EppExtendedError::of_scalar_parameter(
                                Param::registrar_autor,
                                Reason::unauthorized_registrar)));
    }

    // do it before any object state related checks
    LibFred::LockObjectStateRequestLock(contact_data_before_update.id).exec(ctx);
    LibFred::PerformObjectStateRequest(contact_data_before_update.id).exec(ctx);

    if (!is_system_registrar)
    {
        const LibFred::ObjectStatesInfo contact_states_before_update(LibFred::GetObjectStates(
                        contact_data_before_update.id).exec(ctx));
        if (contact_states_before_update.presents(LibFred::Object_State::server_update_prohibited) ||
            contact_states_before_update.presents(LibFred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    const ContactChange trimmed_change = trim(change);

    EppResultFailure parameter_value_policy_errors(EppResultCode::parameter_value_policy_error);

    // when deleting or not-changing, no check of data is needed
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(trimmed_change.country_code))
    {
        if (!is_country_code_valid(
                    ctx,
                    ContactChange::get_value(trimmed_change.country_code)))
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_scalar_parameter(
                            Param::contact_cc,
                            Reason::country_notexist));
        }

    }

    // when deleting or not-changing, no check of data is needed
    if (ContactChange::does_value_mean<ContactChange::Value::to_set>(trimmed_change.mailing_address))
    {
        const ContactChange::Address mailing_address =
            ContactChange::get_value(trimmed_change.mailing_address);
        if (static_cast<bool>(mailing_address.country_code))
        {
            if (!is_country_code_valid(
                        ctx,
                        *mailing_address.country_code))
            {
                throw EppResponseFailure(parameter_value_policy_errors);
            }
        }
    }

    if (!parameter_value_policy_errors.empty())
    {
        throw EppResponseFailure(parameter_value_policy_errors);
    }

    // update itself
    {
        LibFred::UpdateContactByHandle update(contact_handle, session_registrar.handle);

        set_ContactUpdate_member(
                trimmed_change.name,
                update,
                &LibFred::UpdateContactByHandle::set_name);
        set_ContactUpdate_member(
                trimmed_change.organization,
                update,
                &LibFred::UpdateContactByHandle::set_organization);
        set_ContactUpdate_member(
                trimmed_change.telephone,
                update,
                &LibFred::UpdateContactByHandle::set_telephone);
        set_ContactUpdate_member(
                trimmed_change.fax,
                update,
                &LibFred::UpdateContactByHandle::set_fax);
        set_ContactUpdate_member(
                trimmed_change.email,
                update,
                &LibFred::UpdateContactByHandle::set_email);
        set_ContactUpdate_member(
                trimmed_change.notify_email,
                update,
                &LibFred::UpdateContactByHandle::set_notifyemail);
        set_ContactUpdate_member(
                trimmed_change.vat,
                update,
                &LibFred::UpdateContactByHandle::set_vat);
        set_ContactUpdate_member(
                trimmed_change.authinfopw,
                update,
                &LibFred::UpdateContactByHandle::set_authinfo);

        {
            if (ContactChange::does_value_mean<ContactChange::Value::to_set>(trimmed_change.ident))
            {
                const ContactIdent ident = ContactChange::get_value(trimmed_change.ident);
                update.set_personal_id(
                        boost::apply_visitor(
                                GetPersonalIdUnionFromContactIdent(),
                                ident));
            }
            else if (ContactChange::does_value_mean<ContactChange::Value::to_delete>(trimmed_change.ident))
            {
                update.set_personal_id(Nullable<LibFred::PersonalIdUnion>());
            }
        }

        {
            if (ContactChange::does_value_mean<ContactChange::Value::to_set>(trimmed_change.mailing_address))
            {
                const ContactChange::Address src = ContactChange::get_value(trimmed_change.mailing_address);
                LibFred::ContactAddress dst;
                if (static_cast<bool>(src.street1))
                {
                    dst.street1 = *src.street1;
                }
                if ((static_cast<bool>(src.street2)) && !src.street2->empty())
                {
                    dst.street2 = *src.street2;
                }
                if ((static_cast<bool>(src.street3)) && !src.street3->empty())
                {
                    dst.street3 = *src.street3;
                }
                if (static_cast<bool>(src.city))
                {
                    dst.city = *src.city;
                }
                if ((static_cast<bool>(src.state_or_province)) && !src.state_or_province->empty())
                {
                    dst.stateorprovince = *src.state_or_province;
                }
                if (static_cast<bool>(src.postal_code))
                {
                    dst.postalcode = *src.postal_code;
                }
                if (static_cast<bool>(src.country_code))
                {
                    dst.country = *src.country_code;
                }
                update.set_address<LibFred::ContactAddressType::MAILING>(dst);
            }
            else if (ContactChange::does_value_mean<ContactChange::Value::to_delete>(
                             trimmed_change.
                             mailing_address))
            {
                update.reset_address<LibFred::ContactAddressType::MAILING>();
            }
        }

        if (static_cast<bool>(trimmed_change.disclose))
        {
            trimmed_change.disclose->check_validity();
            set_ContactUpdate_discloseflag_address(
                    ctx,
                    trimmed_change,
                    contact_data_before_update,
                    update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::name>(
                    *trimmed_change.disclose,
                    update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::organization>(
                    *trimmed_change.disclose,
                    update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::telephone>(
                    *trimmed_change.disclose,
                    update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::fax>(
                    *trimmed_change.disclose,
                    update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::email>(
                    *trimmed_change.disclose,
                    update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::vat>(
                    *trimmed_change.disclose,
                    update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::ident>(
                    *trimmed_change.disclose,
                    update);
            set_ContactUpdate_discloseflag<ContactDisclose::Item::notify_email>(
                    *trimmed_change.disclose,
                    update);
        }
        else
        {
            const bool address_was_hidden = !contact_data_before_update.discloseaddress;
            if (address_was_hidden)
            {
                const bool address_has_to_be_disclosed = should_address_be_disclosed(
                        ctx,
                        contact_data_before_update,
                        trimmed_change);
                if (address_has_to_be_disclosed)
                {
                    update.set_discloseaddress(true);
                }
            }
        }

        if (has_place_changed(
                    trimmed_change,
                    contact_data_before_update.place))
        {
            LibFred::Contact::PlaceAddress new_place;
            if (ContactChange::does_value_mean<ContactChange::Value::to_set>(trimmed_change.streets))
            {
                const auto streets = ContactChange::get_value(trimmed_change.streets);
                switch (streets.size())
                {
                    case 3: new_place.street3 = streets[2];
                    case 2: new_place.street2 = streets[1];
                    case 1: new_place.street1 = streets[0];
                    case 0: break;
                    default: throw std::runtime_error("Too many streets.");
                }
            }
            set_data(
                    trimmed_change.city,
                    new_place.city);
            set_data(
                    trimmed_change.state_or_province,
                    new_place.stateorprovince);
            set_data(
                    trimmed_change.postal_code,
                    new_place.postalcode);
            set_data(
                    trimmed_change.country_code,
                    new_place.country);
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
        catch (const LibFred::UpdateContactByHandle::ExceptionType& e)
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
                throw EppResponseFailure(
                        EppResultFailure(EppResultCode::parameter_value_policy_error)
                                .add_extended_error(
                                        EppExtendedError::of_scalar_parameter(
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
