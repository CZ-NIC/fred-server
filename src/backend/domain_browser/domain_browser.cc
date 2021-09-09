/*
 * Copyright (C) 2014-2021  CZ.NIC, z. s. p. o.
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

#include "src/backend/domain_browser/domain_browser.hh"

#include "libfred/object/object_impl.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object_state/cancel_object_state_request_id.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/get_object_state_descriptions.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/object_state/object_has_state.hh"
#include "src/deprecated/libfred/object_state/object_state_name.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrar/info_registrar.hh"
#include "util/log/context.hh"
#include "util/map_at.hh"
#include "util/random/random.hh"
#include "util/util.hh"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace DomainBrowser {

const std::string DomainBrowser::output_timezone("UTC");

/**
 * String for logging context
 * @param _name is server name
 * @return "server_name-<call_id>"
 */
static const std::string create_ctx_name(const std::string& _name)
{
    return boost::str(boost::format("%1%-<%2%>") % _name % Random::Generator().get(0, 10000));
}


/**
 * Logs LibFred::OperationException as warning, LibFred::InternalError, std::exception children and other exceptions as error, then rethrows.
 * @param ctx contains reference to database and logging interface
 */
static void log_and_rethrow_exception_handler(LibFred::OperationContext& ctx)
{
    try
    {
        throw;
    }
    catch (const LibFred::OperationException& ex)
    {
        ctx.get_log().warning(ex.what());
        ctx.get_log().warning(
                boost::algorithm::replace_all_copy(
                        boost::diagnostic_information(ex),
                        "\n",
                        " "));
        throw;
    }
    catch (const LibFred::InternalError& ex)
    {
        ctx.get_log().error(
                boost::algorithm::replace_all_copy(
                        ex.get_exception_stack_info(),
                        "\n",
                        " "));
        ctx.get_log().error(
                boost::algorithm::replace_all_copy(
                        boost::diagnostic_information(ex),
                        "\n",
                        " "));
        ctx.get_log().error(ex.what());
        throw;
    }
    catch (const std::exception& ex)
    {
        ctx.get_log().error(ex.what());
        throw;
    }
    catch (...)
    {
        ctx.get_log().error("unknown exception");
        throw;
    }
}

namespace {

bool is_attached_to_identity(
        const LibFred::OperationContext& ctx,
        unsigned long long contact_id)
{
    return 0 < ctx.get_conn().exec_params(
            "SELECT 0 "
            "FROM contact_identity "
            "WHERE contact_id = $1::BIGINT AND "
                  "valid_to IS NULL "
            "LIMIT 1", Database::query_param_list{contact_id}).size();
}

bool is_at_least_identified(
        LibFred::OperationContext& ctx,
        unsigned long long contact_id)
{
    const auto state_flags = LibFred::ObjectStatesInfo{LibFred::GetObjectStates{contact_id}.exec(ctx)};
    if (state_flags.presents(LibFred::Object_State::mojeid_contact))
    {
        return state_flags.presents(LibFred::Object_State::identified_contact) ||
               state_flags.presents(LibFred::Object_State::validated_contact);
    }
    return is_attached_to_identity(ctx, contact_id);
}

bool is_at_least_validated(
        LibFred::OperationContext& ctx,
        unsigned long long contact_id)
{
    const auto state_flags = LibFred::ObjectStatesInfo{LibFred::GetObjectStates{contact_id}.exec(ctx)};
    if (state_flags.presents(LibFred::Object_State::mojeid_contact))
    {
        return state_flags.presents(LibFred::Object_State::validated_contact);
    }
    return is_attached_to_identity(ctx, contact_id);
}

bool has_domainbrowser_allowed(
        LibFred::OperationContext& ctx,
        unsigned long long contact_id)
{
    const auto state_flags = LibFred::ObjectStatesInfo{LibFred::GetObjectStates{contact_id}.exec(ctx)};
    return state_flags.presents(LibFred::Object_State::mojeid_contact) ||
           is_attached_to_identity(ctx, contact_id);
}

}//namespace Fred::Backend::DomainBrowser::{anonymous}

/**
 * Check contact.
 * @param EXCEPTION is type of exception used for reporting when contact is not found
 * @param ctx contains reference to database and logging interface
 * @param contact_id is database id of contact
 * @param lock_contact_for_update indicates whether to lock contact for update (true) or for share (false)
 * @return contact info or if contact is deleted throw @ref EXCEPTION.
 */
template <class EXCEPTION>
LibFred::InfoContactOutput check_contact_id(
        LibFred::OperationContext& ctx,
        unsigned long long user_contact_id,
        const std::string& output_timezone,
        bool lock_contact_for_update = false)
{
    LibFred::InfoContactOutput info;
    try
    {
        LibFred::InfoContactById info_contact_by_id(user_contact_id);
        if (lock_contact_for_update)
        {
            info_contact_by_id.set_lock();
        }
        info = info_contact_by_id.exec(
                ctx,
                output_timezone);
        LibFred::PerformObjectStateRequest(user_contact_id).exec(ctx);
    }
    catch (const LibFred::InfoContactById::Exception& ex)
    {
        if (ex.is_set_unknown_object_id())
        {
            throw EXCEPTION();
        }
        else
        {
            throw;
        }
    }

    return info;
}


/**
 * Check user contact.
 * @param EXCEPTION is type of exception used for reporting when contact is not found or not in required state
 * @param ctx contains reference to database and logging interface
 * @param user_contact_id is database id of user contact
 * @param lock_contact_for_update indicates whether to lock contact for update (true) or for share (false)
 * @return contact info or if user contact is deleted or don't have mojeidContact state throw @ref EXCEPTION.
 */
template <class EXCEPTION>
LibFred::InfoContactOutput check_user_contact_id(
        LibFred::OperationContext& ctx,
        unsigned long long user_contact_id,
        const std::string& output_timezone,
        bool lock_contact_for_update = false)
{
    LibFred::InfoContactOutput info = check_contact_id<EXCEPTION>(
            ctx,
            user_contact_id,
            output_timezone,
            lock_contact_for_update);

    if (!has_domainbrowser_allowed(ctx, user_contact_id))
    {
        throw EXCEPTION();
    }

    return info;
}


/**
 * Split object states string
 * @param _array_string object state codes delimited with ','
 * @return vector of state codes
 */
static std::vector<std::string> split_object_states_string(const std::string& _array_string)
{
    if (_array_string.empty())
    {
        return std::vector<std::string>();

    }
    else
    {
        std::vector<std::string> result;
        boost::split(
                result,
                _array_string,
                boost::is_any_of(","));

        return result;
    }
}


std::vector<std::string> DomainBrowser::get_object_states(
        LibFred::OperationContext& ctx,
        unsigned long long object_id)
{
    Database::Result state_res = ctx.get_conn().exec_params(
            // clang format-off
            "SELECT ARRAY_TO_STRING(ARRAY_AGG(eos.name ORDER BY eos.importance)::text[],',') AS state_codes "
            " FROM object_state os "
            " JOIN enum_object_states eos ON eos.id = os.state_id "
            " WHERE os.object_id = $1::bigint "
            " AND os.valid_from <= CURRENT_TIMESTAMP "
            " AND (os.valid_to IS NULL OR os.valid_to > CURRENT_TIMESTAMP) ",
            // clang format-on
            Database::query_param_list(object_id));

    return split_object_states_string(static_cast<std::string>(state_res[0]["state_codes"]));
}


std::string DomainBrowser::filter_authinfo(
        bool user_is_owner,
        const std::string& authinfopw)
{
    if (user_is_owner)
    {
        return authinfopw;
    }

    return "********";         // if not
}


Nullable<NextDomainState> DomainBrowser::getNextDomainState(
        const boost::gregorian::date& today_date,
        const boost::gregorian::date& expiration_date,
        const boost::gregorian::date& outzone_date,
        const boost::gregorian::date& delete_date)
{
    Nullable<NextDomainState> next;
    if (today_date < expiration_date)
    {
        next = Nullable<NextDomainState>(
                NextDomainState(
                        "expired",
                        expiration_date));
    }
    else if ((today_date < delete_date) || (today_date < outzone_date))
    {
        if (outzone_date < delete_date)
        {
            if (today_date < outzone_date)
            {
                next = Nullable<NextDomainState>(
                        NextDomainState(
                                "outzone",
                                outzone_date));
            }
            else
            {
                next = Nullable<NextDomainState>(
                        NextDomainState(
                                "deleteCandidate",
                                delete_date));
            }
        }
        else         // posibly bad config
        {
            if (today_date < delete_date)
            {
                next = Nullable<NextDomainState>(
                        NextDomainState(
                                "deleteCandidate",
                                delete_date));
            }
            else
            {
                next = Nullable<NextDomainState>(
                        NextDomainState(
                                "outzone",
                                outzone_date));
            }
        }
    }
    return next;
}


