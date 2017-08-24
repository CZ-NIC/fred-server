/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file automatic_keyset_management.cc
 *  automatic keyset management implementation
 */

#include "src/automatic_keyset_management/automatic_keyset_management.hh"

#include "src/automatic_keyset_management/impl/limits.hh"
#include "src/automatic_keyset_management/impl/logger_request.hh"
#include "src/automatic_keyset_management/impl/logger_request_akm_rollover.hh"
#include "src/automatic_keyset_management/impl/logger_request_akm_turn_off.hh"
#include "src/automatic_keyset_management/impl/logger_request_akm_turn_on.hh"
#include "src/automatic_keyset_management/impl/logger_request_data.hh"
#include "src/automatic_keyset_management/impl/logger_request_property.hh"
#include "src/automatic_keyset_management/impl/logger_request_result.hh"
#include "src/automatic_keyset_management/impl/logger_request_type.hh"
#include "src/automatic_keyset_management/impl/logger_service_type.hh"
#include "src/automatic_keyset_management/impl/util.hh"
#include "src/epp/keyset/dns_key.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/update_domain.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/keyset_dns_key.h"
#include "src/fredlib/keyset/update_keyset.h"
#include "src/fredlib/notifier/enqueue_notification.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/object/check_handle.h"
#include "src/fredlib/object/get_id_of_registered.h"
#include "src/fredlib/object/object_id_handle_pair.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/poll/create_transfer_domain_poll_message.h"
#include "src/fredlib/poll/create_update_object_poll_message.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/log/context.h"
#include "util/random.h"
#include "util/util.h"

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
namespace AutomaticKeysetManagement {

bool DnsKey::operator<(const DnsKey& rhs) const
{
    if (key != rhs.key) {
        return key < rhs.key;
    }
    if (alg != rhs.alg) {
        return alg < rhs.alg;
    }
    if (protocol != rhs.protocol) {
        return protocol < rhs.protocol;
    }
    return flags < rhs.flags;
}

bool DnsKey::operator==(const DnsKey& rhs) const
{
    return !(*this < rhs || rhs < *this);
}

namespace {

std::string create_ctx_name(const std::string& _name)
{
    return str(boost::format("%1%-<%2%>") % _name % Random::integer(0, 10000));
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

bool are_nssets_equal(const Nsset& _nsset, const std::vector<Fred::DnsHost>& _dns_hosts)
{
    std::set<std::string> dns_hosts;
    for (std::vector<Fred::DnsHost>::const_iterator dns_host = _dns_hosts.begin();
         dns_host != _dns_hosts.end();
         ++dns_host)
    {
        dns_hosts.insert(dns_host->get_fqdn());
    }
    return _nsset.nameservers == dns_hosts;
}

bool are_keysets_equal(const Keyset& _keyset, const std::vector<Fred::DnsKey>& _dns_keys)
{
    std::set<DnsKey> dns_keys;
    for (std::vector<Fred::DnsKey>::const_iterator key = _dns_keys.begin(); key != _dns_keys.end(); ++key)
    {
        dns_keys.insert(DnsKey(key->get_flags(), key->get_protocol(), key->get_alg(), key->get_key()));
    }
    return _keyset.dns_keys == dns_keys;
}

bool is_automatically_managed_keyset(
        Fred::OperationContext& ctx,
        unsigned long long keyset_id,
        const std::string& automatically_managed_keyset_prefix,
        const std::string& automatically_managed_keyset_registrar)
{
    const Fred::InfoKeysetData info_keyset_data =
            Fred::InfoKeysetById(keyset_id).exec(ctx, "UTC").info_keyset_data;

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
    if (handle_prefix.length() < automatically_managed_keyset_handle_prefix_length_min ||
        handle_prefix.length() > automatically_managed_keyset_handle_prefix_length_max)
    {
        LOGGER(PACKAGE).debug("configuration error: automatically_managed_keyset_prefix configuration invalid");
        throw Fred::AutomaticKeysetManagement::ConfigurationError();
    }
}

std::string generate_automatically_managed_keyset_handle(const std::string& handle_prefix)
{
    check_configuration_of_automatically_managed_keyset_prefix(handle_prefix);

    static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    boost::random_device rng;
    boost::uniform_int<> index_dist(0, alphabet.size() - 1);
    std::string result = handle_prefix;
    result.reserve(automatically_managed_keyset_handle_length);
    while (result.length() < automatically_managed_keyset_handle_length)
    {
        result += alphabet.at(index_dist(rng));
    }
    return result;
}

bool is_keyset_size_within_limits(const Keyset& keyset)
{
    return ((keyset.dns_keys.size() >= min_number_of_dns_keys) &&
            (keyset.dns_keys.size() <= max_number_of_dns_keys));
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

bool has_valid_dnskeys(Fred::OperationContext& ctx, const Keyset& keyset)
{
    Epp::KeySet::DnsKey::AlgValidator alg_validator(ctx);

    for (DnsKeys::const_iterator dns_key = keyset.dns_keys.begin();
         dns_key != keyset.dns_keys.end(); ++dns_key)
    {
        const Epp::KeySet::DnsKey epp_dns_key(dns_key->flags, dns_key->protocol, dns_key->alg, dns_key->key);

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
            case Epp::KeySet::DnsKey::CheckKey::ok:
                break;

            case Epp::KeySet::DnsKey::CheckKey::bad_char:
                return false;

            case Epp::KeySet::DnsKey::CheckKey::bad_length:
                return false;
        }
    }

    return true;
}

ObjectIdHandlePair create_automatically_managed_keyset(
        Fred::OperationContext& _ctx,
        const InfoRegistrarData& _automatically_managed_keyset_registrar,
        const std::string& _automatically_managed_keyset_prefix,
        const std::string& _automatically_managed_keyset_tech_contact,
        const Keyset& _new_keyset,
        unsigned long long _logger_request_id,
        const bool _notifier_disabled)
{
    const std::string automatically_managed_keyset_handle =
            generate_automatically_managed_keyset_handle(_automatically_managed_keyset_prefix);
    // TODO check !exists

    if (TestHandleOf<Object_Type::keyset>(automatically_managed_keyset_handle).is_invalid_handle())
    {
        throw std::runtime_error("automatically_managed_keyset_handle invalid");
    }

    std::vector<Fred::DnsKey> libfred_dns_keys;
    for (DnsKeys::const_iterator dns_key = _new_keyset.dns_keys.begin();
            dns_key != _new_keyset.dns_keys.end();
            ++dns_key)
    {
        libfred_dns_keys.push_back(Fred::DnsKey(dns_key->flags, dns_key->protocol, dns_key->alg, dns_key->key));
    }

    const Fred::CreateKeyset::Result create_keyset_result =
            Fred::CreateKeyset(
                    automatically_managed_keyset_handle,
                    _automatically_managed_keyset_registrar.handle,
                    Optional<std::string>(), // authinfopw
                    libfred_dns_keys,
                    Util::vector_of<std::string>(_automatically_managed_keyset_tech_contact))
                    .exec(_ctx, _logger_request_id, "UTC");

    LOGGER(PACKAGE).debug(boost::str(boost::format("creation_time: %1% object_id: %2% history_id: %3%\n")
                    % create_keyset_result.creation_time
                    % create_keyset_result.create_object_result.object_id
                    % create_keyset_result.create_object_result.history_id));

    if (!_notifier_disabled)
    {
        Notification::enqueue_notification(
                _ctx,
                Notification::created,
                _automatically_managed_keyset_registrar.id,
                create_keyset_result.create_object_result.history_id,
                "");
    }

    return ObjectIdHandlePair(create_keyset_result.create_object_result.object_id, automatically_managed_keyset_handle);
}

void link_automatically_managed_keyset_to_domain(
        Fred::OperationContext& _ctx,
        const InfoRegistrarData& _automatically_managed_keyset_registrar,
        const InfoDomainData& _info_domain_data,
        const std::string& _automatically_managed_keyset,
        unsigned long long logger_request_id,
        const bool _notifier_disabled)
{
    const unsigned long long domain_new_history_id =
            Fred::UpdateDomain(
                    _info_domain_data.fqdn,
                    _automatically_managed_keyset_registrar.handle)
                    .set_keyset(_automatically_managed_keyset) // or get handle by create_keyset_result.create_object_result.object_id
                    .set_logd_request_id(logger_request_id)
                    .exec(_ctx);

    LOGGER(PACKAGE).debug(boost::str(boost::format("domain_new_history_id: %1%\n")
                    % domain_new_history_id));

    if (!_notifier_disabled)
    {
        Notification::enqueue_notification(
                _ctx,
                Notification::updated,
                _automatically_managed_keyset_registrar.id,
                domain_new_history_id,
                "");
    }

    Fred::Poll::CreateUpdateObjectPollMessage(domain_new_history_id).exec(_ctx);
}

bool is_keyset_shared(Fred::OperationContext& _ctx, unsigned long long _keyset_id)
{
    const std::string sql =
            "SELECT 1 < COUNT(*) AS keyset_is_shared "
              "FROM (SELECT 1 "
                      "FROM domain "
                     "WHERE keyset = $1::bigint LIMIT 2) AS some_references";
    const Database::Result db_result =
            _ctx.get_conn().exec_params(
                    sql,
                    Database::query_param_list(_keyset_id));
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
        Fred::OperationContext& _ctx,
        const InfoKeysetData& _info_keyset_data,
        const InfoRegistrarData _automatically_managed_keyset_registrar,
        const Keyset& _new_keyset,
        unsigned long long logger_request_id,
        const bool _notifier_disabled)
{
    if (are_keysets_equal(_new_keyset, _info_keyset_data.dns_keys))
    {
        LOGGER(PACKAGE).debug("new keyset same as current keyset, nothing to do");
        // nothing to commit
        return;
    }

    Fred::LockObjectStateRequestLock(_info_keyset_data.id).exec(_ctx);
    // process object state requests
    Fred::PerformObjectStateRequest(_info_keyset_data.id).exec(_ctx);
    const Fred::ObjectStatesInfo keyset_states(Fred::GetObjectStates(_info_keyset_data.id).exec(_ctx));

    if (keyset_states.presents(Fred::Object_State::server_update_prohibited) ||
        keyset_states.presents(Fred::Object_State::delete_candidate))
    {
        LOGGER(PACKAGE).debug("keyset state prohibits action");
        throw Fred::AutomaticKeysetManagement::KeysetStatePolicyError();
    }

    if (is_keyset_shared(_ctx, _info_keyset_data.id))
    {
        throw std::runtime_error("automatically managed keyset referenced by multiple domains. Cannot update keyset for security reasons. MUST BE fixed!");
    }

    Fred::UpdateKeyset update_keyset(
            _info_keyset_data.handle,
            _automatically_managed_keyset_registrar.handle);

    for (std::vector<Fred::DnsKey>::const_iterator dns_key = _info_keyset_data.dns_keys.begin();
         dns_key != _info_keyset_data.dns_keys.end();
         ++dns_key)
    {
        update_keyset.rem_dns_key(*dns_key);
    }

    std::vector<Fred::DnsKey> libfred_dns_keys;
    for (DnsKeys::const_iterator dns_key = _new_keyset.dns_keys.begin();
            dns_key != _new_keyset.dns_keys.end();
            ++dns_key)
    {
        update_keyset.add_dns_key(Fred::DnsKey(dns_key->flags, dns_key->protocol, dns_key->alg, dns_key->key));
    }

    const unsigned long long keyset_new_history_id =
            update_keyset
                    .set_logd_request_id(logger_request_id)
                    .exec(_ctx);

    LOGGER(PACKAGE).debug(boost::str(boost::format("keyset_new_history_id: %1%\n")
                    % keyset_new_history_id));

    if (!_notifier_disabled)
    {
        Notification::enqueue_notification(
                _ctx,
                Notification::updated,
                _automatically_managed_keyset_registrar.id,
                keyset_new_history_id,
                "");
    }
}

void unlink_automatically_managed_keyset(
        Fred::OperationContext& _ctx,
        const InfoDomainData& _info_domain_data,
        const InfoRegistrarData& _automatically_managed_keyset_registrar,
        unsigned long long logger_request_id,
        const bool _notifier_disabled)
{
    const unsigned long long domain_new_history_id =
            Fred::UpdateDomain(
                    _info_domain_data.fqdn,
                    _automatically_managed_keyset_registrar.handle)
            .unset_keyset()
            .set_logd_request_id(logger_request_id)
            .exec(_ctx);

    LOGGER(PACKAGE).debug(boost::str(boost::format("domain_new_history_id: %1%\n")
                    % domain_new_history_id));

    if (!_notifier_disabled)
    {
        Notification::enqueue_notification(
                _ctx,
                Notification::updated,
                _automatically_managed_keyset_registrar.id,
                domain_new_history_id,
                "");
    }

    Fred::Poll::CreateUpdateObjectPollMessage(domain_new_history_id).exec(_ctx);

}

} // namespace {anonymous}


AutomaticKeysetManagementImpl::AutomaticKeysetManagementImpl(
        const std::string& _server_name,
        const std::string& _automatically_managed_keyset_prefix,
        const std::string& _automatically_managed_keyset_registrar,
        const std::string& _automatically_managed_keyset_tech_contact,
        const std::set<std::string>& _automatically_managed_keyset_zones,
        const bool _disable_notifier,
        Fred::Logger::LoggerClient& _logger_client)
    : server_name_(_server_name),
      automatically_managed_keyset_prefix_(_automatically_managed_keyset_prefix),
      automatically_managed_keyset_registrar_(_automatically_managed_keyset_registrar),
      automatically_managed_keyset_tech_contact_(_automatically_managed_keyset_tech_contact),
      automatically_managed_keyset_zones_(_automatically_managed_keyset_zones),
      notifier_disabled_(_disable_notifier),
      logger_client_(_logger_client)
{
    LOGGING_CONTEXT(log_ctx, *this);

    check_configuration_of_automatically_managed_keyset_prefix(automatically_managed_keyset_prefix_);

    try
    {
        Fred::OperationContextCreator ctx;

        const Fred::InfoRegistrarData automatically_managed_keyset_registrar =
            Fred::InfoRegistrarByHandle(automatically_managed_keyset_registrar_).exec(ctx).info_registrar_data;

        const bool automatically_managed_keyset_registrar_is_system_registrar =
                automatically_managed_keyset_registrar.system.get_value_or(false);

        if (!automatically_managed_keyset_registrar_is_system_registrar)
        {
            LOGGER(PACKAGE).debug("configuration error: automatically_managed_keyset_registrar is not system registrar");
            throw Fred::AutomaticKeysetManagement::ConfigurationError();
        }
    }
    catch (const Fred::InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER(PACKAGE).error(std::string("registrar: '") + automatically_managed_keyset_registrar_ + "' not found");
            throw Fred::AutomaticKeysetManagement::ConfigurationError();
        }
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
        NameserversDomains nameservers_domains;
        Fred::OperationContextCreator ctx;

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
        for (std::set<std::string>::const_iterator it = automatically_managed_keyset_zones_.begin();
                it != automatically_managed_keyset_zones_.end(); ++it)
        {
             sql(in_separator.get()).param_text(*it);
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
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown error");
        throw;
    }
}

NameserversDomains AutomaticKeysetManagementImpl::get_nameservers_with_secure_automatically_managed_domain_candidates()
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        NameserversDomains nameservers_domains;
        Fred::OperationContextCreator ctx;

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
        for (std::set<std::string>::const_iterator it = automatically_managed_keyset_zones_.begin();
                it != automatically_managed_keyset_zones_.end(); ++it)
        {
             sql(in_separator.get()).param_text(*it);
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
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown error");
        throw;
    }
}

NameserversDomains AutomaticKeysetManagementImpl::get_nameservers_with_automatically_managed_domains()
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        NameserversDomains nameservers_domains;
        Fred::OperationContextCreator ctx;

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
        for (std::set<std::string>::const_iterator it = automatically_managed_keyset_zones_.begin();
                it != automatically_managed_keyset_zones_.end(); ++it)
        {
             sql(in_separator.get()).param_text(*it);
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
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown error");
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
        Fred::OperationContextCreator ctx;
        LOGGER(PACKAGE).debug(boost::str(boost::format("domain_id: %1% current_nsset: %2% new_keyset: %3%\n")
                        % _domain_id
                        % _current_nsset.nameservers.size()
                        % _new_keyset.dns_keys.size()));

        const Fred::InfoDomainData info_domain_data =
                Fred::InfoDomainById(_domain_id).exec(ctx, "UTC").info_domain_data;

        const bool domain_zone_is_automatically_managed =
                automatically_managed_keyset_zones_.find(info_domain_data.zone.handle) !=
                automatically_managed_keyset_zones_.end();
        if (!domain_zone_is_automatically_managed)
        {
            throw Fred::AutomaticKeysetManagement::ObjectNotFound();
        }

        if (!info_domain_data.keyset.isnull())
        {
            LOGGER(PACKAGE).debug("domain has a keyset (is not insecure)");
            throw Fred::AutomaticKeysetManagement::DomainHasKeyset();
        }

        if (info_domain_data.nsset.isnull())
        {
            LOGGER(PACKAGE).debug("current_nsset empty");
            throw Fred::AutomaticKeysetManagement::DomainNssetIsEmpty();
        }

        if (_current_nsset.nameservers.empty())
        {
            LOGGER(PACKAGE).debug("domain nsset is empty");
            throw Fred::AutomaticKeysetManagement::NssetIsEmpty();
        }

        if (!is_keyset_size_within_limits(_new_keyset))
        {
            LOGGER(PACKAGE).debug("keyset invalid: incorrect number of dns_keys");
            throw Fred::AutomaticKeysetManagement::KeysetIsInvalid();
        }

        const bool turn_dnssec_off = is_dnssec_turn_off_requested(_new_keyset);
        if (turn_dnssec_off)
        {
            LOGGER(PACKAGE).debug("domain already does not have any keyset to remove");
            throw Fred::AutomaticKeysetManagement::DomainAlreadyDoesNotHaveKeyset();
        }
        else if (!has_valid_dnskeys(ctx, _new_keyset))
        {
            LOGGER(PACKAGE).debug("keyset invalid: incorrect keys");
            throw Fred::AutomaticKeysetManagement::KeysetIsInvalid();
        }

        const Fred::InfoNssetData info_nsset_data =
                Fred::InfoNssetById(info_domain_data.nsset.get_value().id).exec(ctx, "UTC").info_nsset_data; // FIXME if does not have nsset

        if (!are_nssets_equal(_current_nsset, info_nsset_data.dns_hosts))
        {
            throw Fred::AutomaticKeysetManagement::NssetIsDifferent();
        }

        Fred::LockObjectStateRequestLock(info_domain_data.id).exec(ctx);
        // process object state requests
        Fred::PerformObjectStateRequest(info_domain_data.id).exec(ctx);
        const Fred::ObjectStatesInfo domain_states(Fred::GetObjectStates(info_domain_data.id).exec(ctx));

        if (domain_states.presents(Fred::Object_State::server_blocked) ||
            domain_states.presents(Fred::Object_State::server_update_prohibited) ||
            domain_states.presents(Fred::Object_State::delete_candidate))
        {
            LOGGER(PACKAGE).debug("domain state prohibits action");
            throw Fred::AutomaticKeysetManagement::DomainStatePolicyError();
        }

        const Fred::InfoRegistrarData automatically_managed_keyset_registrar =
            Fred::InfoRegistrarByHandle(automatically_managed_keyset_registrar_).exec(ctx).info_registrar_data;

        const bool automatically_managed_keyset_registrar_is_system_registrar =
                automatically_managed_keyset_registrar.system.get_value_or(false);

        if (!automatically_managed_keyset_registrar_is_system_registrar)
        {
            LOGGER(PACKAGE).debug("configuration error: automatically_managed_keyset_registrar is not system registrar");
            throw Fred::AutomaticKeysetManagement::ConfigurationError();
        }

        try
        {
            get_id_of_registered<Object_Type::contact>(ctx, automatically_managed_keyset_tech_contact_);
        }
        catch (const ObjectNotFound&)
        {
            LOGGER(PACKAGE).error(std::string("registrar: '") + automatically_managed_keyset_tech_contact_ + "' not found");
            throw Fred::AutomaticKeysetManagement::ConfigurationError();
        }

        check_configuration_of_automatically_managed_keyset_prefix(automatically_managed_keyset_prefix_);

        const LoggerRequestAkmTurnOn logger_request(logger_client_, ObjectIdHandlePair(info_domain_data.id, info_domain_data.fqdn));
        try
        {
            const ObjectIdHandlePair automatically_managed_keyset = create_automatically_managed_keyset(
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
                    ObjectIdHandlePair(info_domain_data.id, info_domain_data.fqdn),
                    automatically_managed_keyset,
                    _new_keyset);
        }
        catch (...)
        {
            logger_request.close_on_failure();
            throw;
        }

    }
    catch (const Fred::InfoDomainById::Exception& e)
    {
        if (e.is_set_unknown_object_id())
        {
            throw Fred::AutomaticKeysetManagement::ObjectNotFound();
        }
        throw;
    }
    catch (const Fred::InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER(PACKAGE).error(std::string("registrar: '") + automatically_managed_keyset_registrar_ + "' not found");
            throw Fred::AutomaticKeysetManagement::ConfigurationError();
        }
        throw;
    }
    catch (const Fred::CreateKeyset::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER(PACKAGE).warning("unknown registrar handle");
            throw ConfigurationError();
        }
        if (e.is_set_vector_of_already_set_dns_key())
        {
            LOGGER(PACKAGE).warning("duplicate dns key");
        }
        if (e.is_set_vector_of_unknown_technical_contact_handle())
        {
            LOGGER(PACKAGE).warning("unknown technical contact handle");
            throw ConfigurationError();
        }
        if (e.is_set_vector_of_already_set_technical_contact_handle())
        {
            LOGGER(PACKAGE).warning("duplicate technical contact handle");
        }
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown error");
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
        Fred::OperationContextCreator ctx;
        LOGGER(PACKAGE).debug(boost::str(boost::format("domain_id: %1% new_keyset: %2%\n")
                        % _domain_id
                        % _new_keyset.dns_keys.size()));

        const Fred::InfoDomainData info_domain_data =
                Fred::InfoDomainById(_domain_id).exec(ctx, "UTC").info_domain_data;

        const bool domain_zone_is_automatically_managed =
                automatically_managed_keyset_zones_.find(info_domain_data.zone.handle) !=
                automatically_managed_keyset_zones_.end();
        if (!domain_zone_is_automatically_managed)
        {
            throw Fred::AutomaticKeysetManagement::ObjectNotFound();
        }

        if (info_domain_data.keyset.isnull())
        {
            LOGGER(PACKAGE).debug("domain does not have a keyset (is not secure)");
            throw Fred::AutomaticKeysetManagement::DomainDoesNotHaveKeyset();
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
            LOGGER(PACKAGE).debug("domain already has an automatically managed keyset");
            throw Fred::AutomaticKeysetManagement::DomainAlreadyHasAutomaticallyManagedKeyset();
        }

        if (!is_keyset_size_within_limits(_new_keyset))
        {
            LOGGER(PACKAGE).debug("keyset invalid: incorrect number of dns_keys");
            throw Fred::AutomaticKeysetManagement::KeysetIsInvalid();
        }

        const bool turn_dnssec_off = is_dnssec_turn_off_requested(_new_keyset);
        if (!turn_dnssec_off && !has_valid_dnskeys(ctx, _new_keyset))
        {
            LOGGER(PACKAGE).debug("keyset invalid: incorrect keys");
            throw Fred::AutomaticKeysetManagement::KeysetIsInvalid();
        }

        Fred::LockObjectStateRequestLock(info_domain_data.id).exec(ctx);
        // process object state requests
        Fred::PerformObjectStateRequest(info_domain_data.id).exec(ctx);
        const Fred::ObjectStatesInfo domain_states(Fred::GetObjectStates(info_domain_data.id).exec(ctx));

        if (domain_states.presents(Fred::Object_State::server_blocked) ||
            domain_states.presents(Fred::Object_State::server_update_prohibited) ||
            domain_states.presents(Fred::Object_State::delete_candidate))
        {
            LOGGER(PACKAGE).debug("domain state prohibits action");
            throw Fred::AutomaticKeysetManagement::DomainStatePolicyError();
        }

        const Fred::InfoRegistrarData automatically_managed_keyset_registrar =
            Fred::InfoRegistrarByHandle(automatically_managed_keyset_registrar_).exec(ctx).info_registrar_data;

        const bool automatically_managed_keyset_registrar_is_system_registrar =
                automatically_managed_keyset_registrar.system.get_value_or(false);

        if (!automatically_managed_keyset_registrar_is_system_registrar)
        {
            LOGGER(PACKAGE).debug("configuration error: automatically_managed_keyset_registrar is not system registrar");
            throw Fred::AutomaticKeysetManagement::ConfigurationError();
        }

        try
        {
            get_id_of_registered<Object_Type::contact>(ctx, automatically_managed_keyset_tech_contact_);
        }
        catch (const ObjectNotFound&)
        {
            LOGGER(PACKAGE).error(std::string("registrar: '") + automatically_managed_keyset_tech_contact_ + "' not found");
            throw Fred::AutomaticKeysetManagement::ConfigurationError();
        }

        if (turn_dnssec_off)
        {
            const LoggerRequestAkmTurnOff logger_request(logger_client_, ObjectIdHandlePair(info_domain_data.id, info_domain_data.fqdn));
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
                        ObjectIdHandlePair(info_domain_data.id, info_domain_data.fqdn),
                        info_domain_data.keyset.get_value());
            }
            catch (...)
            {
                logger_request.close_on_failure();
                throw;
            }
        }
        else
        {
            const LoggerRequestAkmTurnOn logger_request(logger_client_, ObjectIdHandlePair(info_domain_data.id, info_domain_data.fqdn));
            try
            {
                const ObjectIdHandlePair automatically_managed_keyset = create_automatically_managed_keyset(
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
                        ObjectIdHandlePair(info_domain_data.id, info_domain_data.fqdn),
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
    catch (const Fred::InfoDomainById::Exception& e)
    {
        if (e.is_set_unknown_object_id())
        {
            throw Fred::AutomaticKeysetManagement::ObjectNotFound();
        }
        throw;
    }
    catch (const Fred::InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER(PACKAGE).error(std::string("registrar: '") + automatically_managed_keyset_registrar_ + "' not found");
            throw Fred::AutomaticKeysetManagement::ConfigurationError();
        }
        throw;
    }
    catch (const Fred::CreateKeyset::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER(PACKAGE).warning("unknown registrar handle");
            throw ConfigurationError();
        }
        if (e.is_set_vector_of_already_set_dns_key())
        {
            LOGGER(PACKAGE).warning("duplicate dns key");
        }
        if (e.is_set_vector_of_unknown_technical_contact_handle())
        {
            LOGGER(PACKAGE).warning("unknown technical contact handle");
            throw ConfigurationError();
        }
        if (e.is_set_vector_of_already_set_technical_contact_handle())
        {
            LOGGER(PACKAGE).warning("duplicate technical contact handle");
        }
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown error");
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
        Fred::OperationContextCreator ctx;
        LOGGER(PACKAGE).debug(boost::str(boost::format("domain_id: %1% new_keyset: %2%\n")
                        % _domain_id
                        % _new_keyset.dns_keys.size()));

        const Fred::InfoDomainData info_domain_data =
                Fred::InfoDomainById(_domain_id).exec(ctx, "UTC").info_domain_data;

        const bool domain_zone_is_automatically_managed =
                automatically_managed_keyset_zones_.find(info_domain_data.zone.handle) !=
                automatically_managed_keyset_zones_.end();
        if (!domain_zone_is_automatically_managed)
        {
            throw Fred::AutomaticKeysetManagement::ObjectNotFound();
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
            LOGGER(PACKAGE).debug("domain does not have an automatically managed keyset");
            throw Fred::AutomaticKeysetManagement::DomainDoesNotHaveAutomaticallyManagedKeyset();
        }

        if (!is_keyset_size_within_limits(_new_keyset))
        {
            LOGGER(PACKAGE).debug("keyset invalid: incorrect number of dns_keys");
            throw Fred::AutomaticKeysetManagement::KeysetIsInvalid();
        }

        const bool turn_dnssec_off = is_dnssec_turn_off_requested(_new_keyset);
        if (!turn_dnssec_off && !has_valid_dnskeys(ctx, _new_keyset))
        {
            LOGGER(PACKAGE).debug("keyset invalid: incorrect keys");
            throw Fred::AutomaticKeysetManagement::KeysetIsInvalid();
        }

        const Fred::InfoRegistrarData automatically_managed_keyset_registrar =
            Fred::InfoRegistrarByHandle(automatically_managed_keyset_registrar_).exec(ctx).info_registrar_data;

        const bool automatically_managed_keyset_registrar_is_system_registrar =
                automatically_managed_keyset_registrar.system.get_value_or(false);

        if (!automatically_managed_keyset_registrar_is_system_registrar)
        {
            LOGGER(PACKAGE).debug("configuration error: automatically_managed_keyset_registrar is not system registrar");
            throw Fred::AutomaticKeysetManagement::ConfigurationError();
        }

        if (turn_dnssec_off)
        {
            Fred::LockObjectStateRequestLock(info_domain_data.id).exec(ctx);
            // process object state requests
            Fred::PerformObjectStateRequest(info_domain_data.id).exec(ctx);
            const Fred::ObjectStatesInfo domain_states(Fred::GetObjectStates(info_domain_data.id).exec(ctx));

            if (domain_states.presents(Fred::Object_State::server_blocked) ||
                domain_states.presents(Fred::Object_State::server_update_prohibited) ||
                domain_states.presents(Fred::Object_State::delete_candidate))
            {
                LOGGER(PACKAGE).debug("domain state prohibits action");
                throw Fred::AutomaticKeysetManagement::DomainStatePolicyError();
            }

            const LoggerRequestAkmTurnOff logger_request(logger_client_, ObjectIdHandlePair(info_domain_data.id, info_domain_data.fqdn));
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
                        ObjectIdHandlePair(info_domain_data.id, info_domain_data.fqdn),
                        info_domain_data.keyset.get_value());
            }
            catch (...)
            {
                logger_request.close_on_failure();
                throw;
            }
        }
        else
        {
            const Fred::InfoKeysetData info_keyset_data =
                    Fred::InfoKeysetById(info_domain_data.keyset.get_value().id).exec(ctx, "UTC").info_keyset_data;

            if (are_keysets_equal(_new_keyset, info_keyset_data.dns_keys))
            {
                LOGGER(PACKAGE).debug("new keyset same as current keyset, nothing to do");
                // nothing to commit
                return;
            }

            Fred::LockObjectStateRequestLock(info_keyset_data.id).exec(ctx);
            // process object state requests
            Fred::PerformObjectStateRequest(info_keyset_data.id).exec(ctx);
            const Fred::ObjectStatesInfo keyset_states(Fred::GetObjectStates(info_keyset_data.id).exec(ctx));

            if (keyset_states.presents(Fred::Object_State::server_update_prohibited) ||
                keyset_states.presents(Fred::Object_State::delete_candidate))
            {
                LOGGER(PACKAGE).debug("keyset state prohibits action");
                throw Fred::AutomaticKeysetManagement::KeysetStatePolicyError();
            }

            const LoggerRequestAkmRollover logger_request(logger_client_, ObjectIdHandlePair(info_domain_data.id, info_domain_data.fqdn));
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
                        ObjectIdHandlePair(info_domain_data.id, info_domain_data.fqdn),
                        info_domain_data.keyset.get_value(),
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
    catch (const Fred::InfoDomainById::Exception& e)
    {
        if (e.is_set_unknown_object_id())
        {
            throw Fred::AutomaticKeysetManagement::ObjectNotFound();
        }
        throw;
    }
    catch (const Fred::InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER(PACKAGE).error(std::string("registrar: '") + automatically_managed_keyset_registrar_ + "' not found");
            throw Fred::AutomaticKeysetManagement::ConfigurationError();
        }
        throw;
    }
    catch (const Fred::UpdateKeyset::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_keyset_handle())
        {
            LOGGER(PACKAGE).warning("unknown keyset handle");
        }
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER(PACKAGE).warning("unknown registrar handle");
            throw ConfigurationError();
        }
        if (e.is_set_vector_of_already_set_dns_key())
        {
            LOGGER(PACKAGE).warning("duplicate dns key");
        }
        if (e.is_set_vector_of_unassigned_dns_key())
        {
            LOGGER(PACKAGE).warning("unassigned dns key");
        }
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown error");
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
        Fred::OperationContextCreator ctx;

        {
            const std::string sql =
                    "SELECT 1 FROM domain WHERE id = $1::bigint";
            const Database::Result db_result =
                    ctx.get_conn().exec_params(
                            sql,
                            Database::query_param_list(_domain_id));
            if (db_result.size() < 1)
            {
                throw Fred::AutomaticKeysetManagement::ObjectNotFound();
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

        LOGGER(PACKAGE).debug(boost::str(boost::format("found %d email(s)") % db_result.size()));

        for (unsigned int idx = 0; idx < db_result.size(); ++idx)
        {
            const std::string email_address = static_cast<std::string>(db_result[idx]["email_address"]);
            email_addresses.insert(email_address);
            LOGGER(PACKAGE).debug(std::string("email: ") + email_address);
        }

        return email_addresses;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown error");
        throw;
    }
}

bool operator<(const Domain& _lhs, const Domain& _rhs)
{
    if (_lhs.id != _rhs.id) {
        return _lhs.id < _rhs.id;
    }
    return _lhs.fqdn < _rhs.fqdn;
}

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred
