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

#include "automatic_keyset_management.hh"

#include "src/automatic_keyset_management/impl/limits.hh"
#include "src/epp/keyset/dns_key.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/update_domain.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/keyset_dns_key.h"
#include "src/fredlib/keyset/update_keyset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/object/check_handle.h"
#include "src/fredlib/object/object_states_info.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/zone/zone.h"
#include "util/log/context.h"
#include "util/util.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>
#include <boost/nondet_random.hpp>
#include <boost/random/uniform_int.hpp>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace Fred {
namespace AutomaticKeysetManagement {

AutomaticKeysetManagementImpl::AutomaticKeysetManagementImpl(
        const std::string& _server_name,
        const std::string& _automatically_managed_keyset_prefix,
        const std::string& _automatically_managed_keyset_registrar,
        const std::string& _automatically_managed_keyset_tech_contact)
    : server_name_(_server_name),
      automatically_managed_keyset_prefix_(_automatically_managed_keyset_prefix),
      automatically_managed_keyset_registrar_(_automatically_managed_keyset_registrar),
      automatically_managed_keyset_tech_contact_(_automatically_managed_keyset_tech_contact)
{
}

AutomaticKeysetManagementImpl::~AutomaticKeysetManagementImpl()
{
}

std::string AutomaticKeysetManagementImpl::get_server_name() const
{
    return server_name_;
}

NameserversDomains AutomaticKeysetManagementImpl::get_nameservers_with_automatically_managed_domain_candidates() {
    try
    {
        NameserversDomains nameservers_domains;
        Fred::OperationContextCreator ctx;

        std::string sql = ""
            "SELECT ns.fqdn as nameserver, oreg.name as domain_fqdn, oreg.id as domain_id "
             "FROM host ns "
             "JOIN domain d ON ns.nssetid = d.nsset "
             "JOIN object_registry oreg ON oreg.id = d.id "
            "WHERE d.keyset IS NULL "
              "AND d.zone = 2 "
            "ORDER BY ns.fqdn";

        const Database::Result db_result = ctx.get_conn().exec(sql);

        for (unsigned int idx = 0; idx < db_result.size(); ++idx)
        {
            std::string nameserver = static_cast<std::string>(db_result[idx]["nameserver"]);
            unsigned long long domain_id = static_cast<unsigned long long>(db_result[idx]["domain_id"]);
            std::string domain_fqdn = static_cast<std::string>(db_result[idx]["domain_fqdn"]);
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

NameserversDomains AutomaticKeysetManagementImpl::get_nameservers_with_automatically_managed_domains() {
    try
    {
        NameserversDomains nameservers_domains;
        Fred::OperationContextCreator ctx;

        Database::ParamQuery sql;
        sql(""
            "SELECT ns.fqdn as nameserver, oreg.name as domain_fqdn, oreg.id as domain_id "
             "FROM host ns "
             "JOIN domain d ON ns.nssetid = d.nsset "
             "JOIN object_registry oreg ON oreg.id = d.id "
             "JOIN object k ON k.id = d.keyset "
             "JOIN object_registry oregk ON oregk.id = k.id "
            "WHERE oregk.name LIKE '")(automatically_managed_keyset_prefix_)("%' "
              "AND d.zone = 2 "
            "ORDER BY ns.fqdn");

        Database::Result db_result = ctx.get_conn().exec_params(sql);

        for (unsigned int idx = 0; idx < db_result.size(); ++idx)
        {
            std::string nameserver = static_cast<std::string>(db_result[idx]["nameserver"]);
            unsigned long long domain_id = static_cast<unsigned long long>(db_result[idx]["domain_id"]);
            std::string domain_fqdn = static_cast<std::string>(db_result[idx]["domain_fqdn"]);
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

namespace {

 // TODO order of elements...
bool are_nssets_equal(Nsset nsset, std::vector<DnsHost> dns_hosts) {
    std::vector<std::string> vector1;
    std::vector<std::string> vector2;
    for (Nameservers::const_iterator nameserver = nsset.nameservers.begin(); nameserver != nsset.nameservers.end(); ++nameserver) {
        vector1.push_back(*nameserver);
    }
    for (std::vector<DnsHost>::const_iterator dns_host = dns_hosts.begin(); dns_host != dns_hosts.end(); ++dns_host) {
        vector2.push_back(dns_host->get_fqdn());
    }
    return equal(vector1.begin(), vector1.end(), vector2.begin());
}

bool is_automatically_managed_keyset(
        Fred::OperationContext& ctx,
        unsigned long long keyset_id,
        std::string automatically_managed_keyset_registrar,
        std::string automatically_managed_keyset_tech_contact)
{
    const Fred::InfoKeysetData info_keyset_data =
            Fred::InfoKeysetById(keyset_id).exec(ctx, "UTC").info_keyset_data;

    if (info_keyset_data.sponsoring_registrar_handle != automatically_managed_keyset_registrar)
    { // TODO can change in time... test it anyway?
        return false;
    }
    for (
        std::vector<ObjectIdHandlePair>::const_iterator object_id_handle_pair =
            info_keyset_data.tech_contacts.begin();
        object_id_handle_pair != info_keyset_data.tech_contacts.end();
        ++object_id_handle_pair)
    {
        if (object_id_handle_pair->handle == automatically_managed_keyset_tech_contact) {
            return true;
        }
    }
    return false;
}

std::string generate_automatically_managed_keyset_handle(const std::string& handle_prefix) {
    static const std::size_t keyset_handle_length_max = 30;
    static const std::string alphabet = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789";
    boost::random_device rng;
    boost::uniform_int<> index_dist(0, alphabet.size() - 1);
    if (handle_prefix.length() > keyset_handle_length_max) {
        LOGGER(PACKAGE).error("automatically_managed_keyset_prefix too long");
        throw Fred::AutomaticKeysetManagement::ConfigurationError();
    }
    const int handle_length = keyset_handle_length_max - handle_prefix.length();
    std::string result = handle_prefix;
    for(int i = 0; i < handle_length; ++i) {
        result += alphabet.at(index_dist(rng));
    }
    return result;
}

boost::optional<std::string> get_system_registrar_handle(Fred::OperationContext& ctx) {
    const Database::Result db_result = ctx.get_conn().exec(
        "SELECT handle FROM registrar WHERE system is True"
    );
    return db_result.size() > 0
                   ? static_cast<std::string>(db_result[0][0])
                   : boost::optional<std::string>();
}

bool registrar_exists(Fred::OperationContext& ctx, std::string _registrar_handle) {
    const Database::Result db_result = ctx.get_conn().exec_params(
            "SELECT id FROM registrar WHERE handle = UPPER($1::text)",
            Database::query_param_list(_registrar_handle));
    return db_result.size() != 0;
}

bool is_keyset_size_within_limits(Keyset keyset) {
    if ((keyset.dns_keys.size() < min_number_of_dns_keys) ||
        (keyset.dns_keys.size() > max_number_of_dns_keys))
    {
        return false;
    }
    return true;
}


bool are_keyset_keys_valid(Fred::OperationContext& ctx, Keyset keyset) {
    Epp::Keyset::DnsKey::AlgValidator alg_validator(ctx);

    typedef std::map<Epp::Keyset::DnsKey, unsigned short> DnsKeyIndex;

    DnsKeyIndex unique_dns_keys;
    for (std::vector<DnsKey>::const_iterator dns_key = keyset.dns_keys.begin();
         dns_key != keyset.dns_keys.end(); ++dns_key)
    {
        Epp::Keyset::DnsKey epp_dns_key = Epp::Keyset::DnsKey(dns_key->flags, dns_key->protocol, dns_key->alg, dns_key->key);

        const DnsKeyIndex::const_iterator dns_key_index_ptr = unique_dns_keys.find(epp_dns_key);
        if (dns_key_index_ptr != unique_dns_keys.end())  // a duplicate dns_key
        {
            return false;
        }

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
                break;

            case Epp::Keyset::DnsKey::CheckKey::bad_length:
                return false;
                break;
        }
    }

    return true;
}

} // namespace {anonymous}

void AutomaticKeysetManagementImpl::update_domain_automatic_keyset(
        unsigned long long _domain_id,
        Nsset _current_nsset,
        Keyset _new_keyset)
{
    try
    {
        Fred::OperationContextCreator ctx;
        LOGGER(PACKAGE).debug(boost::str(boost::format("domain_id: %1% current_nsset: %2% new_keyset: %3%\n")
                        % _domain_id
                        % _current_nsset.nameservers.size()
                        % _new_keyset.dns_keys.size()));

        const Fred::InfoDomainData info_domain_data =
                Fred::InfoDomainById(_domain_id).exec(ctx, "UTC").info_domain_data;

        // check fqdn has known zone
        Fred::Zone::Data zone_data;
        zone_data = Fred::Zone::find_zone_in_fqdn(
                ctx,
                Fred::Zone::rem_trailing_dot(info_domain_data.fqdn));

        if (zone_data.is_enum)
        {
            throw std::runtime_error("will not update enum zone");
        }


        if (info_domain_data.nsset.isnull()) {
            LOGGER(PACKAGE).debug("current_nsset empty");
            throw Fred::AutomaticKeysetManagement::NssetInvalid();
        }

        if (!is_keyset_size_within_limits(_new_keyset)) {
            LOGGER(PACKAGE).debug("keyset invalid: incorrect number of dns_keys");
            throw Fred::AutomaticKeysetManagement::KeysetInvalid();
        }

        if (!are_keyset_keys_valid(ctx, _new_keyset)) {
            LOGGER(PACKAGE).debug("keyset invalid: incorrect keys");
            throw Fred::AutomaticKeysetManagement::KeysetInvalid();
        }

        const Fred::InfoNssetData info_nsset_data =
                Fred::InfoNssetById(info_domain_data.nsset.get_value().id).exec(ctx, "UTC").info_nsset_data;

        if (!are_nssets_equal(_current_nsset, info_nsset_data.dns_hosts)) {
            LOGGER(PACKAGE).debug("nsset differs");
            throw Fred::AutomaticKeysetManagement::NssetDiffers();
        }

        const bool keyset_is_automatically_managed_keyset =
                is_automatically_managed_keyset(
                        ctx,
                        info_domain_data.keyset.get_value().id,
                        automatically_managed_keyset_registrar_,
                        automatically_managed_keyset_tech_contact_);

        if (!info_domain_data.keyset.isnull() && !keyset_is_automatically_managed_keyset)
        {
            LOGGER(PACKAGE).debug("domain has other keyset");
            throw Fred::AutomaticKeysetManagement::DomainHasOtherKeyset();
        }

        if (info_domain_data.keyset.isnull())
        {
            Fred::LockObjectStateRequestLock(info_domain_data.id).exec(ctx);
            // process object state requests
            Fred::PerformObjectStateRequest(info_domain_data.id).exec(ctx);
            const Fred::ObjectStatesInfo domain_states(Fred::GetObjectStates(info_domain_data.id).exec(ctx));

            if (domain_states.presents(Fred::Object_State::server_update_prohibited) ||
                domain_states.presents(Fred::Object_State::delete_candidate))
            {
                LOGGER(PACKAGE).debug("domain state prohibits action");
                throw Fred::AutomaticKeysetManagement::DomainStatePolicyError();
            }
        }

        const Fred::InfoRegistrarData automatically_managed_keyset_registrar =
            Fred::InfoRegistrarByHandle(automatically_managed_keyset_registrar_).exec(ctx).info_registrar_data;

        const bool automatically_managed_keyset_registrar_is_system_registrar =
                automatically_managed_keyset_registrar.system.get_value_or(false);

        const boost::optional<std::string> system_registrar =
                automatically_managed_keyset_registrar_is_system_registrar
                        ? automatically_managed_keyset_registrar.handle
                        : get_system_registrar_handle(ctx);

        if (!system_registrar) {
            LOGGER(PACKAGE).warning("system registrar not found");
            throw Fred::AutomaticKeysetManagement::SystemRegistratorNotFound();
        }

        if (info_domain_data.keyset.isnull())
        {
            std::string automatically_managed_keyset_handle =
                    generate_automatically_managed_keyset_handle(automatically_managed_keyset_prefix_);
            // TODO check !exists

            if (TestHandleOf<Object_Type::keyset>(automatically_managed_keyset_handle).is_invalid_handle()) {
                throw std::runtime_error("automatically_managed_keyset_handle invalid");
            }

            if (!registrar_exists(ctx, automatically_managed_keyset_registrar_)) {
                LOGGER(PACKAGE).warning(std::string("registrar: '") + automatically_managed_keyset_registrar_ + "' not found");
                throw Fred::AutomaticKeysetManagement::ConfigurationError();
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
                            automatically_managed_keyset_registrar_,
                            Optional<std::string>(), // authinfopw
                            libfred_dns_keys,
                            Util::vector_of<std::string>(automatically_managed_keyset_tech_contact_))
                            .exec(ctx, 0, "UTC");

            LOGGER(PACKAGE).debug(boost::str(boost::format("creation_time: %1% object_id: %2% history_id: %3%\n")
                            % create_keyset_result.creation_time
                            % create_keyset_result.create_object_result.object_id
                            % create_keyset_result.create_object_result.history_id));

            const unsigned long long domain_new_history_id =
            Fred::UpdateDomain(
                    info_domain_data.fqdn,
                    *system_registrar)
                    .set_keyset(automatically_managed_keyset_handle) // or get handle by create_keyset_result.create_object_result.object_id
                    .exec(ctx);

            LOGGER(PACKAGE).debug(boost::str(boost::format("domain_new_history_id: %1%\n")
                            % domain_new_history_id));

        }
        else {
            Fred::UpdateKeyset update_keyset = Fred::UpdateKeyset(
                            info_domain_data.keyset.get_value().handle,
                            *system_registrar);

            const Fred::InfoKeysetData info_keyset_data =
                    Fred::InfoKeysetById(info_domain_data.keyset.get_value().id).exec(ctx, "UTC").info_keyset_data;

            for (std::vector<Fred::DnsKey>::const_iterator dns_key = info_keyset_data.dns_keys.begin();
                 dns_key != info_keyset_data.dns_keys.end();
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

            unsigned long long keyset_new_history_id =
            update_keyset.exec(ctx);

            LOGGER(PACKAGE).debug(boost::str(boost::format("keyset_new_history_id: %1%\n")
                            % keyset_new_history_id));
        }

        ctx.commit_transaction();

    }
    catch (const Fred::InfoDomainById::Exception& e)
    {

        if (e.is_set_unknown_object_id())
        {
            throw Fred::AutomaticKeysetManagement::ObjectNotFound();
        }
        throw;
    }
    catch (const Fred::Zone::Exception& e)
    {
        if (e.is_set_unknown_zone_in_fqdn())
        {
            LOGGER(PACKAGE).debug("domain has unknown zone");
            throw Fred::AutomaticKeysetManagement::ObjectNotFound();
        }
        throw;
    }
    catch (const Fred::CreateKeyset::Exception& e)
    {
        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_registrar_handle())
        {
            LOGGER(PACKAGE).warning("unknown registrar handle");
        }
        if (e.is_set_vector_of_already_set_dns_key())
        {
            LOGGER(PACKAGE).warning("duplicate dns key");
        }
        if (e.is_set_vector_of_unknown_technical_contact_handle())
        {
            LOGGER(PACKAGE).warning("unknown technical contact handle");
        }
        if (e.is_set_vector_of_already_set_technical_contact_handle())
        {
            LOGGER(PACKAGE).warning("duplicate technical contact handle");
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
        }
        if (e.is_set_vector_of_unknown_technical_contact_handle())
        {
            LOGGER(PACKAGE).warning("unknown technical contact handle");
        }
        if (e.is_set_vector_of_already_set_technical_contact_handle())
        {
            LOGGER(PACKAGE).warning("duplicate technical contact handle");
        }
        if (e.is_set_vector_of_unassigned_technical_contact_handle())
        {
            LOGGER(PACKAGE).warning("unassigned technical contact handle");
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

TechContacts AutomaticKeysetManagementImpl::get_domain_nsset_tech_contacts(
        unsigned long long _domain_id)
{
    try
    {
        TechContacts tech_contacts;
        Fred::OperationContextCreator ctx;

        std::string sql = ""
            "SELECT c.email as tech_contact "
            "FROM domain d "
            "JOIN nsset n ON d.nsset = n.id "
            "JOIN nsset_contact_map ncmap ON ncmap.nssetid = n.id "
            "JOIN contact c on ncmap.contactid = c.id "
            "WHERE d.id = $1::bigint";

        const Database::Result db_result = ctx.get_conn().exec_params(
                sql,
                Database::query_param_list(_domain_id));

        for (unsigned int idx = 0; idx < db_result.size(); ++idx)
        {
            std::string tech_contact = static_cast<std::string>(db_result[idx]["tech_contact"]);
            tech_contacts.push_back(tech_contact);
        }

        return tech_contacts;
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

bool operator < (const Domain& _lhs, const Domain& _rhs)
{
    return (_lhs.id < _rhs.id);
}

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred
