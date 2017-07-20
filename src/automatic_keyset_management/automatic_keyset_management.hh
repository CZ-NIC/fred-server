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

#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace Fred {
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
    const char* what() const throw ()
    {
        return "registry object with specified ID does not exist";
    }
};

struct NssetIsEmpty
    : std::exception
{
    const char* what() const throw ()
    {
        return "current_nsset is empty";
    }
};

struct DomainNssetIsEmpty
    : std::exception
{
    const char* what() const throw ()
    {
        return "domain nsset is empty";
    }
};

struct KeysetIsInvalid
    : std::exception
{
    const char* what() const throw ()
    {
        return "current_keyset is invalid";
    }
};

struct NssetIsDifferent
    : std::exception
{
    const char* what() const throw ()
    {
        return "current_nsset differs";
    }
};

struct DomainHasOtherKeyset
    : std::exception
{
    const char* what() const throw ()
    {
        return "domain has other keyset cannot manage automatically";
    }
};

struct DomainStatePolicyError
    : std::exception
{
    const char* what() const throw ()
    {
        return "domain state prevents action";
    }
};

struct KeysetStatePolicyError
    : std::exception
{
    const char* what() const throw ()
    {
        return "keyset state prevents action";
    }
};

struct ConfigurationError
    : std::exception
{
    const char* what() const throw ()
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
    const char* what() const throw ()
    {
        return "internal server error";
    }
};

typedef std::set<std::string> Nameservers;

struct Nsset {
    Nameservers nameservers;
};

struct DnsKey {
    unsigned short flags;
    unsigned short protocol;
    unsigned short alg;
    std::string key;

    DnsKey(
            unsigned short _flags,
            unsigned short _protocol,
            unsigned short _alg,
            const std::string& _key)
        : flags(_flags),
          protocol(_protocol),
          alg(_alg),
          key(_key)
    {
    }

    /**
     * Comparison of instances converted to std::string
     * @param rhs is right hand side instance of the comparison
     */
    bool operator<(const DnsKey& rhs) const
    {
        return key < rhs.key;
    }

    bool operator==(const DnsKey& rhs) const
    {
        return flags == rhs.flags &&
               protocol == rhs.protocol &&
               alg == rhs.alg &&
               key == rhs.key;
    }
};

typedef std::set<DnsKey> DnsKeys;

struct Keyset {
    DnsKeys dns_keys;
};

struct Domain {
    Domain(const unsigned long long _id, const std::string& _fqdn)
        : id(_id), fqdn(_fqdn)
    {
    }
    unsigned long long id;
    std::string fqdn;
    friend bool operator < (const Domain& _lhs, const Domain& _rhs);
};

typedef std::string Nameserver;
typedef std::set<Domain> Domains;
typedef std::map<Nameserver, Domains> NameserversDomains;
typedef std::vector<std::string> EmailAddresses;

class AutomaticKeysetManagementImpl
{
public:
    AutomaticKeysetManagementImpl(
            const std::string& _server_name,
            const std::string& _automatically_managed_keyset_prefix,
            const std::string& _automatically_managed_keyset_registrar,
            const std::string& _automatically_managed_keyset_tech_contact,
            const std::vector<std::string>& _automatically_managed_keyset_zones,
            bool _disable_notifier);

    virtual ~AutomaticKeysetManagementImpl();

    /**
     * Get server name
     * @return name for logging context
     */
    std::string get_server_name() const;

    NameserversDomains get_nameservers_with_automatically_managed_domain_candidates();

    NameserversDomains get_nameservers_with_automatically_managed_domains();

    void update_domain_automatic_keyset(
            unsigned long long _domain_id,
            const Nsset& _current_nsset,
            const Keyset& _new_keyset);

    EmailAddresses get_email_addresses_by_domain_id(
            unsigned long long _domain_id);

private:
    std::string server_name_;
    std::string automatically_managed_keyset_prefix_;
    std::string automatically_managed_keyset_registrar_;
    std::string automatically_managed_keyset_tech_contact_;
    std::vector<std::string> automatically_managed_keyset_zones_;
    bool disable_notifier_;
};

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred

#endif