DomainBrowser::DomainBrowser(
        const std::string& server_name,
        const std::string& update_registrar_handle,
        unsigned int domain_list_limit,
        unsigned int nsset_list_limit,
        unsigned int keyset_list_limit,
        unsigned int contact_list_limit)
    : server_name_(server_name),
      update_registrar_(update_registrar_handle),
      domain_list_limit_(domain_list_limit),
      nsset_list_limit_(nsset_list_limit),
      keyset_list_limit_(keyset_list_limit),
      contact_list_limit_(contact_list_limit)
{
    Logging::Context lctx_server(server_name_);
    Logging::Context lctx("init");
    LibFred::OperationContextCreator ctx;
    Database::Result db_config = ctx.get_conn().exec(
            "SELECT MAX(importance) * 2 AS minimal_status_importance FROM enum_object_states");
    lowest_status_importance_ = static_cast<unsigned int>(db_config[0]["minimal_status_importance"]);
}


DomainBrowser::~DomainBrowser()
{
}


// exception with dummy set handle
struct ObjectNotExistsWithDummyHandleSetter
    : ObjectNotExists
{
    ObjectNotExistsWithDummyHandleSetter& set_handle(const std::string&)
    {
        return *this;
    }

};

unsigned long long DomainBrowser::getContactId(const std::string& handle)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-object-registry-id");
    LibFred::OperationContextCreator ctx;
    try
    {
        return LibFred::get_object_id_by_handle_and_type_with_lock(
                ctx,
                true,
                handle,
                "contact",
                static_cast<ObjectNotExistsWithDummyHandleSetter*>(NULL),
                &ObjectNotExistsWithDummyHandleSetter::set_handle);
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return 0;
}


RegistrarDetail DomainBrowser::getRegistrarDetail(
        unsigned long long user_contact_id,
        const std::string& registrar_handle)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-registrar-detail");
    LibFred::OperationContextCreator ctx;
    try
    {
        check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone);

        LibFred::InfoRegistrarOutput registar_info;
        try
        {
            registar_info = LibFred::InfoRegistrarByHandle(registrar_handle).exec(
                    ctx,
                    output_timezone);
        }
        catch (const LibFred::InfoRegistrarByHandle::Exception& ex)
        {
            if (ex.is_set_unknown_registrar_handle())
            {
                throw ObjectNotExists();
            }
            else
            {
                throw;
            }
        }
        RegistrarDetail result;
        result.id = registar_info.info_registrar_data.id;
        result.handle = registar_info.info_registrar_data.handle;
        result.name = registar_info.info_registrar_data.name.get_value_or_default();
        result.phone = registar_info.info_registrar_data.telephone.get_value_or_default();
        result.fax = registar_info.info_registrar_data.fax.get_value_or_default();
        result.url = registar_info.info_registrar_data.url.get_value_or_default();

        Util::HeadSeparator addr_separator("", ", ");

        if (!registar_info.info_registrar_data.street1.isnull())
        {
            result.address += addr_separator.get();
            result.address += registar_info.info_registrar_data.street1.get_value();
        }

        if (!registar_info.info_registrar_data.street2.isnull())
        {
            result.address += addr_separator.get();
            result.address += registar_info.info_registrar_data.street2.get_value();
        }

        if (!registar_info.info_registrar_data.street3.isnull())
        {
            result.address += addr_separator.get();
            result.address += registar_info.info_registrar_data.street3.get_value();
        }

        if (!registar_info.info_registrar_data.city.isnull())
        {
            result.address += addr_separator.get();
            if (!registar_info.info_registrar_data.postalcode.isnull())
            {
                result.address += registar_info.info_registrar_data.postalcode.get_value();
                result.address += " ";
            }
            result.address += registar_info.info_registrar_data.city.get_value();
        }

        if (!registar_info.info_registrar_data.stateorprovince.isnull())
        {
            result.address += addr_separator.get();
            result.address += registar_info.info_registrar_data.stateorprovince.get_value();
        }

        return result;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return RegistrarDetail();
}


std::string DomainBrowser::get_server_name()
{
    return server_name_;
}


ContactDetail DomainBrowser::getContactDetail(
        unsigned long long user_contact_id,
        unsigned long long contact_id)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-contact-detail");
    LibFred::OperationContextCreator ctx;
    try
    {
        check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone);

        LibFred::InfoContactOutput contact_info;
        try
        {
            contact_info = LibFred::InfoContactById(contact_id).exec(
                    ctx,
                    output_timezone);
        }
        catch (const LibFred::InfoContactById::Exception& ex)
        {
            if (ex.is_set_unknown_object_id())
            {
                throw ObjectNotExists();
            }
            else
            {
                throw;
            }
        }

        LibFred::InfoRegistrarOutput sponsoring_registar_info = LibFred::InfoRegistrarByHandle(
                contact_info.info_contact_data.sponsoring_registrar_handle).exec(
                ctx,
                output_timezone);

        RegistryReference sponsoring_registrar;
        sponsoring_registrar.id = sponsoring_registar_info.info_registrar_data.id;
        sponsoring_registrar.handle = sponsoring_registar_info.info_registrar_data.handle;
        sponsoring_registrar.name = sponsoring_registar_info.info_registrar_data.name.get_value_or_default();

        ContactDiscloseFlags disclose_flags;
        disclose_flags.name = contact_info.info_contact_data.disclosename;
        disclose_flags.organization = contact_info.info_contact_data.discloseorganization;
        disclose_flags.email = contact_info.info_contact_data.discloseemail;
        disclose_flags.address = contact_info.info_contact_data.discloseaddress;
        disclose_flags.telephone = contact_info.info_contact_data.disclosetelephone;
        disclose_flags.fax = contact_info.info_contact_data.disclosefax;
        disclose_flags.ident = contact_info.info_contact_data.discloseident;
        disclose_flags.vat = contact_info.info_contact_data.disclosevat;
        disclose_flags.notify_email = contact_info.info_contact_data.disclosenotifyemail;

        ContactDetail detail;
        detail.id = contact_info.info_contact_data.id;
        detail.handle = contact_info.info_contact_data.handle;
        detail.roid = contact_info.info_contact_data.roid;
        detail.sponsoring_registrar = sponsoring_registrar;

        detail.creation_time = contact_info.info_contact_data.creation_time;
        detail.update_time = contact_info.info_contact_data.update_time;
        detail.transfer_time = contact_info.info_contact_data.transfer_time;

        detail.is_owner = (user_contact_id == contact_id);
        detail.authinfopw = filter_authinfo(
                detail.is_owner,
                contact_info.info_contact_data.authinfopw);

        detail.name = contact_info.info_contact_data.name;
        detail.organization = contact_info.info_contact_data.organization;
        detail.permanent_address = contact_info.info_contact_data.place.get_value_or_default();
        detail.mailing_address = optional_map_at<Nullable>(
                contact_info.info_contact_data.addresses,
                LibFred::ContactAddressType::MAILING);
        detail.telephone = contact_info.info_contact_data.telephone;
        detail.fax = contact_info.info_contact_data.fax;
        detail.email = contact_info.info_contact_data.email;
        detail.notifyemail = contact_info.info_contact_data.notifyemail;
        detail.vat = contact_info.info_contact_data.vat;
        detail.ssntype = contact_info.info_contact_data.ssntype;
        detail.ssn = contact_info.info_contact_data.ssn;
        detail.disclose_flags = disclose_flags;

        // get states
        detail.state_codes = get_object_states(
                ctx,
                contact_info.info_contact_data.id);

        detail.warning_letter = contact_info.info_contact_data.warning_letter;

        return detail;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return ContactDetail();
}


