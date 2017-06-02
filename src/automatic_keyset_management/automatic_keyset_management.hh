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

struct DomainHasOtherKeysetCannotManageAutomatically
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "domain has other keyset cannot manage automatically";
    }
};

struct CurrentNssetDiffers
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "current_nsset differs";
    }
};

struct NewNssetNotAcceptedBreaksContinuity
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "new_nsset not accepted, breaks continuity";
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

class AutomaticKeysetManagementImpl
{
private:
    std::string server_name_;
    std::string automatically_managed_keyset_prefix_;
    std::string automatically_managed_keyset_registrar_;
    std::string automatically_managed_keyset_tech_contact_;

public:
    AutomaticKeysetManagementImpl(
            const std::string& server_name,
            const std::string& automatically_managed_keyset_prefix_,
            const std::string& automatically_managed_keyset_registrar_,
            const std::string& automatically_managed_keyset_tech_contact_);

    virtual ~AutomaticKeysetManagementImpl();

    /**
     * Get server name
     * @return name for logging context
     */
    std::string get_server_name() const;

    NameserversDomains get_nameservers_with_automatically_managed_domain_candidates();

    NameserversDomains get_nameservers_with_automatically_managed_domains();

    void domain_automatic_keyset_update(
            unsigned long long domain_id,
            Nsset current_nsset,
            Keyset new_keyset);
};

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred

#endif
