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
 *  @file automatic_keyset_management.hh
 *  header of automatic keyset management implementation
 */

#ifndef AUTOMATIC_KEYSET_MANAGEMENT_HH_E7D0CA5C7FDA4FF6A7217BE8252D99A1
#define AUTOMATIC_KEYSET_MANAGEMENT_HH_E7D0CA5C7FDA4FF6A7217BE8252D99A1

#include "src/deprecated/libfred/logger_client.hh"
#include "src/backend/automatic_keyset_management/keyset.hh"
#include "src/backend/automatic_keyset_management/nsset.hh"

#include <exception>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace LibFred {
namespace AutomaticKeysetManagement {

/**
 * Requested object was not found.
 * Requested object could have been deleted or set into inappropriate state.
 */
struct ObjectNotFound
    : std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    const char* what() const noexcept
    {
        return "registry object with specified ID does not exist";
    }
};

struct NssetIsEmpty
    : std::exception
{
    const char* what() const noexcept
    {
        return "current_nsset is empty";
    }
};

struct DomainNssetIsEmpty
    : std::exception
{
    const char* what() const noexcept
    {
        return "domain nsset is empty";
    }
};

struct KeysetIsInvalid
    : std::exception
{
    const char* what() const noexcept
    {
        return "current_keyset is invalid";
    }
};

struct NssetIsDifferent
    : std::exception
{
    const char* what() const noexcept
    {
        return "current_nsset differs";
    }
};

struct DomainHasKeyset
    : std::exception
{
    const char* what() const noexcept
    {
        return "domain has keyset (domain is not insecure)";
    }
};

struct DomainDoesNotHaveKeyset
    : std::exception
{
    const char* what() const noexcept
    {
        return "domain does not have a keyset (domain is not secure)";
    }
};

struct DomainAlreadyDoesNotHaveKeyset
    : std::exception
{
    const char* what() const noexcept
    {
        return "domain already does not have a keyset";
    }
};

struct DomainDoesNotHaveAutomaticallyManagedKeyset
    : std::exception
{
    const char* what() const noexcept
    {
        return "domain does not have an automatically managed keyset";
    }
};

struct DomainAlreadyHasAutomaticallyManagedKeyset
    : std::exception
{
    const char* what() const noexcept
    {
        return "domain already has an automatically managed keyset";
    }
};

struct KeysetSameAsDomainKeyset
    : std::exception
{
    const char* what() const noexcept
    {
        return "the keyset is the same as the current keyset of the domain";
    }
};

struct DomainStatePolicyError
    : std::exception
{
    const char* what() const noexcept
    {
        return "domain state prevents action";
    }
};

struct KeysetStatePolicyError
    : std::exception
{
    const char* what() const noexcept
    {
        return "keyset state prevents action";
    }
};

struct ConfigurationError
    : std::exception
{
    const char* what() const noexcept
    {
        return "configuration error";
    }
};

/**
 * Internal server error.
 * Unexpected failure, requires maintenance.
 */
struct InternalServerError
    : std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    const char* what() const noexcept
    {
        return "internal server error";
    }
};

struct Domain
{
    Domain(const unsigned long long _id, const std::string& _fqdn)
        : id(_id), fqdn(_fqdn)
    {
    }

    friend bool operator<(const Domain& lhs, const Domain& rhs);

    unsigned long long id;
    std::string fqdn;
};

typedef std::string Nameserver;
typedef std::set<Domain> Domains;
typedef std::map<Nameserver, Domains> NameserversDomains;
typedef std::set<std::string> EmailAddresses;

class AutomaticKeysetManagementImpl
{
public:
    AutomaticKeysetManagementImpl(
            const std::string& _server_name,
            const std::string& _automatically_managed_keyset_prefix,
            const std::string& _automatically_managed_keyset_registrar,
            const std::string& _automatically_managed_keyset_tech_contact,
            const std::set<std::string>& _automatically_managed_keyset_zones,
            bool _disable_notifier,
            LibFred::Logger::LoggerClient& _logger_client);

    virtual ~AutomaticKeysetManagementImpl();

    /**
     * Get server name
     * @return name for logging context
     */
    std::string get_server_name() const;

    NameserversDomains get_nameservers_with_insecure_automatically_managed_domain_candidates();

    NameserversDomains get_nameservers_with_secure_automatically_managed_domain_candidates();

    NameserversDomains get_nameservers_with_automatically_managed_domains();

    void update_automatically_managed_keyset_of_domain_impl(
            unsigned long long _domain_id,
            const Nsset& _current_nsset,
            const Keyset& _new_keyset);

    void turn_on_automatic_keyset_management_on_insecure_domain(
            unsigned long long _domain_id,
            const Nsset& _current_nsset,
            const Keyset& _new_keyset);

    void turn_on_automatic_keyset_management_on_secure_domain(
            unsigned long long _domain_id,
            const Keyset& _new_keyset);

    void update_automatically_managed_keyset_of_domain(
            unsigned long long _domain_id,
            const Keyset& _new_keyset);

    EmailAddresses get_email_addresses_by_domain_id(
            unsigned long long _domain_id);

private:
    std::string server_name_;
    std::string automatically_managed_keyset_prefix_;
    std::string automatically_managed_keyset_registrar_;
    std::string automatically_managed_keyset_tech_contact_;
    std::set<std::string> automatically_managed_keyset_zones_;
    bool notifier_disabled_;
    LibFred::Logger::LoggerClient& logger_client_;

};

} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

#endif
