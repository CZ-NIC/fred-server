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
#include "src/bin/corba/automatic_keyset_management/corba_conversions.hh"

#include "src/backend/automatic_keyset_management/automatic_keyset_management.hh"
#include "src/bin/corba/util/corba_conversions_int.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"

#include <string>

namespace CorbaConversions {

Registry::AutomaticKeysetManagement::DomainSeq_var wrap_Domains(const Fred::Backend::AutomaticKeysetManagement::Domains& domains)
{
    Registry::AutomaticKeysetManagement::DomainSeq_var result(new Registry::AutomaticKeysetManagement::DomainSeq());
    result->length(domains.size());

    CORBA::ULong i = 0;
    for (const auto& domain : domains)
    {
        LibFred::Corba::wrap_int(domain.id, result[i].id);
        result[i].fqdn = LibFred::Corba::wrap_string_to_corba_string(domain.fqdn);
        ++i;
    }

    return result;
}

Registry::AutomaticKeysetManagement::NameserverDomainsSeq_var wrap_NameserversDomains(
        const Fred::Backend::AutomaticKeysetManagement::NameserversDomains& nameservers_domains)
{
    Registry::AutomaticKeysetManagement::NameserverDomainsSeq_var result(new Registry::AutomaticKeysetManagement::NameserverDomainsSeq());
    result->length(nameservers_domains.size());

    CORBA::ULong i = 0;
    for (const auto& nameserver_domains : nameservers_domains)
    {
        result[i].nameserver = LibFred::Corba::wrap_string_to_corba_string(nameserver_domains.first);
        result[i].nameserver_domains = wrap_Domains(nameserver_domains.second);
        ++i;
    }

    return result;
}

Registry::AutomaticKeysetManagement::EmailAddressSeq_var wrap_EmailAddresses(const Fred::Backend::AutomaticKeysetManagement::EmailAddresses& email_addresses)
{
    Registry::AutomaticKeysetManagement::EmailAddressSeq_var result(new Registry::AutomaticKeysetManagement::EmailAddressSeq());
    result->length(email_addresses.size());

    CORBA::ULong i = 0;
    for (const auto& email_address : email_addresses)
    {
        result[i] = LibFred::Corba::wrap_string_to_corba_string(email_address);
        ++i;
    }

    return result;
}

Fred::Backend::AutomaticKeysetManagement::Nsset unwrap_Nsset(const Registry::AutomaticKeysetManagement::Nsset& nsset)
{
    Fred::Backend::AutomaticKeysetManagement::Nsset result;
    for (CORBA::ULong i = 0; i < nsset.nameservers.length(); ++i)
    {
        result.nameservers.insert(LibFred::Corba::unwrap_string_from_const_char_ptr(nsset.nameservers[i]));
    }
    return result;
}

Fred::Backend::AutomaticKeysetManagement::DnsKey unwrap_DnsKey(const Registry::AutomaticKeysetManagement::DnsKey& dns_key)
{
    unsigned short flags;
    LibFred::Corba::unwrap_int(dns_key.flags, flags);
    unsigned short protocol;
    LibFred::Corba::unwrap_int(dns_key.protocol, protocol);
    unsigned short alg;
    LibFred::Corba::unwrap_int(dns_key.alg, alg);
    const std::string key = LibFred::Corba::unwrap_string(dns_key.public_key);

    return Fred::Backend::AutomaticKeysetManagement::DnsKey(
            flags,
            protocol,
            alg,
            key);
}

Fred::Backend::AutomaticKeysetManagement::DnsKeys unwrap_DnsKeys(const Registry::AutomaticKeysetManagement::DnsKeySeq& dns_key_seq)
{
    Fred::Backend::AutomaticKeysetManagement::DnsKeys result;
    for (CORBA::ULong i = 0; i < dns_key_seq.length(); ++i)
    {
        result.insert(unwrap_DnsKey(dns_key_seq[i]));
    }
    return result;
}

Fred::Backend::AutomaticKeysetManagement::Keyset unwrap_Keyset(const Registry::AutomaticKeysetManagement::Keyset& keyset)
{
    Fred::Backend::AutomaticKeysetManagement::Keyset result;
    result.dns_keys = unwrap_DnsKeys(keyset.dns_keys);
    return result;
}

} // namespace CorbaConversions
