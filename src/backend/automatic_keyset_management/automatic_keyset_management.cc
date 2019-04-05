/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file automatic_keyset_management.cc
 *  automatic keyset management implementation
 */

#include "src/backend/automatic_keyset_management/automatic_keyset_management.hh"

#include "src/backend/automatic_keyset_management/impl/limits.hh"
#include "src/backend/automatic_keyset_management/dns_key.hh"
#include "src/backend/automatic_keyset_management/impl/domain_reference.hh"
#include "src/backend/automatic_keyset_management/impl/keyset_reference.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_akm_rollover.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_akm_turn_off.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_akm_turn_on.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_data.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_property.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_result.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_type.hh"
#include "src/backend/automatic_keyset_management/impl/logger_service_type.hh"
#include "src/backend/automatic_keyset_management/impl/util.hh"
#include "src/backend/epp/keyset/dns_key.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/keyset_dns_key.hh"
#include "libfred/registrable_object/keyset/update_keyset.hh"
#include "libfred/notifier/enqueue_notification.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/object/check_handle.hh"
#include "libfred/object/get_id_of_registered.hh"
#include "libfred/object/object_id_handle_pair.hh"
#include "libfred/object/object_type.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/object_state/lock_object_state_request_lock.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/opcontext.hh"
#include "libfred/poll/create_poll_message.hh"
#include "libfred/poll/create_update_object_poll_message.hh"
#include "libfred/registrable_object/domain/domain_reference.hh"
#include "libfred/registrar/info_registrar.hh"
#include "util/log/context.hh"
#include "util/random.hh"
#include "util/util.hh"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>
#include <boost/nondet_random.hpp>
#include <boost/random/uniform_int.hpp>

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace AutomaticKeysetManagement {

namespace {

std::string create_ctx_name(const std::string& name)
{
    return str(boost::format("%1%-<%2%>") % name % Random::integer(0, 10000));
}


std::string create_ctx_operation_name(const char* function_name)
{
    std::string ctx_operation_name(function_name);
    std::replace(ctx_operation_name.begin(), ctx_operation_name.end(), '_', '-');
    return ctx_operation_name;
}


class LogContext
{
public:
    LogContext(
            const AutomaticKeysetManagementImpl& _impl,
            const std::string& _ctx_operation_name)
        : ctx_server_(create_ctx_name(_impl.get_server_name())),
          ctx_operation_(_ctx_operation_name)
    {
    }


private:
    Logging::Context ctx_server_;
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR, IMPL_OBJ) LogContext CTX_VAR((IMPL_OBJ), create_ctx_operation_name(__FUNCTION__))

bool are_nssets_equal(const Nsset& nsset, const std::vector<LibFred::DnsHost>& dns_hosts)
{
    std::set<std::string> fred_dns_hosts;
    for (const auto& fred_dns_host : dns_hosts)
    {
        fred_dns_hosts.insert(fred_dns_host.get_fqdn());
    }
    return nsset.nameservers == fred_dns_hosts;
}

bool are_keysets_equal(const Keyset& keyset, const std::vector<LibFred::DnsKey>& dns_keys)
{
    std::set<DnsKey> fred_dns_keys;
    for (const auto& fred_dns_key : dns_keys)
    {
        fred_dns_keys.insert(
                DnsKey(fred_dns_key.get_flags(),
                       fred_dns_key.get_protocol(),
                       fred_dns_key.get_alg(),
                       fred_dns_key.get_key()));
    }
    return keyset.dns_keys == fred_dns_keys;
}

bool is_automatically_managed_keyset(
        LibFred::OperationContext& ctx,
        unsigned long long keyset_id,
        const std::string& automatically_managed_keyset_prefix,
        const std::string& automatically_managed_keyset_registrar)
{
    const LibFred::InfoKeysetData info_keyset_data =
            LibFred::InfoKeysetById(keyset_id).exec(ctx, "UTC").info_keyset_data;

    if (!boost::starts_with(info_keyset_data.handle, automatically_managed_keyset_prefix))
    {
        return false;
    }
    if (info_keyset_data.sponsoring_registrar_handle != automatically_managed_keyset_registrar)
    {
        return false;
    }
    return true;
}

void check_configuration_of_automatically_managed_keyset_prefix(const std::string& handle_prefix)
{
    if (handle_prefix.length() < Impl::automatically_managed_keyset_handle_prefix_length_min ||
        handle_prefix.length() > Impl::automatically_managed_keyset_handle_prefix_length_max)
    {
        LOGGER.debug("configuration error: automatically_managed_keyset_prefix configuration invalid");
        throw ConfigurationError();
    }
}

std::string generate_automatically_managed_keyset_handle(const std::string& handle_prefix)
{
    check_configuration_of_automatically_managed_keyset_prefix(handle_prefix);

    static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    boost::random_device rng;
    boost::uniform_int<> index_dist(0, alphabet.size() - 1);
    std::string result = handle_prefix;
    result.reserve(Impl::automatically_managed_keyset_handle_length);
    while (result.length() < Impl::automatically_managed_keyset_handle_length)
    {
        result += alphabet.at(index_dist(rng));
    }
    return result;
}

bool is_keyset_size_within_limits(const Keyset& keyset)
{
    return ((keyset.dns_keys.size() >= Impl::min_number_of_dns_keys) &&
            (keyset.dns_keys.size() <= Impl::max_number_of_dns_keys));
}

// RFC 8078 Section 4. "DNSSEC Delete Algorithm"
bool is_special_delete_key(const DnsKey& dns_key)
{
    static const std::string base64_encoded_zero = "AA==";
    return ((dns_key.flags == 0) &&
            (dns_key.protocol == 3) &&
            (dns_key.alg == 0) &&
            (dns_key.key == base64_encoded_zero));
}

bool is_dnssec_turn_off_requested(const Keyset& keyset)
{
    // RFC 8078 Section 4. "DNSSEC Delete Algorithm"
    if (keyset.dns_keys.size() == 1)
    {
        const DnsKey& the_only_key = *(keyset.dns_keys).begin();
        if (is_special_delete_key(the_only_key))
        {
            return true;
        }
    }
    return false;
}

bool has_valid_dnskeys(LibFred::OperationContext& ctx, const Keyset& keyset)
{
    Epp::Keyset::DnsKey::AlgValidator alg_validator(ctx);

    for (const auto& dns_key : keyset.dns_keys)
    {
        const Epp::Keyset::DnsKey epp_dns_key(dns_key.flags, dns_key.protocol, dns_key.alg, dns_key.key);

        if (!epp_dns_key.is_flags_correct())
        {
            return false;
        }

        if (!epp_dns_key.is_protocol_correct())
        {
            return false;
        }

        if (!alg_validator.is_alg_correct(epp_dns_key))
        {
            return false;
        }

        switch (epp_dns_key.check_key())
        {
            case Epp::Keyset::DnsKey::CheckKey::ok:
                break;

            case Epp::Keyset::DnsKey::CheckKey::bad_char:
                return false;

            case Epp::Keyset::DnsKey::CheckKey::bad_length:
                return false;
        }
    }

    return true;
}

Impl::KeysetReference create_automatically_managed_keyset(
        LibFred::OperationContext& ctx,
        const LibFred::InfoRegistrarData& automatically_managed_keyset_registrar,
        const std::string& automatically_managed_keyset_prefix,
        const std::string& automatically_managed_keyset_tech_contact,
        const Keyset& new_keyset,
        unsigned long long logger_request_id,
        const bool notifier_disabled)
{
    const std::string automatically_managed_keyset_handle =
            generate_automatically_managed_keyset_handle(automatically_managed_keyset_prefix);
    // TODO(akm) check !exists

    if (LibFred::TestHandleOf<LibFred::Object_Type::keyset>(automatically_managed_keyset_handle).is_invalid_handle(ctx))
    {
        throw std::runtime_error("automatically_managed_keyset_handle invalid");
    }

    std::vector<LibFred::DnsKey> libfred_dns_keys;
    for (const auto& dns_key : new_keyset.dns_keys)
    {
        libfred_dns_keys.push_back(LibFred::DnsKey(dns_key.flags, dns_key.protocol, dns_key.alg, dns_key.key));
    }

    const LibFred::CreateKeyset::Result create_keyset_result =
            LibFred::CreateKeyset(
                    automatically_managed_keyset_handle,
                    automatically_managed_keyset_registrar.handle,
                    Optional<std::string>(), // authinfopw
                    libfred_dns_keys,
                    Util::vector_of<std::string>(automatically_managed_keyset_tech_contact))
                    .exec(ctx, logger_request_id, "UTC");

    LOGGER.debug(boost::str(boost::format("creation_time: %1% object_id: %2% history_id: %3%\n")
                    % create_keyset_result.creation_time
                    % create_keyset_result.create_object_result.object_id
                    % create_keyset_result.create_object_result.history_id));

    if (!notifier_disabled)
    {
        Notification::enqueue_notification(
                ctx,
                Notification::created,
                automatically_managed_keyset_registrar.id,
                create_keyset_result.create_object_result.history_id,
                "");
    }

    return Impl::KeysetReference(
            create_keyset_result.create_object_result.object_id,
            automatically_managed_keyset_handle);
}

void link_automatically_managed_keyset_to_domain(
        LibFred::OperationContext& ctx,
        const LibFred::InfoRegistrarData& automatically_managed_keyset_registrar,
        const LibFred::InfoDomainData& info_domain_data,
        const std::string& automatically_managed_keyset,
        unsigned long long logger_request_id,
        const bool _notifier_disabled)
{
    const unsigned long long domain_new_history_id =
            LibFred::UpdateDomain(
                    info_domain_data.fqdn,
                    automatically_managed_keyset_registrar.handle)
                    .set_keyset(automatically_managed_keyset) // or get handle by create_keyset_result.create_object_result.object_id
                    .set_logd_request_id(logger_request_id)
                    .exec(ctx);

    LOGGER.debug(boost::str(boost::format("domain_new_history_id: %1%\n")
                    % domain_new_history_id));

    if (!_notifier_disabled)
    {
        Notification::enqueue_notification(
                ctx,
                Notification::updated,
                automatically_managed_keyset_registrar.id,
                domain_new_history_id,
                "");
    }

    LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, domain_new_history_id);
}

bool is_keyset_shared(LibFred::OperationContext& ctx, unsigned long long keyset_id)
{
    // clang-format off
    const std::string sql =
            "SELECT 1 < COUNT(*) AS keyset_is_shared "
              "FROM (SELECT 1 "
                      "FROM domain "
                     "WHERE keyset = $1::bigint LIMIT 2) AS some_references";
    // clang-format on
    const Database::Result db_result =
            ctx.get_conn().exec_params(
                    sql,
                    Database::query_param_list(keyset_id));
    if (db_result.size() < 1)
    {
        throw std::runtime_error("no rows");
    }
    if (db_result.size() > 1)
    {
        throw std::runtime_error("too many rows");
    }
    const bool keyset_is_shared = static_cast<bool>(db_result[0]["keyset_is_shared"]);
    return keyset_is_shared;
}

void update_automatically_managed_keyset(
        LibFred::OperationContext& ctx,
        const LibFred::InfoKeysetData& info_keyset_data,
        const LibFred::InfoRegistrarData automatically_managed_keyset_registrar,
        const Keyset& new_keyset,
        unsigned long long logger_request_id,
        const bool notifier_disabled)
{
    if (are_keysets_equal(new_keyset, info_keyset_data.dns_keys))
    {
        LOGGER.debug("new keyset same as current keyset, nothing to do");
        // nothing to commit
        return;
    }

    LibFred::LockObjectStateRequestLock(info_keyset_data.id).exec(ctx);
    // process object state requests
    LibFred::PerformObjectStateRequest(info_keyset_data.id).exec(ctx);
    const LibFred::ObjectStatesInfo keyset_states(LibFred::GetObjectStates(info_keyset_data.id).exec(ctx));

    if (keyset_states.presents(LibFred::Object_State::server_update_prohibited) ||
        keyset_states.presents(LibFred::Object_State::delete_candidate))
    {
        LOGGER.debug("keyset state prohibits action");
        throw KeysetStatePolicyError();
    }

    if (is_keyset_shared(ctx, info_keyset_data.id))
    {
        throw std::runtime_error("automatically managed keyset referenced by multiple domains. Cannot update keyset for security reasons. MUST BE fixed!");
    }

    LibFred::UpdateKeyset update_keyset(
            info_keyset_data.handle,
            automatically_managed_keyset_registrar.handle);

    for (const auto& dns_key : info_keyset_data.dns_keys)
    {
        update_keyset.rem_dns_key(dns_key);
    }

    std::vector<LibFred::DnsKey> libfred_dns_keys;
    for (const auto& dns_key : new_keyset.dns_keys)
    {
        update_keyset.add_dns_key(LibFred::DnsKey(dns_key.flags, dns_key.protocol, dns_key.alg, dns_key.key));
    }

    const unsigned long long keyset_new_history_id =
            update_keyset
                    .set_logd_request_id(logger_request_id)
                    .exec(ctx);

    LOGGER.debug(boost::str(boost::format("keyset_new_history_id: %1%\n")
                    % keyset_new_history_id));

    if (!notifier_disabled)
    {
        Notification::enqueue_notification(
                ctx,
                Notification::updated,
                automatically_managed_keyset_registrar.id,
                keyset_new_history_id,
                "");
    }
}

void unlink_automatically_managed_keyset(
        LibFred::OperationContext& ctx,
        const LibFred::InfoDomainData& info_domain_data,
        const LibFred::InfoRegistrarData& automatically_managed_keyset_registrar,
        unsigned long long logger_request_id,
        const bool notifier_disabled)
{
    const unsigned long long domain_new_history_id =
            LibFred::UpdateDomain(
                    info_domain_data.fqdn,
                    automatically_managed_keyset_registrar.handle)
            .unset_keyset()
            .set_logd_request_id(logger_request_id)
            .exec(ctx);

    LOGGER.debug(boost::str(boost::format("domain_new_history_id: %1%\n")
                    % domain_new_history_id));

    if (!notifier_disabled)
    {
        Notification::enqueue_notification(
                ctx,
                Notification::updated,
                automatically_managed_keyset_registrar.id,
                domain_new_history_id,
                "");
    }

    LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, domain_new_history_id);

}

} // namespace {anonymous}


AutomaticKeysetManagementImpl::AutomaticKeysetManagementImpl(
        const std::string& _server_name,
        const std::string& _automatically_managed_keyset_prefix,
        const std::string& _automatically_managed_keyset_registrar,
        const std::string& _automatically_managed_keyset_tech_contact,
        const std::set<std::string>& _automatically_managed_keyset_zones,
        const bool _disable_notifier,
        LibFred::Logger::LoggerClient& _logger_client)
    : server_name_(_server_name),
      automatically_managed_keyset_prefix_(_automatically_managed_keyset_prefix),
      automatically_managed_keyset_registrar_(_automatically_managed_keyset_registrar),
      automatically_managed_keyset_tech_contact_(_automatically_managed_keyset_tech_contact),
      automatically_managed_keyset_zones_(_automatically_managed_keyset_zones),
      notifier_disabled_(_disable_notifier),
      logger_client_(_logger_client)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        check_configuration_of_automatically_managed_keyset_prefix(automatically_managed_keyset_prefix_);

        LibFred::OperationContextCreator ctx;

        const LibFred::InfoRegistrarData automatically_managed_keyset_registrar =
            LibFred::InfoRegistrarByHandle(automatically_managed_keyset_registrar_).exec(ctx).info_registrar_data;

        const bool automatically_managed_keyset_registrar_is_system_registrar =
                automatically_managed_keyset_registrar.system.get_value_or(false);

        if (!automatically_managed_keyset_registrar_is_system_registrar)
        {
            LOGGER.debug("configuration error: automatically_managed_keyset_registrar is not system registrar");
            throw ConfigurationError();
        }
    }
    catch (const LibFred::InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER.error(std::string("registrar: '") + automatically_managed_keyset_registrar_ + "' not found");
            throw ConfigurationError();
        }
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown error");
        throw;
    }
}

AutomaticKeysetManagementImpl::~AutomaticKeysetManagementImpl()
{
}

std::string AutomaticKeysetManagementImpl::get_server_name() const
{
    return server_name_;
}

NameserversDomains AutomaticKeysetManagementImpl::get_nameservers_with_insecure_automatically_managed_domain_candidates()
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        NameserversDomains nameservers_domains;

        if (automatically_managed_keyset_zones_.empty())
        {
            return nameservers_domains;
        }

        // clang-format off
        Database::ParamQuery sql(
                "SELECT ns.fqdn AS nameserver, oreg.name AS domain_fqdn, oreg.id AS domain_id "
                  "FROM host ns "
                  "JOIN domain d ON d.nsset = ns.nssetid "
                  "JOIN object_registry oreg ON oreg.id = d.id "
                  "JOIN zone z ON z.id = d.zone "
                 "WHERE d.keyset IS NULL "
                   "AND z.fqdn IN (");
        Util::HeadSeparator in_separator("", ", ");
        for (const auto& automatically_managed_keyset_zone : automatically_managed_keyset_zones_)
        {
             sql(in_separator.get()).param_text(automatically_managed_keyset_zone);
        }
        sql(") ORDER BY ns.fqdn");
        // clang-format on

        const Database::Result db_result = ctx.get_conn().exec_params(sql);

        for (unsigned int idx = 0; idx < db_result.size(); ++idx)
        {
            const std::string nameserver = static_cast<std::string>(db_result[idx]["nameserver"]);
            const std::string domain_fqdn = static_cast<std::string>(db_result[idx]["domain_fqdn"]);
            const unsigned long long domain_id = static_cast<unsigned long long>(db_result[idx]["domain_id"]);
            nameservers_domains[nameserver].insert(Domain(domain_id, domain_fqdn));
        }

        return nameservers_domains;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown error");
        throw;
    }
}