DomainDetail DomainBrowser::getDomainDetail(
        unsigned long long user_contact_id,
        unsigned long long domain_id)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-domain-detail");
    LibFred::OperationContextCreator ctx;
    try
    {
        check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone);

        LibFred::InfoDomainOutput domain_info;
        try
        {
            domain_info = LibFred::InfoDomainById(domain_id).exec(
                    ctx,
                    output_timezone);
        }
        catch (const LibFred::InfoDomainById::Exception& ex)
        {
            if (ex.is_set_unknown_object_id())
            {
                throw ObjectNotExists();
            }
            else
            {
                throw;
            }
        }

        LibFred::InfoRegistrarOutput sponsoring_registar_info = LibFred::InfoRegistrarByHandle(
                domain_info.info_domain_data.sponsoring_registrar_handle).exec(
                ctx,
                output_timezone);

        RegistryReference sponsoring_registrar;
        sponsoring_registrar.id = sponsoring_registar_info.info_registrar_data.id;
        sponsoring_registrar.handle = sponsoring_registar_info.info_registrar_data.handle;
        sponsoring_registrar.name = sponsoring_registar_info.info_registrar_data.name.get_value_or_default();

        LibFred::InfoContactOutput registrant_contact_info = LibFred::InfoContactById(
                domain_info.info_domain_data.registrant.id).exec(
                ctx,
                output_timezone);

        RegistryReference registrant;
        registrant.id = registrant_contact_info.info_contact_data.id;
        registrant.handle = registrant_contact_info.info_contact_data.handle;
        registrant.name =
                registrant_contact_info.info_contact_data.organization.get_value_or_default().empty()
                        ? registrant_contact_info.info_contact_data.name.get_value_or_default()
                        : registrant_contact_info.info_contact_data.organization.get_value();

        RegistryReference nsset;
        nsset.id = domain_info.info_domain_data.nsset.get_value_or_default().id;
        nsset.handle = domain_info.info_domain_data.nsset.get_value_or_default().handle;

        RegistryReference keyset;
        keyset.id = domain_info.info_domain_data.keyset.get_value_or_default().id;
        keyset.handle = domain_info.info_domain_data.keyset.get_value_or_default().handle;

        DomainDetail detail;
        detail.id = domain_info.info_domain_data.id;
        detail.fqdn = domain_info.info_domain_data.fqdn;
        detail.roid = domain_info.info_domain_data.roid;
        detail.sponsoring_registrar = sponsoring_registrar;
        detail.creation_time = domain_info.info_domain_data.creation_time;
        detail.update_time = domain_info.info_domain_data.update_time;

        detail.is_owner = (user_contact_id == registrant_contact_info.info_contact_data.id);

        detail.registrant = registrant;
        detail.expiration_date = domain_info.info_domain_data.expiration_date;
        detail.enum_domain_validation = domain_info.info_domain_data.enum_domain_validation;
        detail.nsset = nsset;
        detail.keyset = keyset;

        bool set_authinfo = detail.is_owner;
        detail.admins.reserve(domain_info.info_domain_data.admin_contacts.size());
        for (const auto& admin_contact : domain_info.info_domain_data.admin_contacts)
        {
            LibFred::InfoContactOutput admin_contact_info = LibFred::InfoContactById(admin_contact.id).exec(
                    ctx,
                    output_timezone);

            RegistryReference admin;
            admin.id = admin_contact_info.info_contact_data.id;
            admin.handle = admin_contact_info.info_contact_data.handle;
            admin.name = admin_contact_info.info_contact_data.organization.get_value_or_default().empty()
                         ? admin_contact_info.info_contact_data.name.get_value_or_default()
                         : admin_contact_info.info_contact_data.organization.get_value();

            detail.admins.push_back(admin);

            if (admin.id == user_contact_id)
            {
                set_authinfo = true;        // reveal authinfo to admin
                detail.is_admin = true;
            }
        }

        detail.authinfopw = filter_authinfo(
                set_authinfo,
                domain_info.info_domain_data.authinfopw);

        detail.state_codes = get_object_states(
                ctx,
                domain_info.info_domain_data.id);

        return detail;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainDetail();
}


NssetDetail DomainBrowser::getNssetDetail(
        unsigned long long user_contact_id,
        unsigned long long nsset_id)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-nsset-detail");
    LibFred::OperationContextCreator ctx;
    try
    {
        check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone);

        LibFred::InfoNssetOutput nsset_info;
        try
        {
            nsset_info = LibFred::InfoNssetById(nsset_id).exec(
                    ctx,
                    output_timezone);
        }
        catch (const LibFred::InfoNssetById::Exception& ex)
        {
            if (ex.is_set_unknown_object_id())
            {
                throw ObjectNotExists();
            }
            else
            {
                throw;
            }
        }

        LibFred::InfoRegistrarOutput sponsoring_registar_info = LibFred::InfoRegistrarByHandle(
                nsset_info.info_nsset_data.sponsoring_registrar_handle).exec(
                ctx,
                output_timezone);
        RegistryReference sponsoring_registrar;
        sponsoring_registrar.id = sponsoring_registar_info.info_registrar_data.id;
        sponsoring_registrar.handle = sponsoring_registar_info.info_registrar_data.handle;
        sponsoring_registrar.name = sponsoring_registar_info.info_registrar_data.name.get_value_or_default();

        LibFred::InfoRegistrarOutput create_registar_info = LibFred::InfoRegistrarByHandle(
                nsset_info.info_nsset_data.create_registrar_handle).exec(
                ctx,
                output_timezone);
        RegistryReference create_registrar;
        create_registrar.id = create_registar_info.info_registrar_data.id;
        create_registrar.handle = create_registar_info.info_registrar_data.handle;
        create_registrar.name = create_registar_info.info_registrar_data.name.get_value_or_default();

        RegistryReference update_registrar;
        if (!nsset_info.info_nsset_data.update_registrar_handle.isnull())
        {
            LibFred::InfoRegistrarOutput update_registar_info = LibFred::InfoRegistrarByHandle(
                    nsset_info.info_nsset_data.update_registrar_handle.get_value()).exec(
                    ctx,
                    output_timezone);
            update_registrar.id = update_registar_info.info_registrar_data.id;
            update_registrar.handle = update_registar_info.info_registrar_data.handle;
            update_registrar.name = update_registar_info.info_registrar_data.name.get_value_or_default();
        }

        NssetDetail detail;
        detail.id = nsset_info.info_nsset_data.id;
        detail.handle = nsset_info.info_nsset_data.handle;
        detail.roid = nsset_info.info_nsset_data.roid;
        detail.sponsoring_registrar = sponsoring_registrar;
        detail.creation_time = nsset_info.info_nsset_data.creation_time;
        detail.transfer_time = nsset_info.info_nsset_data.transfer_time;
        detail.update_time = nsset_info.info_nsset_data.update_time;

        detail.create_registrar = create_registrar;
        detail.update_registrar = update_registrar;

        detail.is_owner = false;

        detail.admins.reserve(nsset_info.info_nsset_data.tech_contacts.size());
        for (const auto& tech_contact : nsset_info.info_nsset_data.tech_contacts)
        {
            LibFred::InfoContactOutput tech_contact_info = LibFred::InfoContactById(tech_contact.id).exec(
                    ctx,
                    output_timezone);

            RegistryReference admin;
            admin.id = tech_contact_info.info_contact_data.id;
            admin.handle = tech_contact_info.info_contact_data.handle;
            admin.name = tech_contact_info.info_contact_data.organization.get_value_or_default().empty()
                         ? tech_contact_info.info_contact_data.name.get_value_or_default()
                         : tech_contact_info.info_contact_data.organization.get_value();
            detail.admins.push_back(admin);

            if (admin.id == user_contact_id)
            {
                detail.is_owner = true;                                    // reveal authinfo
            }
        }
        detail.authinfopw = filter_authinfo(
                detail.is_owner,
                nsset_info.info_nsset_data.authinfopw);
        detail.hosts = nsset_info.info_nsset_data.dns_hosts;

        detail.state_codes = get_object_states(
                ctx,
                nsset_info.info_nsset_data.id);

        detail.report_level = nsset_info.info_nsset_data.tech_check_level.get_value_or_default();

        return detail;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return NssetDetail();
}


KeysetDetail DomainBrowser::getKeysetDetail(
        unsigned long long user_contact_id,
        unsigned long long keyset_id)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-keyset-detail");
    LibFred::OperationContextCreator ctx;

    try
    {
        check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone);

        LibFred::InfoKeysetOutput keyset_info;
        try
        {
            keyset_info = LibFred::InfoKeysetById(keyset_id).exec(
                    ctx,
                    output_timezone);
        }
        catch (const LibFred::InfoKeysetById::Exception& ex)
        {
            if (ex.is_set_unknown_object_id())
            {
                throw ObjectNotExists();
            }
            else
            {
                throw;
            }
        }

        LibFred::InfoRegistrarOutput sponsoring_registar_info = LibFred::InfoRegistrarByHandle(
                keyset_info.info_keyset_data.sponsoring_registrar_handle).exec(
                ctx,
                output_timezone);
        RegistryReference sponsoring_registrar;
        sponsoring_registrar.id = sponsoring_registar_info.info_registrar_data.id;
        sponsoring_registrar.handle = sponsoring_registar_info.info_registrar_data.handle;
        sponsoring_registrar.name = sponsoring_registar_info.info_registrar_data.name.get_value_or_default();

        LibFred::InfoRegistrarOutput create_registar_info = LibFred::InfoRegistrarByHandle(
                keyset_info.info_keyset_data.create_registrar_handle).exec(
                ctx,
                output_timezone);
        RegistryReference create_registrar;
        create_registrar.id = create_registar_info.info_registrar_data.id;
        create_registrar.handle = create_registar_info.info_registrar_data.handle;
        create_registrar.name = create_registar_info.info_registrar_data.name.get_value_or_default();

        RegistryReference update_registrar;
        if (!keyset_info.info_keyset_data.update_registrar_handle.isnull())
        {
            LibFred::InfoRegistrarOutput update_registar_info = LibFred::InfoRegistrarByHandle(
                    keyset_info.info_keyset_data.update_registrar_handle.get_value()).exec(
                    ctx,
                    output_timezone);
            update_registrar.id = update_registar_info.info_registrar_data.id;
            update_registrar.handle = update_registar_info.info_registrar_data.handle;
            update_registrar.name = update_registar_info.info_registrar_data.name.get_value_or_default();
        }

        KeysetDetail detail;
        detail.id = keyset_info.info_keyset_data.id;
        detail.handle = keyset_info.info_keyset_data.handle;
        detail.roid = keyset_info.info_keyset_data.roid;
        detail.sponsoring_registrar = sponsoring_registrar;
        detail.creation_time = keyset_info.info_keyset_data.creation_time;
        detail.transfer_time = keyset_info.info_keyset_data.transfer_time;
        detail.update_time = keyset_info.info_keyset_data.update_time;

        detail.create_registrar = create_registrar;
        detail.update_registrar = update_registrar;

        detail.is_owner = false;

        detail.admins.reserve(keyset_info.info_keyset_data.tech_contacts.size());
        for (const auto& tech_contact : keyset_info.info_keyset_data.tech_contacts)
        {
            LibFred::InfoContactOutput tech_contact_info = LibFred::InfoContactById(tech_contact.id).exec(
                    ctx,
                    output_timezone);

            RegistryReference admin;
            admin.id = tech_contact_info.info_contact_data.id;
            admin.handle = tech_contact_info.info_contact_data.handle;
            admin.name = tech_contact_info.info_contact_data.organization.get_value_or_default().empty()
                         ? tech_contact_info.info_contact_data.name.get_value_or_default()
                         : tech_contact_info.info_contact_data.organization.get_value();
            detail.admins.push_back(admin);

            if (admin.id == user_contact_id)
            {
                detail.is_owner = true;                                    // reveal authinfo
            }
        }

        detail.authinfopw = filter_authinfo(
                detail.is_owner,
                keyset_info.info_keyset_data.authinfopw);

        detail.dnskeys.reserve(keyset_info.info_keyset_data.dns_keys.size());
        for (std::vector<LibFred::DnsKey>::const_iterator ci = keyset_info.info_keyset_data.dns_keys.begin();
             ci != keyset_info.info_keyset_data.dns_keys.end(); ++ci)
        {
            DNSKey dnskey;
            dnskey.flags = ci->get_flags();
            dnskey.protocol = ci->get_protocol();
            dnskey.alg = ci->get_alg();
            dnskey.key = ci->get_key();

            detail.dnskeys.push_back(dnskey);
        }

        detail.state_codes = get_object_states(
                ctx,
                keyset_info.info_keyset_data.id);

        return detail;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return KeysetDetail();
}


bool DomainBrowser::setContactDiscloseFlags(
        unsigned long long user_contact_id,
        const ContactDiscloseFlagsToSet& flags,
        unsigned long long request_id)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("set-contact-disclose-flags");
    LibFred::OperationContextCreator ctx;
    try
    {
        LibFred::InfoContactOutput contact_info = check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone,
                true);

        if (!is_at_least_identified(ctx, user_contact_id))
        {
            throw AccessDenied();
        }

        if (LibFred::ObjectHasState(
                    user_contact_id,
                    LibFred::Object_State::server_blocked).exec(ctx))
        {
            throw ObjectBlocked();
        }

        // when organization is set it's not allowed to hide address
        if ((!contact_info.info_contact_data.organization.get_value_or_default().empty()) &&
            (flags.address == false))
        {
            throw IncorrectUsage();
        }

        LibFred::UpdateContactById update_contact(user_contact_id, update_registrar_);
        bool exec_update = false;
        if (flags.email != contact_info.info_contact_data.discloseemail)
        {
            update_contact.set_discloseemail(flags.email);
            exec_update = true;
        }

        if (flags.address != contact_info.info_contact_data.discloseaddress)
        {
            update_contact.set_discloseaddress(flags.address);
            exec_update = true;
        }

        if (flags.telephone != contact_info.info_contact_data.disclosetelephone)
        {
            update_contact.set_disclosetelephone(flags.telephone);
            exec_update = true;
        }

        if (flags.fax != contact_info.info_contact_data.disclosefax)
        {
            update_contact.set_disclosefax(flags.fax);
            exec_update = true;
        }

        if (flags.ident != contact_info.info_contact_data.discloseident)
        {
            update_contact.set_discloseident(flags.ident);
            exec_update = true;
        }

        if (flags.vat != contact_info.info_contact_data.disclosevat)
        {
            update_contact.set_disclosevat(flags.vat);
            exec_update = true;
        }

        if (flags.notify_email != contact_info.info_contact_data.disclosenotifyemail)
        {
            update_contact.set_disclosenotifyemail(flags.notify_email);
            exec_update = true;
        }

        if (exec_update)
        {
            update_contact.set_logd_request_id(request_id).exec(ctx);
            ctx.commit_transaction();
        }
        else
        {
            return false;
        }
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return true;
}


bool DomainBrowser::setContactAuthInfo(
        unsigned long long user_contact_id,
        const std::string& authinfo,
        unsigned long long request_id)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("set-contact-auth-info");
    LibFred::OperationContextCreator ctx;
    try
    {
        LibFred::InfoContactOutput contact_info = check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone,
                true);

        unsigned long long contact_id = contact_info.info_contact_data.id;

        const unsigned MAX_AUTH_INFO_LENGTH = 300u;
        if (authinfo.length() > MAX_AUTH_INFO_LENGTH)
        {
            throw IncorrectUsage();
        }

        if (!is_at_least_identified(ctx, contact_id))
        {
            throw AccessDenied();
        }

        if (LibFred::ObjectHasState(
                    contact_id,
                    LibFred::Object_State::server_blocked).exec(ctx))
        {
            throw ObjectBlocked();
        }

        if (contact_info.info_contact_data.authinfopw == authinfo)
        {
            return false;
        }

        LibFred::UpdateContactById(
                contact_id,
                update_registrar_).set_authinfo(authinfo).set_logd_request_id(request_id).exec(ctx);
        ctx.commit_transaction();
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return true;
}


bool DomainBrowser::setObjectBlockStatus(
        unsigned long long user_contact_id,
        const std::string& objtype,
        const std::vector<unsigned long long>& object_id,
        unsigned block_type,
        std::vector<std::string>& blocked_objects)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("set-object-block-status");
    LibFred::OperationContextCreator ctx;
    try
    {
        LibFred::InfoContactOutput contact_info = check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone);

        if (!is_at_least_validated(ctx, user_contact_id))
        {
            throw AccessDenied();
        }

        unsigned long long object_type_id = 0;
        try         // check objtype exists and get id
        {
            object_type_id = LibFred::get_object_type_id(
                    ctx,
                    objtype);
        }
        catch (const LibFred::InternalError&)
        {
            throw IncorrectUsage();
        }

        // check input size
        if (object_id.size() == 0)
        {
            return false;                              // nothing to do
        }
        const unsigned SET_STATUS_MAX_ITEMS = 500u;
        if (object_id.size() > SET_STATUS_MAX_ITEMS)
        {
            throw IncorrectUsage();                                                // input too big

        }
        // object ids made unique
        std::set<unsigned long long> object_id_set(object_id.begin(), object_id.end());

        // checked object id-handle pairs
        std::set<std::pair<unsigned long long, std::string> > object_id_name_pairs;

        // check contact object type
        if (objtype == "contact")
        {
            throw IncorrectUsage();        // contact is not valid object type in this use case
        }
        else
        {        // check ownership for other object types
            Database::QueryParams params;
            std::ostringstream object_sql;
            object_sql << "SELECT oreg.id, oreg.name FROM object_registry oreg ";

            params.push_back(object_type_id);        // $1
            params.push_back(user_contact_id);        // $2
            if (objtype == "nsset")       // user have to be tech contact
            {
                object_sql << " JOIN nsset_contact_map map ON map.nssetid = oreg.id AND map.contactid = $"
                           << params.size() << "::bigint ";
            }
            else if (objtype == "domain")       // user have to be admin contact or registrant
            {
                object_sql << " LEFT JOIN domain d ON oreg.id = d.id AND d.registrant = $" << params.size() <<
                            "::bigint "
                    " LEFT JOIN domain_contact_map map ON map.domainid = oreg.id AND map.contactid = $" <<
                            params.size() << "::bigint ";
            }
            else if (objtype == "keyset")       // user have to be tech contact
            {
                object_sql << " JOIN keyset_contact_map map ON map.keysetid = oreg.id AND map.contactid = $"
                           << params.size() << "::bigint ";
            }
            else
            {
                throw InternalServerError();        // unknown object type, should'v been checked before
            }

            object_sql << " WHERE oreg.type = $1::integer AND oreg.erdate IS NULL ";

            if (objtype == "domain")       // user have to be admin contact or registrant
            {
                object_sql << " AND (d.id IS NOT NULL OR map.domainid IS NOT NULL) ";
            }

            object_sql << " AND (";

            Util::HeadSeparator or_separator("", " OR ");
            for (std::set<unsigned long long>::const_iterator ci = object_id_set.begin();
                 ci != object_id_set.end();
                 ++ci)
            {
                params.push_back(*ci);
                object_sql << or_separator.get() << "oreg.id = $" << params.size() << "::bigint";
            }
            object_sql << ") FOR SHARE OF oreg";        // lock to prevent change of linked contacts

            Database::Result object_result = ctx.get_conn().exec_params(
                    object_sql.str(),
                    params);
            if (object_id_set.size() != object_result.size())
            {
                throw ObjectNotExists();                                                     // given objects was not found all in database belonging to user contact

            }
            for (std::size_t i = 0; i < object_result.size(); ++i)
            {
                object_id_name_pairs.insert(
                        std::make_pair(
                                static_cast<unsigned long long>(object_result[i][0]),
                                static_cast<std::string>(object_result[i][1])));
            }
        }

        bool retval = false;
        for (std::set<std::pair<unsigned long long, std::string> >::const_iterator ci =
                 object_id_name_pairs.begin()
             ; ci != object_id_name_pairs.end(); ++ci)
        {
            LibFred::OperationContextCreator ctx_per_object;
            if (LibFred::ObjectHasState(
                        ci->first,
                        LibFred::Object_State::server_blocked).exec(ctx_per_object)) // object administratively blocked
            {
                blocked_objects.push_back(ci->second);
                continue;
            }

            if ((block_type == UNBLOCK_TRANSFER)
                && LibFred::ObjectHasState(
                        ci->first,
                        LibFred::Object_State::server_update_prohibited).exec(ctx_per_object)) // forbidden partial unblocking
            {
                blocked_objects.push_back(ci->second);
                continue;
            }

            if ((block_type == BLOCK_TRANSFER) || (block_type == BLOCK_TRANSFER_AND_UPDATE)) // block transfer
            {
                if (!LibFred::ObjectHasState(
                            ci->first,
                            LibFred::Object_State::server_transfer_prohibited).exec(ctx_per_object))
                {
                    LibFred::CreateObjectStateRequestId(
                            ci->first,
                            Util::set_of<std::string>(LibFred::ObjectState::SERVER_TRANSFER_PROHIBITED)).exec(
                            ctx_per_object);
                }
            }

            if (block_type == BLOCK_TRANSFER_AND_UPDATE) // block update
            {
                if (!LibFred::ObjectHasState(
                            ci->first,
                            LibFred::Object_State::server_update_prohibited).exec(ctx_per_object))
                {
                    LibFred::CreateObjectStateRequestId(
                            ci->first,
                            Util::set_of<std::string>(LibFred::ObjectState::SERVER_UPDATE_PROHIBITED)).exec(
                            ctx_per_object);
                }
            }

            if ((block_type == UNBLOCK_TRANSFER) || (block_type == UNBLOCK_TRANSFER_AND_UPDATE)) // unblock transfer
            {
                if (LibFred::ObjectHasState(
                            ci->first,
                            LibFred::Object_State::server_transfer_prohibited).exec(ctx_per_object))
                {
                    LibFred::CancelObjectStateRequestId(
                            ci->first,
                            Util::set_of<std::string>(LibFred::ObjectState::SERVER_TRANSFER_PROHIBITED)).exec(
                            ctx_per_object);
                }
            }

            if (block_type == UNBLOCK_TRANSFER_AND_UPDATE) // unblock transfer and update
            {
                if (LibFred::ObjectHasState(
                            ci->first,
                            LibFred::Object_State::server_update_prohibited).exec(ctx_per_object))
                {
                    LibFred::CancelObjectStateRequestId(
                            ci->first,
                            Util::set_of<std::string>(LibFred::ObjectState::SERVER_UPDATE_PROHIBITED)).exec(
                            ctx_per_object);
                }
            }

            if (block_type == INVALID_BLOCK_TYPE)
            {
                throw InternalServerError(); // bug in implementation
            }
            LibFred::PerformObjectStateRequest(ci->first).exec(ctx_per_object);
            ctx_per_object.commit_transaction();
            retval = true;        // ok at least one object have required state
        }

        ctx.commit_transaction();
        return retval;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return false;
}


