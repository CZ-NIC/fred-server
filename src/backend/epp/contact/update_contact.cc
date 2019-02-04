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
#include "src/backend/epp/contact/impl/get_personal_id_union.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/impl/util.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object_state/lock_object_state_request_lock.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrar/info_registrar.hh"

#include <boost/mpl/assert.hpp>
#include <boost/variant.hpp>

#include <string>
#include <type_traits>
#include <vector>

namespace Epp {
namespace Contact {

namespace {

template <typename U>
U& update_privacy(
        U& updater,
        const boost::optional<PrivacyPolicy>& publishability,
        U&(LibFred::UpdateContact<U>::*privacy_setter)(bool))
{
    if (static_cast<bool>(publishability))
    {
        (updater.*privacy_setter)(*publishability == PrivacyPolicy::show);
    }
    return updater;
}

template <typename U, typename O, typename T>
U& update_value(
        U& updater,
        const O& operation,
        U&(LibFred::UpdateContact<U>::*value_setter)(const T&))
{
    if (operation == UpdateOperation::Action::set_value)
    {
        return (updater.*value_setter)(*operation);
    }
    if (operation == UpdateOperation::Action::delete_value)
    {
        const T value_meaning_to_delete;
        return (updater.*value_setter)(value_meaning_to_delete);
    }
    return updater;
}

using ContactUpdater = LibFred::UpdateContact<LibFred::UpdateContactByHandle>;

template <typename O, typename T>
void update_operation_to_value(const O& operation, T& value)
{
    if (operation == UpdateOperation::Action::set_value)
    {
        value = *operation;
    }
    else if (operation == UpdateOperation::Action::delete_value)
    {
        const T value_meaning_to_delete;
        value = value_meaning_to_delete;
    }
}

template <>
LibFred::UpdateContactByHandle&
update_value<LibFred::UpdateContactByHandle,
             ContactChange::MainAddress,
             Nullable<LibFred::Contact::PlaceAddress>>(
        LibFred::UpdateContactByHandle& updater,
        const ContactChange::MainAddress& address,
        LibFred::UpdateContactByHandle&(ContactUpdater::*value_setter)(const Nullable<LibFred::Contact::PlaceAddress>&))
{
    if ((address.street[0] == UpdateOperation::Action::no_operation) &&
        (address.street[1] == UpdateOperation::Action::no_operation) &&
        (address.street[2] == UpdateOperation::Action::no_operation) &&
        (address.city == UpdateOperation::Action::no_operation) &&
        (address.state_or_province == UpdateOperation::Action::no_operation) &&
        (address.postal_code == UpdateOperation::Action::no_operation) &&
        (address.country_code == UpdateOperation::Action::no_operation))
    {
        return updater;
    }
    if ((address.street[0] == UpdateOperation::Action::delete_value) &&
        (address.street[1] == UpdateOperation::Action::delete_value) &&
        (address.street[2] == UpdateOperation::Action::delete_value) &&
        (address.city == UpdateOperation::Action::delete_value) &&
        (address.state_or_province == UpdateOperation::Action::delete_value) &&
        (address.postal_code == UpdateOperation::Action::delete_value) &&
        (address.country_code == UpdateOperation::Action::delete_value))
    {
        const Nullable<LibFred::Contact::PlaceAddress> value_meaning_to_delete;
        return (updater.*value_setter)(value_meaning_to_delete);
    }
    LibFred::Contact::PlaceAddress dst_address;
    update_operation_to_value(address.street[0], dst_address.street1);
    update_operation_to_value(address.street[1], dst_address.street2);
    update_operation_to_value(address.street[2], dst_address.street3);
    update_operation_to_value(address.city, dst_address.city);
    update_operation_to_value(address.state_or_province, dst_address.stateorprovince);
    update_operation_to_value(address.postal_code, dst_address.postalcode);
    update_operation_to_value(address.country_code, dst_address.country);
    return (updater.*value_setter)(dst_address);
}

template <>
LibFred::UpdateContactByHandle&
update_value<LibFred::UpdateContactByHandle,
             Deletable<ContactIdent>,
             Nullable<LibFred::PersonalIdUnion>>(
        LibFred::UpdateContactByHandle& updater,
        const Deletable<ContactIdent>& operation,
        LibFred::UpdateContactByHandle&(ContactUpdater::*value_setter)(const Nullable<LibFred::PersonalIdUnion>&))
{
    if (operation == UpdateOperation::Action::set_value)
    {
        const auto ident = *operation;
        return (updater.*value_setter)(Impl::get_personal_id_union(ident));
    }
    if (operation == UpdateOperation::Action::delete_value)
    {
        return (updater.*value_setter)(Nullable<LibFred::PersonalIdUnion>());
    }
    return updater;
}

template <typename T>
void optional_to_value(const boost::optional<std::string>&, T&);

template <>
void optional_to_value<std::string>(const boost::optional<std::string>& src, std::string& dst)
{
    if (static_cast<bool>(src))
    {
        dst = *src;
    }
}

template <>
void optional_to_value<Optional<std::string>>(const boost::optional<std::string>& src, Optional<std::string>& dst)
{
    if (static_cast<bool>(src) && !src->empty())
    {
        dst = *src;
    }
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
        if (e.is_set_unknown_contact_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
        throw;
    }
}

}//namespace Epp::Contact::{anonymous}

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
                EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
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