NameserversDomains AutomaticKeysetManagementImpl::get_nameservers_with_secure_automatically_managed_domain_candidates()
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        NameserversDomains nameservers_domains;

        if (automatically_managed_keyset_zones_.empty())
        {
            return nameservers_domains;
        }

        // clang-format off
        Database::ParamQuery sql;
        sql(
               "SELECT ns.fqdn AS nameserver, oreg.name AS domain_fqdn, oreg.id AS domain_id "
                 "FROM host ns "
                 "JOIN domain d ON d.nsset = ns.nssetid "
                 "JOIN object_registry oreg ON oreg.id = d.id "
                 "JOIN object ok ON ok.id = d.keyset "
                 "JOIN object_registry oregk ON oregk.id = ok.id "
                 "JOIN registrar r ON r.id = ok.clid "
                 "JOIN zone z ON z.id = d.zone "
                "WHERE NOT ( "
                    "r.handle = UPPER(").param_text(automatically_managed_keyset_registrar_)(") "
                    "AND UPPER(oregk.name) LIKE UPPER(").param_text(automatically_managed_keyset_prefix_ + "%")(")) "
                  "AND oregk.erdate IS NULL "
                  "AND oregk.type = get_object_type_id('keyset') "
                  "AND z.fqdn IN (");
        Util::HeadSeparator in_separator("", ", ");
        for (const auto& automatically_managed_keyset_zone : automatically_managed_keyset_zones_)
        {
             sql(in_separator.get()).param_text(automatically_managed_keyset_zone);
        }
        sql(") ORDER BY ns.fqdn");
        // clang-format on

        const Database::Result db_result = ctx.get_conn().exec_params(sql);

        for (unsigned int idx = 0; idx < db_result.size(); ++idx)
        {
            const std::string nameserver = static_cast<std::string>(db_result[idx]["nameserver"]);
            const std::string domain_fqdn = static_cast<std::string>(db_result[idx]["domain_fqdn"]);
            const unsigned long long domain_id = static_cast<unsigned long long>(db_result[idx]["domain_id"]);
            nameservers_domains[nameserver].insert(Domain(domain_id, domain_fqdn));
        }

        return nameservers_domains;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown error");
        throw;
    }
}

NameserversDomains AutomaticKeysetManagementImpl::get_nameservers_with_automatically_managed_domains()
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        NameserversDomains nameservers_domains;

        if (automatically_managed_keyset_zones_.empty())
        {
            return nameservers_domains;
        }

        // clang-format off
        Database::ParamQuery sql;
        sql(
               "SELECT ns.fqdn AS nameserver, oreg.name AS domain_fqdn, oreg.id AS domain_id "
                 "FROM host ns "
                 "JOIN domain d ON d.nsset = ns.nssetid "
                 "JOIN object_registry oreg ON oreg.id = d.id "
                 "JOIN object ok ON ok.id = d.keyset "
                 "JOIN object_registry oregk ON oregk.id = ok.id "
                 "JOIN registrar r ON r.id = ok.clid "
                 "JOIN zone z ON z.id = d.zone "
                "WHERE r.handle = UPPER(").param_text(automatically_managed_keyset_registrar_)(") "
                  "AND UPPER(oregk.name) LIKE UPPER(").param_text(automatically_managed_keyset_prefix_ + "%")(") "
                  "AND oregk.erdate IS NULL "
                  "AND oregk.type = get_object_type_id('keyset') "
                  "AND z.fqdn IN (");
        Util::HeadSeparator in_separator("", ", ");
        for (const auto& automatically_managed_keyset_zone : automatically_managed_keyset_zones_)
        {
             sql(in_separator.get()).param_text(automatically_managed_keyset_zone);
        }
        sql(") ORDER BY ns.fqdn");
        // clang-format on

        const Database::Result db_result = ctx.get_conn().exec_params(sql);

        for (unsigned int idx = 0; idx < db_result.size(); ++idx)
        {
            const std::string nameserver = static_cast<std::string>(db_result[idx]["nameserver"]);
            const unsigned long long domain_id = static_cast<unsigned long long>(db_result[idx]["domain_id"]);
            const std::string domain_fqdn = static_cast<std::string>(db_result[idx]["domain_fqdn"]);
            nameservers_domains[nameserver].insert(Domain(domain_id, domain_fqdn));
        }

        return nameservers_domains;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown error");
        throw;
    }
}