DomainList DomainBrowser::getDomainList(
        unsigned long long user_contact_id,
        const Optional<unsigned long long>& list_domains_for_contact_id,
        const Optional<unsigned long long>& list_domains_for_nsset_id,
        const Optional<unsigned long long>& list_domains_for_keyset_id,
        unsigned long long offset)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-domain-list");
    LibFred::OperationContextCreator ctx;
    try
    {
        check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone);

        if (list_domains_for_contact_id.isset())
        {
            check_contact_id<ObjectNotExists>(
                    ctx,
                    list_domains_for_contact_id.get_value(),
                    output_timezone);
        }

        const unsigned long long contact_id =
                list_domains_for_contact_id.isset()
                        ? list_domains_for_contact_id.get_value()
                        : user_contact_id;

        Database::QueryParams params;
        params.push_back(contact_id);
        const int idx_of_contact_id = params.size();

        params.push_back(domain_list_limit_ + 1); // limit + 1 => exceeding detection
        const int idx_of_limit = params.size();

        params.push_back(offset);
        const int idx_of_offset = params.size();

        int idx_of_nsset_id = -1;
        if (list_domains_for_nsset_id.isset())
        {
            // check nsset owned by user contact
            Database::Result nsset_ownership_result = ctx.get_conn().exec_params(
                    "SELECT 1 "
                    "FROM nsset_contact_map "
                    "WHERE nssetid=$1::bigint AND contactid=$2::bigint"
                    ,
                    Database::query_param_list(list_domains_for_nsset_id.get_value())(contact_id));
            if (nsset_ownership_result.size() == 0)
            {
                throw AccessDenied();
            }
            params.push_back(list_domains_for_nsset_id.get_value());
            idx_of_nsset_id = params.size();
        }

        int idx_of_keyset_id = -1;
        if (list_domains_for_keyset_id.isset())
        {
            // check keyset owned by user contact
            Database::Result keyset_ownership_result = ctx.get_conn().exec_params(
                    "SELECT 1 "
                    "FROM keyset_contact_map "
                    "WHERE keysetid=$1::bigint AND contactid=$2::bigint"
                    ,
                    Database::query_param_list(list_domains_for_keyset_id.get_value())(contact_id));
            if (keyset_ownership_result.size() == 0)
            {
                throw AccessDenied();
            }
            params.push_back(list_domains_for_keyset_id.get_value());
            idx_of_keyset_id = params.size();
        }

        params.push_back(output_timezone);
        const int idx_timezone(params.size());

        std::ostringstream sql;
        sql <<
// clang-format off
"WITH domain_list AS ("
    "WITH domains AS (";
                if(!list_domains_for_nsset_id.isset() && !list_domains_for_keyset_id.isset())
                {   //select domains related to user_contact_id
                    sql <<
        "SELECT d.id,d.exdate,d.registrant,d.keyset "
        "FROM domain d "
        "LEFT JOIN domain_contact_map dcm ON dcm.domainid=d.id AND "
                                            "dcm.role=1 AND "
                                            "dcm.contactid=$" << idx_of_contact_id << "::BIGINT "
        "WHERE $" << idx_of_contact_id << "::BIGINT IN (dcm.contactid,d.registrant) "
        "ORDER BY d.exdate,d.id";
                }
                else
                {
                    sql <<
        "SELECT id,exdate,registrant,keyset "
        "FROM domain WHERE ";
                    if(list_domains_for_nsset_id.isset())
                    {   //select domains with given nsset
                        sql << "nsset=$" << idx_of_nsset_id << "::BIGINT";
                    }
                    if(list_domains_for_keyset_id.isset())
                    {   //select domains with given keyset
                        if(list_domains_for_nsset_id.isset())
                        {
                            sql << " AND ";
                        }
                        sql << "keyset=$" << idx_of_keyset_id << "::BIGINT";
                    }
                    sql << " "
        "ORDER BY exdate,id";
                }
                sql << " "
        "OFFSET $" << idx_of_offset << "::BIGINT "
        "LIMIT $" << idx_of_limit << "::BIGINT) "
"SELECT d.id,oreg.name AS fqdn,r.handle AS registrar_handle,"
       "r.name AS registrar_name,d.exdate AS expiration_date,"
       "d.registrant AS registrant_id,d.keyset IS NOT NULL AS have_keyset "
"FROM domains d "
"JOIN object_registry oreg ON oreg.id=d.id AND "
                             "oreg.type=(SELECT id FROM enum_object_type WHERE name='domain') AND "
                             "oreg.erdate IS NULL "
"JOIN object o ON o.id=d.id "
"JOIN registrar r ON r.id=o.clid) "
"SELECT dl.id,dl.fqdn,dl.registrar_handle,dl.registrar_name,dl.expiration_date AT TIME ZONE 'utc' AT TIME ZONE $"<< idx_timezone << "::text AS expiration_date,"
       "dl.registrant_id,dl.have_keyset,"
       "CASE WHEN dl.registrant_id=$" << idx_of_contact_id << "::BIGINT "
            "THEN 'holder' "
            "WHEN (SELECT role=1 FROM domain_contact_map "
                  "WHERE domainid=dl.id AND "
                        "contactid=$" << idx_of_contact_id << "::BIGINT) "
            "THEN 'admin' "
            "ELSE '' "
            "END AS user_role,"
       "CURRENT_DATE AS today_date,"
       "(dl.expiration_date AT TIME ZONE 'utc' AT TIME ZONE $"<< idx_timezone << "::text + dlp.expiration_dns_protection_period)::DATE AS outzone_date,"
       "(dl.expiration_date AT TIME ZONE 'utc' AT TIME ZONE $"<< idx_timezone << "::text + dlp.expiration_registration_protection_period)::DATE AS delete_date,"
       "COALESCE(BIT_OR(eos.external::INTEGER*eos.importance),0) AS external_importance,"
       "COALESCE(BOOL_OR(eos.name='serverBlocked'),false) AS is_server_blocked,"
       "ARRAY_TO_STRING(ARRAY_AGG((CASE WHEN eos.external THEN eos.name "
                                                         "ELSE NULL END) "
                                 "ORDER BY eos.importance)::text[],',') AS state_code "