    const auto trimmed_change = change.get_trimmed_copy();

    EppResultFailure parameter_value_policy_errors(EppResultCode::parameter_value_policy_error);

    // when deleting or not-changing, no check of data is needed
    if (trimmed_change.address.country_code == UpdateOperation::Action::set_value)
    {
        if (!is_country_code_valid(
                    ctx,
                    *trimmed_change.address.country_code))
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_scalar_parameter(
                            Param::contact_cc,
                            Reason::country_notexist));
        }
    }

    // when deleting or not-changing, no check of data is needed
    if (trimmed_change.mailing_address == UpdateOperation::Action::set_value)
    {
        const auto mailing_address = *trimmed_change.mailing_address;
        if (mailing_address.country_code != boost::none)
        {
            if (!is_country_code_valid(ctx, *mailing_address.country_code))
            {
                parameter_value_policy_errors.add_unspecified_error();
            }
        }
    }

    if (!parameter_value_policy_errors.empty())
    {
        throw EppResponseFailure(parameter_value_policy_errors);
    }

    // update itself
    {
        LibFred::UpdateContactByHandle updater(contact_handle, session_registrar.handle);

        update_value(
                updater,
                trimmed_change.name,
                &LibFred::UpdateContactByHandle::set_name);
        update_value(
                updater,
                trimmed_change.organization,
                &LibFred::UpdateContactByHandle::set_organization);
        update_value(
                updater,
                trimmed_change.address,
                &LibFred::UpdateContactByHandle::set_place);
        update_value(
                updater,
                trimmed_change.telephone,
                &LibFred::UpdateContactByHandle::set_telephone);
        update_value(
                updater,
                trimmed_change.fax,
                &LibFred::UpdateContactByHandle::set_fax);
        update_value(
                updater,
                trimmed_change.email,
                &LibFred::UpdateContactByHandle::set_email);
        update_value(
                updater,
                trimmed_change.notify_email,
                &LibFred::UpdateContactByHandle::set_notifyemail);
        update_value(
                updater,
                trimmed_change.vat,
                &LibFred::UpdateContactByHandle::set_vat);
        update_value(
                updater,
                trimmed_change.ident,
                &LibFred::UpdateContactByHandle::set_personal_id);
        update_value(
                updater,
                trimmed_change.authinfopw,
                &LibFred::UpdateContactByHandle::set_authinfo);

        if (trimmed_change.disclose == UpdateOperation::Action::set_value)
        {
            update_privacy(
                    updater,
                    (*trimmed_change.disclose).name,
                    &LibFred::UpdateContactByHandle::set_disclosename);
            update_privacy(
                    updater,
                    (*trimmed_change.disclose).organization,
                    &LibFred::UpdateContactByHandle::set_discloseorganization);
            update_privacy(
                    updater,
                    (*trimmed_change.disclose).address,
                    &LibFred::UpdateContactByHandle::set_discloseaddress);
            update_privacy(
                    updater,
                    (*trimmed_change.disclose).telephone,
                    &LibFred::UpdateContactByHandle::set_disclosetelephone);
            update_privacy(
                    updater,
                    (*trimmed_change.disclose).fax,
                    &LibFred::UpdateContactByHandle::set_disclosefax);
            update_privacy(
                    updater,
                    (*trimmed_change.disclose).email,
                    &LibFred::UpdateContactByHandle::set_discloseemail);
            update_privacy(
                    updater,
                    (*trimmed_change.disclose).notify_email,
                    &LibFred::UpdateContactByHandle::set_disclosenotifyemail);
            update_privacy(
                    updater,
                    (*trimmed_change.disclose).vat,
                    &LibFred::UpdateContactByHandle::set_disclosevat);
            update_privacy(
                    updater,
                    (*trimmed_change.disclose).ident,
                    &LibFred::UpdateContactByHandle::set_discloseident);
        }

        if (trimmed_change.mailing_address == UpdateOperation::Action::set_value)
        {
            const ContactChange::Address src_address = *trimmed_change.mailing_address;
            LibFred::ContactAddress dst_address;
            optional_to_value(src_address.street[0], dst_address.street1);
            optional_to_value(src_address.street[1], dst_address.street2);
            optional_to_value(src_address.street[2], dst_address.street3);
            optional_to_value(src_address.city, dst_address.city);
            optional_to_value(src_address.state_or_province, dst_address.stateorprovince);
            optional_to_value(src_address.postal_code, dst_address.postalcode);
            optional_to_value(src_address.country_code, dst_address.country);
            updater.set_address<LibFred::ContactAddressType::MAILING>(dst_address);
        }
        else if (trimmed_change.mailing_address == UpdateOperation::Action::delete_value)
        {
            updater.reset_address<LibFred::ContactAddressType::MAILING>();
        }

        if (session_data.logd_request_id.isset())
        {
            updater.set_logd_request_id(session_data.logd_request_id.get_value());
        }

        try
        {
            update_contact_config_data.get_data_filter()(
                    ctx,
                    contact_data_before_update,
                    change,
                    session_data,
                    updater);
        }
        catch (const UpdateContactDataFilter::DiscloseflagRulesViolation&)
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }

        try
        {
            const unsigned long long new_history_id = updater.exec(ctx);
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

}//namespace Epp::Contact
}//namespace Epp