void AutomaticKeysetManagementImpl::turn_on_automatic_keyset_management_on_insecure_domain(
        unsigned long long _domain_id,
        const Nsset& _current_nsset,
        const Keyset& _new_keyset)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        LOGGER.debug(boost::str(boost::format("domain_id: %1% current_nsset: %2% new_keyset: %3%\n")
                        % _domain_id
                        % _current_nsset.nameservers.size()
                        % _new_keyset.dns_keys.size()));

        const LibFred::InfoDomainData info_domain_data =
                LibFred::InfoDomainById(_domain_id).exec(ctx, "UTC").info_domain_data;

        const bool domain_zone_is_automatically_managed =
                automatically_managed_keyset_zones_.find(info_domain_data.zone.handle) !=
                automatically_managed_keyset_zones_.end();
        if (!domain_zone_is_automatically_managed)
        {
            throw ObjectNotFound();
        }

        if (!info_domain_data.keyset.isnull())
        {
            throw DomainHasKeyset();
        }

        if (info_domain_data.nsset.isnull())
        {
            throw DomainNssetIsEmpty();
        }

        if (_current_nsset.nameservers.empty())
        {
            throw NssetIsEmpty();
        }

        if (!is_keyset_size_within_limits(_new_keyset))
        {
            LOGGER.debug("keyset invalid: incorrect number of dns_keys");
            throw KeysetIsInvalid();
        }

        const bool turn_dnssec_off = is_dnssec_turn_off_requested(_new_keyset);
        if (turn_dnssec_off)
        {
            throw DomainAlreadyDoesNotHaveKeyset();
        }
        else if (!has_valid_dnskeys(ctx, _new_keyset))
        {
            LOGGER.debug("keyset invalid: incorrect keys");
            throw KeysetIsInvalid();
        }

        const LibFred::InfoNssetData info_nsset_data =
                LibFred::InfoNssetById(info_domain_data.nsset.get_value().id).exec(ctx, "UTC").info_nsset_data; // FIXME if does not have nsset

        if (!are_nssets_equal(_current_nsset, info_nsset_data.dns_hosts))
        {
            throw NssetIsDifferent();
        }

        LibFred::LockObjectStateRequestLock(info_domain_data.id).exec(ctx);
        // process object state requests
        LibFred::PerformObjectStateRequest(info_domain_data.id).exec(ctx);
        const LibFred::ObjectStatesInfo domain_states(LibFred::GetObjectStates(info_domain_data.id).exec(ctx));

        if (domain_states.presents(LibFred::Object_State::server_blocked) ||
            domain_states.presents(LibFred::Object_State::server_update_prohibited) ||
            domain_states.presents(LibFred::Object_State::delete_candidate))
        {
            throw DomainStatePolicyError();
        }

        const LibFred::InfoRegistrarData automatically_managed_keyset_registrar =
            LibFred::InfoRegistrarByHandle(automatically_managed_keyset_registrar_).exec(ctx).info_registrar_data;

        const bool automatically_managed_keyset_registrar_is_system_registrar =
                automatically_managed_keyset_registrar.system.get_value_or(false);

        if (!automatically_managed_keyset_registrar_is_system_registrar)
        {
            LOGGER.debug("configuration error: automatically_managed_keyset_registrar is not system registrar");
            throw ConfigurationError();
        }

        try
        {
            LibFred::get_id_of_registered<LibFred::Object_Type::contact>(ctx, automatically_managed_keyset_tech_contact_);
        }
        catch (const ObjectNotFound&)
        {
            LOGGER.error(std::string("registrar: '") + automatically_managed_keyset_tech_contact_ + "' not found");
            throw ConfigurationError();
        }

        check_configuration_of_automatically_managed_keyset_prefix(automatically_managed_keyset_prefix_);

        const Impl::LoggerRequestAkmTurnOn logger_request(
                logger_client_,
                Impl::DomainReference(
                        info_domain_data.id,
                        info_domain_data.fqdn));
        try
        {
            const Impl::KeysetReference automatically_managed_keyset = create_automatically_managed_keyset(
                    ctx,
                    automatically_managed_keyset_registrar,
                    automatically_managed_keyset_prefix_,
                    automatically_managed_keyset_tech_contact_,
                    _new_keyset,
                    logger_request.get_request_id(),
                    notifier_disabled_);

            link_automatically_managed_keyset_to_domain(
                    ctx,
                    automatically_managed_keyset_registrar,
                    info_domain_data,
                    automatically_managed_keyset.handle,
                    logger_request.get_request_id(),
                    notifier_disabled_);

            ctx.commit_transaction();

            logger_request.close_on_success(
                    Impl::DomainReference(info_domain_data.id, info_domain_data.fqdn),
                    automatically_managed_keyset,
                    _new_keyset);
        }
        catch (...)
        {
            logger_request.close_on_failure();
            throw;
        }

    }
    catch (const LibFred::InfoDomainById::Exception& e)
    {
        if (e.is_set_unknown_object_id())
        {
            throw ObjectNotFound();
        }
        throw;
    }
    catch (const LibFred::InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER.error(std::string("registrar: '") + automatically_managed_keyset_registrar_ + "' not found");
            throw ConfigurationError();
        }
        throw;
    }
    catch (const LibFred::CreateKeyset::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER.warning("unknown registrar handle");
            throw ConfigurationError();
        }
        if (e.is_set_vector_of_already_set_dns_key())
        {
            LOGGER.warning("duplicate dns key");
        }
        if (e.is_set_vector_of_unknown_technical_contact_handle())
        {
            LOGGER.warning("unknown technical contact handle");
            throw ConfigurationError();
        }
        if (e.is_set_vector_of_already_set_technical_contact_handle())
        {
            LOGGER.warning("duplicate technical contact handle");
        }
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown error");
        throw;
    }
}