"FROM domain_list dl "
"JOIN domain_lifecycle_parameters dlp ON dlp.valid_for_exdate_after=(SELECT MAX(valid_for_exdate_after) FROM domain_lifecycle_parameters WHERE valid_for_exdate_after<=dl.expiration_date) "
"LEFT JOIN object_state os ON os.object_id=dl.id AND "
                             "os.valid_from<=CURRENT_TIMESTAMP AND (CURRENT_TIMESTAMP<os.valid_to OR "
                                                                   "os.valid_to IS NULL) "
"LEFT JOIN enum_object_states eos ON eos.id=os.state_id "
"GROUP BY dl.expiration_date,dl.id,dl.fqdn,dl.registrar_handle,dl.registrar_name,"
         "dl.registrant_id,dl.have_keyset,user_role,dlp.id "
"ORDER BY dl.expiration_date,dl.id";
// clang-format on

            Database::Result domain_list_result = ctx.get_conn().exec_params(
                sql.str(),
                params);

        unsigned long long limited_domain_list_size = (domain_list_result.size() > domain_list_limit_)
                                                      ? domain_list_limit_ : domain_list_result.size();

        DomainList ret;
        ret.dld.reserve(limited_domain_list_size);
        for (unsigned long long i = 0; i < limited_domain_list_size; ++i)
        {
            DomainListData dld;
            dld.id = static_cast<unsigned long long>(domain_list_result[i]["id"]);
            dld.fqdn = static_cast<std::string>(domain_list_result[i]["fqdn"]);

            unsigned long long external_status_importance =
                static_cast<unsigned long long>(domain_list_result[i]["external_importance"]);
            dld.external_importance = external_status_importance ==
                                      0 ? lowest_status_importance_ : external_status_importance;

            boost::gregorian::date today_date =
                domain_list_result[i]["today_date"].isnull() ? boost::gregorian::date()
                                                             : boost::
                gregorian::from_string(static_cast<std::string>(domain_list_result[i]["today_date"]));
            boost::gregorian::date expiration_date =
                domain_list_result[i]["expiration_date"].isnull() ? boost::gregorian::date()
                                                                  : boost
                ::gregorian::from_string(
                        static_cast<std::string>(domain_list_result[i][
                                                     "expiration_date"]));
            boost::gregorian::date outzone_date =
                domain_list_result[i]["outzone_date"].isnull() ? boost::gregorian::date()
                                                               : boost::
                gregorian::from_string(static_cast<std::string>(domain_list_result[i]["outzone_date"]));
            boost::gregorian::date delete_date =
                domain_list_result[i]["delete_date"].isnull() ? boost::gregorian::date()
                                                              : boost::
                gregorian::from_string(static_cast<std::string>(domain_list_result[i]["delete_date"]));

            dld.next_state = getNextDomainState(
                    today_date,
                    expiration_date,
                    outzone_date,
                    delete_date);

            dld.have_keyset = static_cast<bool>(domain_list_result[i]["have_keyset"]);
            dld.user_role = static_cast<std::string>(domain_list_result[i]["user_role"]);
            dld.registrar_handle = static_cast<std::string>(domain_list_result[i]["registrar_handle"]);
            dld.registrar_name = static_cast<std::string>(domain_list_result[i]["registrar_name"]);
            dld.state_code =
                split_object_states_string(
                        static_cast<std::string>(domain_list_result[i]["state_code"
                                                 ]));
            dld.is_server_blocked = static_cast<bool>(domain_list_result[i]["is_server_blocked"]);

            ret.dld.push_back(dld);
        }

        ret.limit_exceeded = domain_list_limit_ < domain_list_result.size();
        return ret;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainList();
}


NssetList DomainBrowser::getNssetList(
        unsigned long long user_contact_id,
        const Optional<unsigned long long>& list_nssets_for_contact_id,
        unsigned long long offset)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-nsset-list");
    LibFred::OperationContextCreator ctx;
    try
    {
        check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone);

        if (list_nssets_for_contact_id.isset())
        {
            check_contact_id<ObjectNotExists>(
                    ctx,
                    list_nssets_for_contact_id.get_value(),
                    output_timezone);
        }

        unsigned long long contact_id = list_nssets_for_contact_id.isset()
                                        ? list_nssets_for_contact_id.get_value() : user_contact_id;

        Database::Result nsset_list_result = ctx.get_conn().exec_params(
                   // clang-format off
                    "SELECT nsset_list.id, nsset_list.handle, "
                        " nsset_list.registrar_handle, nsset_list.registrar_name "
                        " , nsset_list.domain_number, "
                    " COALESCE(BIT_OR(CASE WHEN eos.external THEN eos.importance ELSE NULL END), 0) AS external_importance, "
                    " SUM(CASE WHEN eos.name = 'serverBlocked' THEN 1 ELSE 0 END) AS is_server_blocked, "
                    " ARRAY_TO_STRING(ARRAY_AGG((CASE WHEN eos.external THEN eos.name ELSE NULL END) ORDER BY eos.importance)::text[],',') AS state_code "
                    "FROM "
                    "(SELECT oreg.id AS id "
                    ", oreg.name AS handle "
                    ", registrar.handle AS registrar_handle "
                    ", registrar.name AS registrar_name "
                    ", COALESCE(domains.number,0) AS domain_number "
                    " FROM object_registry oreg "
                    " JOIN object obj ON obj.id = oreg.id "
                    " JOIN registrar ON registrar.id = obj.clid "
                    " JOIN nsset_contact_map ncm ON ncm.nssetid = oreg.id "
                    " LEFT JOIN (SELECT d.nsset AS nsset, count(d.id) AS number FROM domain d "
                    " JOIN nsset_contact_map ncm ON  d.nsset = ncm.nssetid "
                    " WHERE ncm.contactid = $1::bigint "
                    " GROUP BY d.nsset) AS domains ON domains.nsset = oreg.id "
                    " WHERE ncm.contactid = $1::bigint "
                    " AND oreg.type = (SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) AND oreg.erdate IS NULL "
                    ") AS nsset_list "
                    " LEFT JOIN object_state os ON os.object_id = nsset_list.id "
                        " AND os.valid_from <= CURRENT_TIMESTAMP "
                        " AND (os.valid_to IS NULL OR os.valid_to > CURRENT_TIMESTAMP) "
                    " LEFT JOIN enum_object_states eos ON eos.id = os.state_id "
                    " GROUP BY nsset_list.id, nsset_list.handle, "
                        " nsset_list.registrar_handle, nsset_list.registrar_name "
                        " , nsset_list.domain_number "
                    " ORDER BY nsset_list.id "
                    " LIMIT $2::bigint OFFSET $3::bigint ",
                    // clang-format on
                Database::query_param_list(
                        contact_id)(nsset_list_limit_ + 1)(offset));

        unsigned long long limited_nsset_list_size = (nsset_list_result.size() > nsset_list_limit_)
                                                     ? nsset_list_limit_ : nsset_list_result.size();

        NssetList ret;
        ret.nld.reserve(limited_nsset_list_size);
        for (unsigned long long i = 0; i < limited_nsset_list_size; ++i)
        {
            NssetListData nld;
            nld.id = static_cast<unsigned long long>(nsset_list_result[i]["id"]);
            nld.handle = static_cast<std::string>(nsset_list_result[i]["handle"]);
            nld.domain_count = static_cast<unsigned long long>(nsset_list_result[i]["domain_number"]);
            nld.registrar_handle = static_cast<std::string>(nsset_list_result[i]["registrar_handle"]);
            nld.registrar_name = static_cast<std::string>(nsset_list_result[i]["registrar_name"]);
            unsigned long long external_status_importance =
                static_cast<unsigned long long>(nsset_list_result[i]["external_importance"]);
            nld.external_importance = external_status_importance ==
                                      0 ? lowest_status_importance_ : external_status_importance;
            nld.state_code =
                split_object_states_string(static_cast<std::string>(nsset_list_result[i]["state_code"]));
            nld.is_server_blocked = static_cast<bool>(nsset_list_result[i]["is_server_blocked"]);
            ret.nld.push_back(nld);
        }

        ret.limit_exceeded = nsset_list_result.size() > nsset_list_limit_;
        return ret;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return NssetList();
}


