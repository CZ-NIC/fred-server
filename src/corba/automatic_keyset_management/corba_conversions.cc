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
 * along with FRED.  If not, see <http://www.gnu.or/licenses/>.
 */

#include "src/corba/automatic_keyset_management/corba_conversions.hh"

#include "src/automatic_keyset_management/automatic_keyset_management.hh"
#include "src/corba/util/corba_conversions_int.h"
#include "src/corba/util/corba_conversions_string.h"

#include <string>

namespace Fred {
namespace Corba {
namespace AutomaticKeysetManagement {

Registry::AutomaticKeysetManagement::DomainSeq_var wrap_Domains(const Fred::AutomaticKeysetManagement::Domains& domains)
{
    Registry::AutomaticKeysetManagement::DomainSeq_var result(new Registry::AutomaticKeysetManagement::DomainSeq());
    result->length(domains.size());

    CORBA::ULong i = 0;
    for (Fred::AutomaticKeysetManagement::Domains::const_iterator domain = domains.begin();
         domain != domains.end();
         ++domain, ++i)
    {
        Fred::Corba::wrap_int(domain->id, result[i].id);
        result[i].fqdn = ::Corba::wrap_string_to_corba_string(domain->fqdn);
    }

    return result;
}

Registry::AutomaticKeysetManagement::NameserverDomainsSeq_var wrap_NameserversDomains(
        const Fred::AutomaticKeysetManagement::NameserversDomains& nameservers_domains)
{
    Registry::AutomaticKeysetManagement::NameserverDomainsSeq_var result(new Registry::AutomaticKeysetManagement::NameserverDomainsSeq());
    result->length(nameservers_domains.size());

    CORBA::ULong i = 0;
    for (Fred::AutomaticKeysetManagement::NameserversDomains::const_iterator nameserver_domains =
             nameservers_domains.begin();
         nameserver_domains != nameservers_domains.end();
         ++nameserver_domains, ++i)
    {
        result[i].nameserver = ::Corba::wrap_string_to_corba_string(nameserver_domains->first);
        result[i].nameserver_domains = wrap_Domains(nameserver_domains->second);
    }

    return result;
}

Registry::AutomaticKeysetManagement::EmailAddressSeq_var wrap_EmailAddresses(const Fred::AutomaticKeysetManagement::EmailAddresses& email_addresses)
{
    Registry::AutomaticKeysetManagement::EmailAddressSeq_var result(new Registry::AutomaticKeysetManagement::EmailAddressSeq());
    result->length(email_addresses.size());

    CORBA::ULong i = 0;
    for (Fred::AutomaticKeysetManagement::EmailAddresses::const_iterator email_address =
             email_addresses.begin();
         email_address != email_addresses.end();
         ++email_address, ++i)
    {
        result[i] = ::Corba::wrap_string_to_corba_string(*email_address);
    }

    return result;
}

Fred::AutomaticKeysetManagement::Nsset unwrap_Nsset(const Registry::AutomaticKeysetManagement::Nsset& nsset)
{
    Fred::AutomaticKeysetManagement::Nsset result;
    for (CORBA::ULong i = 0; i < nsset.nameservers.length(); ++i)
    {
        result.nameservers.insert(::Corba::unwrap_string_from_const_char_ptr(nsset.nameservers[i]));
    }
    return result;
}

Fred::AutomaticKeysetManagement::DnsKey unwrap_DnsKey(const Registry::AutomaticKeysetManagement::DnsKey& dns_key)
{
    unsigned short flags;
    Fred::Corba::unwrap_int(dns_key.flags, flags);
    unsigned short protocol;
    Fred::Corba::unwrap_int(dns_key.protocol, protocol);
    unsigned short alg;
    Fred::Corba::unwrap_int(dns_key.alg, alg);
    const std::string key = ::Corba::unwrap_string(dns_key.public_key);

    return Fred::AutomaticKeysetManagement::DnsKey(
            flags,
            protocol,
            alg,
            key);
}

Fred::AutomaticKeysetManagement::DnsKeys unwrap_DnsKeys(const Registry::AutomaticKeysetManagement::DnsKeySeq& dns_key_seq)
{
    Fred::AutomaticKeysetManagement::DnsKeys result;
    for (CORBA::ULong i = 0; i < dns_key_seq.length(); ++i)
    {
        result.insert(unwrap_DnsKey(dns_key_seq[i]));
    }
    return result;
}

Fred::AutomaticKeysetManagement::Keyset unwrap_Keyset(const Registry::AutomaticKeysetManagement::Keyset& keyset)
{
    Fred::AutomaticKeysetManagement::Keyset result;
    result.dns_keys = unwrap_DnsKeys(keyset.dns_keys);
    return result;
}

} // namespace Fred::Corba::AutomaticKeysetManagement
} // namespace Fred::Corba
} // namespace Corba