void AutomaticKeysetManagementImpl::turn_on_automatic_keyset_management_on_secure_domain(
        unsigned long long _domain_id,
        const Keyset& _new_keyset)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        LOGGER.debug(boost::str(boost::format("domain_id: %1% new_keyset: %2%\n")
                        % _domain_id
                        % _new_keyset.dns_keys.size()));

        const LibFred::InfoDomainData info_domain_data =
                LibFred::InfoDomainById(_domain_id).exec(ctx, "UTC").info_domain_data;

        const bool domain_zone_is_automatically_managed =
                automatically_managed_keyset_zones_.find(info_domain_data.zone.handle) !=
                automatically_managed_keyset_zones_.end();
        if (!domain_zone_is_automatically_managed)
        {
            throw ObjectNotFound();
        }

        if (info_domain_data.keyset.isnull())
        {
            throw DomainDoesNotHaveKeyset();
        }

        const bool domain_has_automatically_managed_keyset =
                !info_domain_data.keyset.isnull() &&
                is_automatically_managed_keyset(
                        ctx,
                        info_domain_data.keyset.get_value().id,
                        automatically_managed_keyset_prefix_,
                        automatically_managed_keyset_registrar_);

        if (domain_has_automatically_managed_keyset)
        {
            throw DomainAlreadyHasAutomaticallyManagedKeyset();
        }

        if (!is_keyset_size_within_limits(_new_keyset))
        {
            LOGGER.debug("keyset invalid: incorrect number of dns_keys");
            throw KeysetIsInvalid();
        }

        const bool turn_dnssec_off = is_dnssec_turn_off_requested(_new_keyset);
        if (!turn_dnssec_off && !has_valid_dnskeys(ctx, _new_keyset))
        {
            LOGGER.debug("keyset invalid: incorrect keys");
            throw KeysetIsInvalid();
        }

        LibFred::LockObjectStateRequestLock(info_domain_data.id).exec(ctx);
        // process object state requests
        LibFred::PerformObjectStateRequest(info_domain_data.id).exec(ctx);
        const LibFred::ObjectStatesInfo domain_states(LibFred::GetObjectStates(info_domain_data.id).exec(ctx));

        if (domain_states.presents(LibFred::Object_State::server_blocked) ||
            domain_states.presents(LibFred::Object_State::server_update_prohibited) ||
            domain_states.presents(LibFred::Object_State::delete_candidate))
        {
            throw DomainStatePolicyError();
        }

        const LibFred::InfoRegistrarData automatically_managed_keyset_registrar =
            LibFred::InfoRegistrarByHandle(automatically_managed_keyset_registrar_).exec(ctx).info_registrar_data;

        const bool automatically_managed_keyset_registrar_is_system_registrar =
                automatically_managed_keyset_registrar.system.get_value_or(false);

        if (!automatically_managed_keyset_registrar_is_system_registrar)
        {
            LOGGER.debug("configuration error: automatically_managed_keyset_registrar is not system registrar");
            throw ConfigurationError();
        }

        try
        {
            LibFred::get_id_of_registered<LibFred::Object_Type::contact>(ctx, automatically_managed_keyset_tech_contact_);
        }
        catch (const ObjectNotFound&)
        {
            LOGGER.error(std::string("registrar: '") + automatically_managed_keyset_tech_contact_ + "' not found");
            throw ConfigurationError();
        }

        if (turn_dnssec_off)
        {
            const Impl::LoggerRequestAkmTurnOff logger_request(
                    logger_client_,
                    Impl::DomainReference(
                            info_domain_data.id,
                            info_domain_data.fqdn));
            try
            {
                unlink_automatically_managed_keyset(
                        ctx,
                        info_domain_data,
                        automatically_managed_keyset_registrar,
                        logger_request.get_request_id(),
                        notifier_disabled_);

                ctx.commit_transaction();

                logger_request.close_on_success(
                        Impl::DomainReference(
                                info_domain_data.id,
                                info_domain_data.fqdn),
                        Impl::KeysetReference(
                                info_domain_data.keyset.get_value().id,
                                info_domain_data.keyset.get_value().handle));
            }
            catch (...)
            {
                logger_request.close_on_failure();
                throw;
            }
        }
        else
        {
            const Impl::LoggerRequestAkmTurnOn logger_request(
                    logger_client_,
                    Impl::DomainReference(
                            info_domain_data.id,
                            info_domain_data.fqdn));
            try
            {
                const Impl::KeysetReference automatically_managed_keyset = create_automatically_managed_keyset(
                        ctx,
                        automatically_managed_keyset_registrar,
                        automatically_managed_keyset_prefix_,
                        automatically_managed_keyset_tech_contact_,
                        _new_keyset,
                        logger_request.get_request_id(),
                        notifier_disabled_);

                link_automatically_managed_keyset_to_domain(
                        ctx,
                        automatically_managed_keyset_registrar,
                        info_domain_data,
                        automatically_managed_keyset.handle,
                        logger_request.get_request_id(),
                        notifier_disabled_);

                ctx.commit_transaction();

                logger_request.close_on_success(
                        Impl::DomainReference(info_domain_data.id, info_domain_data.fqdn),
                        automatically_managed_keyset,
                        _new_keyset);
            }
            catch (...)
            {
                logger_request.close_on_failure();
                throw;
            }
        }

    }
    catch (const LibFred::InfoDomainById::Exception& e)
    {
        if (e.is_set_unknown_object_id())
        {
            throw ObjectNotFound();
        }
        throw;
    }
    catch (const LibFred::InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER.error(std::string("registrar: '") + automatically_managed_keyset_registrar_ + "' not found");
            throw ConfigurationError();
        }
        throw;
    }
    catch (const LibFred::CreateKeyset::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER.warning("unknown registrar handle");
            throw ConfigurationError();
        }
        if (e.is_set_vector_of_already_set_dns_key())
        {
            LOGGER.warning("duplicate dns key");
        }
        if (e.is_set_vector_of_unknown_technical_contact_handle())
        {
            LOGGER.warning("unknown technical contact handle");
            throw ConfigurationError();
        }
        if (e.is_set_vector_of_already_set_technical_contact_handle())
        {
            LOGGER.warning("duplicate technical contact handle");
        }
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown error");
        throw;
    }
}