KeysetList DomainBrowser::getKeysetList(
        unsigned long long user_contact_id,
        const Optional<unsigned long long>& list_keysets_for_contact_id,
        unsigned long long offset)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-keyset-list");
    LibFred::OperationContextCreator ctx;
    try
    {
        check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone);

        if (list_keysets_for_contact_id.isset())
        {
            check_contact_id<ObjectNotExists>(
                    ctx,
                    list_keysets_for_contact_id.get_value(),
                    output_timezone);
        }

        unsigned long long contact_id = list_keysets_for_contact_id.isset()
                                        ? list_keysets_for_contact_id.get_value() : user_contact_id;

        Database::Result keyset_list_result = ctx.get_conn().exec_params(
// clang-format off
"SELECT keyset_list.id,keyset_list.handle,keyset_list.registrar_handle,keyset_list.registrar_name,"
       "keyset_list.domain_number,"
       "COALESCE(BIT_OR(CASE WHEN eos.external THEN eos.importance ELSE 0 END),0) AS external_importance,"
       "COALESCE(BOOL_OR(eos.name='serverBlocked'),false) AS is_server_blocked,"
       "ARRAY_TO_STRING(ARRAY_AGG((CASE WHEN eos.external THEN eos.name ELSE NULL END) ORDER BY eos.importance)::text[],',') AS state_code "
"FROM (WITH keyset AS ("
          "SELECT keysetid AS id "
          "FROM keyset_contact_map "
          "WHERE contactid=$1::BIGINT " // $1=contact_id
          "ORDER BY 1 "
          "OFFSET $2::BIGINT LIMIT $3::BIGINT) "
      "SELECT oreg.id AS id,oreg.name AS handle,registrar.handle AS registrar_handle,"
             "registrar.name AS registrar_name,"
             "COALESCE(domains.number,0) AS domain_number "
      "FROM keyset k "
      "JOIN object_registry oreg ON oreg.id=k.id "
      "JOIN object obj ON obj.id=oreg.id "
      "JOIN registrar ON registrar.id=obj.clid "
      "LEFT JOIN (SELECT d.keyset,count(d.id) AS number "
                 "FROM keyset_contact_map kcm "
                 "JOIN domain d ON d.keyset=kcm.keysetid "
                 "WHERE kcm.contactid=$1::BIGINT " // $1=contact_id
                 "GROUP BY d.keyset) "
                "AS domains ON domains.keyset=oreg.id "
      "WHERE oreg.type=(SELECT id FROM enum_object_type eot WHERE eot.name='keyset') AND "
            "oreg.erdate IS NULL) AS keyset_list "
"LEFT JOIN object_state os ON os.object_id=keyset_list.id AND "
                             "os.valid_from<=CURRENT_TIMESTAMP AND (CURRENT_TIMESTAMP<os.valid_to OR "
                                                                   "os.valid_to IS NULL) "
"LEFT JOIN enum_object_states eos ON eos.id=os.state_id "
"GROUP BY keyset_list.id,keyset_list.handle,keyset_list.registrar_handle,keyset_list.registrar_name,"
         "keyset_list.domain_number "
"ORDER BY keyset_list.id",
// clang-format on
                Database::query_param_list(
                        contact_id)(offset)(keyset_list_limit_ + 1));                       // limit + 1 => exceeding detection

        const unsigned long long limited_keyset_list_size = keyset_list_limit_ < keyset_list_result.size()
                                                            ? keyset_list_limit_ : keyset_list_result.size();

        KeysetList ret;
        ret.kld.reserve(limited_keyset_list_size);
        for (unsigned long long i = 0; i < limited_keyset_list_size; ++i)
        {
            KeysetListData kld;
            kld.id = static_cast<unsigned long long>(keyset_list_result[i]["id"]);
            kld.handle = static_cast<std::string>(keyset_list_result[i]["handle"]);
            kld.domain_count = static_cast<unsigned long long>(keyset_list_result[i]["domain_number"]);
            kld.registrar_handle = static_cast<std::string>(keyset_list_result[i]["registrar_handle"]);
            kld.registrar_name = static_cast<std::string>(keyset_list_result[i]["registrar_name"]);
            unsigned long long external_status_importance =
                static_cast<unsigned long long>(keyset_list_result[i]["external_importance"]);
            kld.external_importance = external_status_importance ==
                                      0 ? lowest_status_importance_ : external_status_importance;
            kld.state_code =
                split_object_states_string(
                        static_cast<std::string>(keyset_list_result[i]["state_code"
                                                 ]));
            kld.is_server_blocked = static_cast<bool>(keyset_list_result[i]["is_server_blocked"]);
            ret.kld.push_back(kld);
        }

        ret.limit_exceeded = keyset_list_limit_ < keyset_list_result.size();
        return ret;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return KeysetList();
}


std::vector<StatusDesc> DomainBrowser::getPublicStatusDesc(const std::string& lang)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-public-status-desc");
    LibFred::OperationContextCreator ctx;
    try
    {
        Database::Result state_desc_res = ctx.get_conn().exec_params(
                "SELECT eos.name AS name, COALESCE(eosd.description, '') AS description "
                " FROM enum_object_states_desc eosd "
                " JOIN enum_object_states eos ON eos.id = eosd.state_id "
                " WHERE UPPER(eosd.lang) = UPPER($1::text) "
                " AND eos.external = TRUE "
                " ORDER BY eos.id ",
                Database::query_param_list(lang));

        std::vector<StatusDesc> ret;
        ret.reserve(state_desc_res.size());
        for (unsigned long long i = 0; i < state_desc_res.size(); ++i)
        {
            ret.push_back(
                    StatusDesc(
                            static_cast<std::string>(state_desc_res[i]["name"])
                            ,
                            static_cast<std::string>(state_desc_res[i]["description"])));
        }
        return ret;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return std::vector<StatusDesc>();
}


struct MergeContactDiffContacts
{
    bool operator()(
            LibFred::OperationContext& ctx,
            const std::string& src_contact_handle,
            const std::string& dst_contact_handle) const
    {
        if (boost::algorithm::to_upper_copy(src_contact_handle).compare(
                    boost::algorithm::to_upper_copy(dst_contact_handle)) == 0)
        {
            BOOST_THROW_EXCEPTION(
                    LibFred::MergeContact::Exception().set_identical_contacts_handle(dst_contact_handle));
        }

        Database::Result diff_result = ctx.get_conn().exec_params(
                // clang-format off
                "SELECT "//--c_src.name, oreg_src.name, o_src.clid, c_dst.name, oreg_dst.name , o_dst.clid,
                //the same
                " (trim(BOTH ' ' FROM COALESCE(c_src.name,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.name,''))) OR "
                " (trim(BOTH ' ' FROM COALESCE(c_src.organization,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.organization,''))) OR "
                " (trim(BOTH ' ' FROM COALESCE(c_src.street1,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.street1,''))) OR "
                " (trim(BOTH ' ' FROM COALESCE(c_src.street2,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.street2,''))) OR "
                " (trim(BOTH ' ' FROM COALESCE(c_src.street3,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.street3,''))) OR "
                " (trim(BOTH ' ' FROM COALESCE(c_src.city,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.city,''))) OR "
                " (trim(BOTH ' ' FROM COALESCE(c_src.postalcode,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.postalcode,''))) OR "
                " (trim(BOTH ' ' FROM COALESCE(c_src.stateorprovince,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.stateorprovince,''))) OR "
                " (trim(BOTH ' ' FROM COALESCE(c_src.country,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.country,''))) OR "
                " (trim(BOTH ' ' FROM COALESCE(c_src.email,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.email,''))) OR "
                //if dst filled then src the same or empty
                " (trim(BOTH ' ' FROM COALESCE(c_src.vat,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.vat,'')) AND trim(BOTH ' ' FROM COALESCE(c_src.vat,'')) != ''::text) OR "
                " (trim(BOTH ' ' FROM COALESCE(c_src.ssn,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.ssn,'')) AND trim(BOTH ' ' FROM COALESCE(c_src.ssn,'')) != ''::text) OR "
                " (COALESCE(c_src.ssntype,0) != COALESCE(c_dst.ssntype,0) AND COALESCE(c_src.ssntype,0) != 0) "
                "  AS differ, c_src.id AS src_contact_id, c_dst.id AS dst_contact_id"
                " FROM (object_registry oreg_src "
                " JOIN contact c_src ON c_src.id = oreg_src.id AND oreg_src.name = UPPER($1::text) AND oreg_src.erdate IS NULL) "
                " JOIN (object_registry oreg_dst "
                " JOIN contact c_dst ON c_dst.id = oreg_dst.id AND oreg_dst.name = UPPER($2::text) AND oreg_dst.erdate IS NULL"
                ") ON TRUE ",
                // clang-format on
                Database::query_param_list(
                        src_contact_handle)(dst_contact_handle));
        if (diff_result.size() != 1)
        {
            BOOST_THROW_EXCEPTION(
                    LibFred::MergeContact::Exception().set_unable_to_get_difference_of_contacts(
                            LibFred::MergeContact::InvalidContacts(
                                    src_contact_handle,
                                    dst_contact_handle)));
        }

        unsigned long long dst_contact_id = static_cast<unsigned long long>(diff_result[0]["dst_contact_id"]);

        if (LibFred::ObjectHasState(
                    dst_contact_id,
                    LibFred::Object_State::server_blocked).exec(ctx))
        {
            BOOST_THROW_EXCEPTION(
                    LibFred::MergeContact::Exception().set_dst_contact_invalid(src_contact_handle));
        }

        const auto src_contact_id = static_cast<unsigned long long>(diff_result[0]["src_contact_id"]);
        const auto state_flags = LibFred::ObjectStatesInfo{LibFred::GetObjectStates{src_contact_id}.exec(ctx)};

        if (state_flags.presents(LibFred::Object_State::mojeid_contact) ||
            state_flags.presents(LibFred::Object_State::server_blocked) ||
            state_flags.presents(LibFred::Object_State::server_delete_prohibited) ||
            is_attached_to_identity(ctx, src_contact_id))
        {
            BOOST_THROW_EXCEPTION(
                    LibFred::MergeContact::Exception().set_src_contact_invalid(src_contact_handle));
        }

        bool contact_differs = static_cast<bool>(diff_result[0]["differ"]);
        return contact_differs;
    }

};

MergeContactCandidateList DomainBrowser::getMergeContactCandidateList(
        unsigned long long user_contact_id,
        unsigned long long offset)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-merge-contact-candidate-list");
    LibFred::OperationContextCreator ctx;
    try
    {
        check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone);

        Database::Result candidate_list_result = ctx.get_conn().exec_params(
                // clang-format off
                    " SELECT oreg_src.id AS id, oreg_src.name AS handle"
                    " , (SELECT count(foo.c) FROM (SELECT id AS c FROM domain WHERE registrant = oreg_src.id UNION SELECT domainid AS c FROM domain_contact_map WHERE contactid = oreg_src.id) AS foo) AS domain_count "
                    " , (SELECT count(ncm.nssetid) FROM nsset_contact_map ncm WHERE ncm.contactid = oreg_src.id ) AS nsset_count "
                    " , (SELECT count(kcm.keysetid) FROM keyset_contact_map kcm WHERE kcm.contactid = oreg_src.id ) AS keyset_count "
                    " , r.handle AS registrar_handle, r.name AS registrar_name "
                    " FROM (object_registry oreg_src "
                    " JOIN contact c_src ON c_src.id = oreg_src.id AND oreg_src.erdate IS NULL "
                    " JOIN object o ON o.id = oreg_src.id JOIN registrar r ON r.id = o.clid) "
                    " JOIN (object_registry oreg_dst "
                    " JOIN contact c_dst ON c_dst.id = oreg_dst.id  AND oreg_dst.erdate IS NULL AND oreg_dst.id = $1::bigint "
                    " ) ON TRUE "
                    " LEFT JOIN object_state os_src ON os_src.object_id = c_src.id "
                    " AND os_src.state_id IN (SELECT eos.id FROM enum_object_states eos WHERE eos.name = 'mojeidContact'::text "
                    " OR eos.name = 'serverDeleteProhibited'::text OR eos.name = 'serverBlocked'::text) "//forbidden states of src contact
                    " AND os_src.valid_from <= CURRENT_TIMESTAMP AND (os_src.valid_to is null OR os_src.valid_to > CURRENT_TIMESTAMP)"
                    " WHERE "
                    " ( "
                    //the same
                    " (trim(BOTH ' ' FROM COALESCE(c_src.name,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.name,''))) OR "
                    " (trim(BOTH ' ' FROM COALESCE(c_src.organization,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.organization,''))) OR "
                    " (trim(BOTH ' ' FROM COALESCE(c_src.street1,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.street1,''))) OR "
                    " (trim(BOTH ' ' FROM COALESCE(c_src.street2,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.street2,''))) OR "
                    " (trim(BOTH ' ' FROM COALESCE(c_src.street3,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.street3,''))) OR "
                    " (trim(BOTH ' ' FROM COALESCE(c_src.city,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.city,''))) OR "
                    " (trim(BOTH ' ' FROM COALESCE(c_src.postalcode,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.postalcode,''))) OR "
                    " (trim(BOTH ' ' FROM COALESCE(c_src.stateorprovince,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.stateorprovince,''))) OR "
                    " (trim(BOTH ' ' FROM COALESCE(c_src.country,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.country,''))) OR "
                    " (trim(BOTH ' ' FROM COALESCE(c_src.email,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.email,''))) OR "
                    //if dst filled then src the same or empty
                    " (trim(BOTH ' ' FROM COALESCE(c_src.vat,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.vat,'')) AND trim(BOTH ' ' FROM COALESCE(c_src.vat,'')) != ''::text) OR "
                    " (trim(BOTH ' ' FROM COALESCE(c_src.ssn,'')) != trim(BOTH ' ' FROM COALESCE(c_dst.ssn,'')) AND trim(BOTH ' ' FROM COALESCE(c_src.ssn,'')) != ''::text) OR "
                    " (COALESCE(c_src.ssntype,0) != COALESCE(c_dst.ssntype,0) AND COALESCE(c_src.ssntype,0) != 0)) = false "
                    " AND oreg_src.name != oreg_dst.name AND os_src.id IS NULL "
                    " ORDER BY oreg_src.id "
                    " LIMIT $2::bigint OFFSET $3::bigint ",
                // clang-format on
                Database::query_param_list(
                        user_contact_id)(contact_list_limit_ + 1)(offset));

        unsigned long long limited_contact_list_size = (candidate_list_result.size() > contact_list_limit_)
                                                       ? contact_list_limit_ : candidate_list_result.size();
        MergeContactCandidateList ret;
        ret.mccl.reserve(limited_contact_list_size);
        for (unsigned long long i = 0; i < limited_contact_list_size; ++i)
        {
            MergeContactCandidateData cd;
            cd.id = static_cast<unsigned long long>(candidate_list_result[i]["id"]);
            cd.handle = static_cast<std::string>(candidate_list_result[i]["handle"]);
            cd.domain_count = static_cast<unsigned long long>(candidate_list_result[i]["domain_count"]);
            cd.nsset_count = static_cast<unsigned long long>(candidate_list_result[i]["nsset_count"]);
            cd.keyset_count = static_cast<unsigned long long>(candidate_list_result[i]["keyset_count"]);
            cd.registrar_handle = static_cast<std::string>(candidate_list_result[i]["registrar_handle"]);
            cd.registrar_name = static_cast<std::string>(candidate_list_result[i]["registrar_name"]);

            ret.mccl.push_back(cd);
        }
        ret.limit_exceeded = candidate_list_result.size() > contact_list_limit_;
        return ret;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return MergeContactCandidateList();
}


void DomainBrowser::mergeContacts(
        unsigned long long dst_contact_id,
        const std::vector<unsigned long long>& contact_list,
        unsigned long long request_id)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("get-merge-contact");
    LibFred::OperationContextCreator ctx;
    try
    {
        LibFred::InfoContactOutput dst = check_user_contact_id<UserNotExists>(
                ctx,
                dst_contact_id,
                output_timezone);
        if (contact_list.empty())
        {
            throw Fred::Backend::DomainBrowser::InvalidContacts();
        }

        // get src contact handle
        Database::query_param_list params;
        Util::HeadSeparator id_separator("$", ", $");
        std::string sql("SELECT name, id FROM object_registry WHERE id IN (");

        for (std::vector<unsigned long long>::const_iterator ci = contact_list.begin();
             ci < contact_list.end();
             ++ci)
        {
            sql += id_separator.get();
            sql += params.add(*ci);
            sql += "::bigint";
        }

        sql += ")";

        Database::Result src_handle_result = ctx.get_conn().exec_params(
                sql,
                params);

        for (Database::Result::size_type i = 0; i < src_handle_result.size(); ++i)
        {
            LibFred::MergeContactOutput merge_data;
            try
            {
                merge_data = LibFred::MergeContact(
                        src_handle_result[i]["name"],
                        dst.info_contact_data.handle,
                        update_registrar_,
                        MergeContactDiffContacts()).set_logd_request_id(request_id).exec(ctx);
            }
            catch (const LibFred::MergeContact::Exception& ex)
            {
                ctx.get_log().warning(
                        boost::algorithm::replace_all_copy(
                                boost::diagnostic_information(ex),
                                "\n",
                                " "));
                throw InvalidContacts();
            }

            if ((merge_data.contactid.src_contact_id !=
                 static_cast<unsigned long long>(src_handle_result[i]["id"]))
                || (merge_data.contactid.dst_contact_id != dst_contact_id))
            {
                ctx.get_log().error(
                        boost::format(
                                "id mismatch merge_data.contactid.src_contact_id: %1% != src_handle_result[i][\"id\"]: %2% "
                                "or merge_data.contactid.dst_contact_id: %3% != dst_contact_id: %4%")
                        % merge_data.contactid.src_contact_id
                        % static_cast<unsigned long long>(src_handle_result[i]["id"])
                        % merge_data.contactid.dst_contact_id
                        % dst_contact_id);
                throw InternalServerError();
            }

            LibFred::create_poll_messages(
                    merge_data,
                    ctx);
        }
    }
    catch (const InvalidContacts& ex)
    {
        ctx.get_log().warning(ex.what());
        throw;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    ctx.commit_transaction();
}


void DomainBrowser::setContactPreferenceForDomainExpirationLetters(
        unsigned long long user_contact_id,
        bool send_expiration_letters,
        unsigned long long request_id)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("set-contact-auth-info");
    LibFred::OperationContextCreator ctx;
    try
    {
        LibFred::InfoContactOutput contact_info = check_user_contact_id<UserNotExists>(
                ctx,
                user_contact_id,
                output_timezone,
                true);

        unsigned long long contact_id = contact_info.info_contact_data.id;

        if (!send_expiration_letters && !is_at_least_validated(ctx, contact_id))
        {
            throw AccessDenied();
        }

        if (LibFred::ObjectHasState(
                    contact_id,
                    LibFred::Object_State::server_blocked).exec(ctx))
        {
            throw ObjectBlocked();
        }

        LibFred::UpdateContactById(
                contact_id,
                update_registrar_)
        .set_domain_expiration_warning_letter_enabled(send_expiration_letters)
        .set_logd_request_id(request_id).exec(ctx);
        ctx.commit_transaction();
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
}

}//namespace Fred::Backend::DomainBrowser
}//namespace Fred::Backend
}//namespace Fred
