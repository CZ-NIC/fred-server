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
    : virtual std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    virtual const char* what() const throw ()
    {
        return "registry object with specified ID does not exist";
    }
};

struct NssetInvalid
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "current_nsset invalid";
    }
};

struct KeysetInvalid
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "current_keyset invalid";
    }
};

struct NssetDiffers
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "current_nsset differs";
    }
};

struct DomainHasOtherKeyset
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "domain has other keyset cannot manage automatically";
    }
};

struct DomainStatePolicyError
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "domain state prevents action";
    }
};

struct SystemRegistratorNotFound
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "system registrator not found";
    }
};

struct ConfigurationError
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "configuration error";
    }
};

/**
 * Internal server error.
 * Unexpected failure, requires maintenance.
 */
struct InternalServerError
    : virtual std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    virtual const char* what() const throw ()
    {
        return "internal server error";
    }
};

typedef std::vector<std::string> Nameservers;

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
};

typedef std::vector<DnsKey> DnsKeys;

struct Keyset {
    DnsKeys dns_keys;
};

struct Domain {
    Domain(unsigned long long _id, std::string& _fqdn)
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
typedef std::vector<std::string> TechContacts;

class AutomaticKeysetManagementImpl
{
private:
    std::string server_name_;
    std::string automatically_managed_keyset_prefix_;
    std::string automatically_managed_keyset_registrar_;
    std::string automatically_managed_keyset_tech_contact_;

public:
    AutomaticKeysetManagementImpl(
            const std::string& _server_name,
            const std::string& _automatically_managed_keyset_prefix,
            const std::string& _automatically_managed_keyset_registrar,
            const std::string& _automatically_managed_keyset_tech_contact);

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
            Nsset _current_nsset,
            Keyset _new_keyset);

    TechContacts get_nsset_notification_emails_by_domain_id(
            unsigned long long _domain_id);

};

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred

#endif