void AutomaticKeysetManagementImpl::update_automatically_managed_keyset_of_domain(
        unsigned long long _domain_id,
        const Keyset& _new_keyset)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        LOGGER.debug(boost::str(boost::format("domain_id: %1% new_keyset: %2%\n")
                        % _domain_id
                        % _new_keyset.dns_keys.size()));

        const LibFred::InfoDomainData info_domain_data =
                LibFred::InfoDomainById(_domain_id).exec(ctx, "UTC").info_domain_data;

        const bool domain_zone_is_automatically_managed =
                automatically_managed_keyset_zones_.find(info_domain_data.zone.handle) !=
                automatically_managed_keyset_zones_.end();
        if (!domain_zone_is_automatically_managed)
        {
            throw ObjectNotFound();
        }

        const bool domain_has_automatically_managed_keyset =
                !info_domain_data.keyset.isnull() &&
                is_automatically_managed_keyset(
                        ctx,
                        info_domain_data.keyset.get_value().id,
                        automatically_managed_keyset_prefix_,
                        automatically_managed_keyset_registrar_);

        if (!domain_has_automatically_managed_keyset)
        {
            throw DomainDoesNotHaveAutomaticallyManagedKeyset();
        }

        if (!is_keyset_size_within_limits(_new_keyset))
        {
            LOGGER.debug("keyset invalid: incorrect number of dns_keys");
            throw KeysetIsInvalid();
        }

        const bool turn_dnssec_off = is_dnssec_turn_off_requested(_new_keyset);
        if (!turn_dnssec_off && !has_valid_dnskeys(ctx, _new_keyset))
        {
            LOGGER.debug("keyset invalid: incorrect keys");
            throw KeysetIsInvalid();
        }

        const LibFred::InfoRegistrarData automatically_managed_keyset_registrar =
            LibFred::InfoRegistrarByHandle(automatically_managed_keyset_registrar_).exec(ctx).info_registrar_data;

        const bool automatically_managed_keyset_registrar_is_system_registrar =
                automatically_managed_keyset_registrar.system.get_value_or(false);

        if (!automatically_managed_keyset_registrar_is_system_registrar)
        {
            LOGGER.debug("configuration error: automatically_managed_keyset_registrar is not system registrar");
            throw ConfigurationError();
        }

        if (turn_dnssec_off)
        {
            LibFred::LockObjectStateRequestLock(info_domain_data.id).exec(ctx);
            // process object state requests
            LibFred::PerformObjectStateRequest(info_domain_data.id).exec(ctx);
            const LibFred::ObjectStatesInfo domain_states(LibFred::GetObjectStates(info_domain_data.id).exec(ctx));

            if (domain_states.presents(LibFred::Object_State::server_blocked) ||
                domain_states.presents(LibFred::Object_State::server_update_prohibited) ||
                domain_states.presents(LibFred::Object_State::delete_candidate))
            {
                throw DomainStatePolicyError();
            }

            const Impl::LoggerRequestAkmTurnOff logger_request(
                    logger_client_,
                    Impl::DomainReference(
                            info_domain_data.id,
                            info_domain_data.fqdn));
            try
            {
                unlink_automatically_managed_keyset(
                        ctx,
                        info_domain_data,
                        automatically_managed_keyset_registrar,
                        logger_request.get_request_id(),
                        notifier_disabled_);

                ctx.commit_transaction();

                logger_request.close_on_success(
                        Impl::DomainReference(
                                info_domain_data.id,
                                info_domain_data.fqdn),
                        Impl::KeysetReference(
                                info_domain_data.keyset.get_value().id,
                                info_domain_data.keyset.get_value().handle));
            }
            catch (...)
            {
                logger_request.close_on_failure();
                throw;
            }
        }
        else
        {
            const LibFred::InfoKeysetData info_keyset_data =
                    LibFred::InfoKeysetById(info_domain_data.keyset.get_value().id).exec(ctx, "UTC").info_keyset_data;

            if (are_keysets_equal(_new_keyset, info_keyset_data.dns_keys))
            {
                throw KeysetSameAsDomainKeyset();
            }

            LibFred::LockObjectStateRequestLock(info_keyset_data.id).exec(ctx);
            // process object state requests
            LibFred::PerformObjectStateRequest(info_keyset_data.id).exec(ctx);
            const LibFred::ObjectStatesInfo keyset_states(LibFred::GetObjectStates(info_keyset_data.id).exec(ctx));

            if (keyset_states.presents(LibFred::Object_State::server_update_prohibited) ||
                keyset_states.presents(LibFred::Object_State::delete_candidate))
            {
                throw KeysetStatePolicyError();
            }

            const Impl::LoggerRequestAkmRollover logger_request(
                    logger_client_,
                    Impl::DomainReference(
                            info_domain_data.id,
                            info_domain_data.fqdn));
            try
            {
                update_automatically_managed_keyset(
                        ctx,
                        info_keyset_data,
                        automatically_managed_keyset_registrar,
                        _new_keyset,
                        logger_request.get_request_id(),
                        notifier_disabled_);

                ctx.commit_transaction();

                logger_request.close_on_success(
                        Impl::DomainReference(
                                info_domain_data.id,
                                info_domain_data.fqdn),
                        Impl::KeysetReference(
                                info_domain_data.keyset.get_value().id,
                                info_domain_data.keyset.get_value().handle),
                        info_keyset_data.dns_keys,
                        _new_keyset);
            }
            catch (...)
            {
                logger_request.close_on_failure();
                throw;
            }
        }

    }
    catch (const LibFred::InfoDomainById::Exception& e)
    {
        if (e.is_set_unknown_object_id())
        {
            throw ObjectNotFound();
        }
        throw;
    }
    catch (const LibFred::InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER.error(std::string("registrar: '") + automatically_managed_keyset_registrar_ + "' not found");
            throw AutomaticKeysetManagement::ConfigurationError();
        }
        throw;
    }
    catch (const LibFred::UpdateKeyset::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_keyset_handle())
        {
            LOGGER.warning("unknown keyset handle");
        }
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER.warning("unknown registrar handle");
            throw ConfigurationError();
        }
        if (e.is_set_vector_of_already_set_dns_key())
        {
            LOGGER.warning("duplicate dns key");
        }
        if (e.is_set_vector_of_unassigned_dns_key())
        {
            LOGGER.warning("unassigned dns key");
        }
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown error");
        throw;
    }
}


EmailAddresses AutomaticKeysetManagementImpl::get_email_addresses_by_domain_id(
        unsigned long long _domain_id)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        EmailAddresses email_addresses;
        LibFred::OperationContextCreator ctx;

        {
            const std::string sql =
                    "SELECT 1 FROM domain WHERE id = $1::bigint";
            const Database::Result db_result =
                    ctx.get_conn().exec_params(
                            sql,
                            Database::query_param_list(_domain_id));
            if (db_result.size() < 1)
            {
                throw ObjectNotFound();
            }
            if (db_result.size() > 1)
            {
                throw std::runtime_error("too many rows");
            }
        }

        // clang-format off
        const std::string sql =
                "SELECT COALESCE(NULLIF(c.notifyemail, ''), NULLIF(c.email, '')) AS email_address "
                  "FROM domain d "
                  "JOIN nsset_contact_map ncmap ON ncmap.nssetid = d.nsset "
                  "JOIN contact c ON ncmap.contactid = c.id "
                 "WHERE d.id = $1::bigint "
                   "AND COALESCE(NULLIF(c.notifyemail, ''), NULLIF(c.email, '')) IS NOT NULL";
        // clang-format on

        const Database::Result db_result =
                ctx.get_conn().exec_params(
                        sql,
                        Database::query_param_list(_domain_id));

        LOGGER.debug(boost::str(boost::format("found %d email(s)") % db_result.size()));

        for (unsigned int idx = 0; idx < db_result.size(); ++idx)
        {
            const std::string email_address = static_cast<std::string>(db_result[idx]["email_address"]);
            email_addresses.insert(email_address);
            LOGGER.debug(std::string("email: ") + email_address);
        }

        return email_addresses;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown error");
        throw;
    }
}

bool operator<(const Domain& lhs, const Domain& rhs)
{
    if (lhs.id != rhs.id) {
        return lhs.id < rhs.id;
    }
    return lhs.fqdn < rhs.fqdn;
}

} // namespace Fred::Backend::AutomaticKeysetManagement
} // namespace Fred::Backend
} // namespace Fred
