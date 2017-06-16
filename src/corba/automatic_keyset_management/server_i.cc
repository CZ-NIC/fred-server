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
 *  @file
 *  automatic keyset management corba implementation
 */

//pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace_18680/enum/idl/idl/AutomaticKeysetManagement.idl

#include "src/corba/automatic_keyset_management/server_i.hh"

#include "src/automatic_keyset_management/automatic_keyset_management.hh"
#include "src/corba/AutomaticKeysetManagement.hh"
#include "src/corba/util/corba_conversions_string.h"
#include "src/corba/util/corba_conversions_int.h"

#include <iostream>

namespace Registry {
namespace AutomaticKeysetManagement {

namespace {

DomainSeq_var wrap_Domains(const Fred::AutomaticKeysetManagement::Domains& domains)
{
    DomainSeq_var result(new DomainSeq());
    result->length(domains.size());

    CORBA::ULong i = 0;
    for (Fred::AutomaticKeysetManagement::Domains::const_iterator domain = domains.begin();
         domain != domains.end();
         ++domain, ++i)
    {
        Fred::Corba::wrap_int(domain->id, result[i].id);
        result[i].fqdn = Corba::wrap_string_to_corba_string(domain->fqdn);
    }

    return result;
}

NameserverDomainsSeq_var wrap_NameserversDomains(
        const Fred::AutomaticKeysetManagement::NameserversDomains& nameservers_domains)
{
    NameserverDomainsSeq_var result(new NameserverDomainsSeq());
    result->length(nameservers_domains.size());

    CORBA::ULong i = 0;
    for (Fred::AutomaticKeysetManagement::NameserversDomains::const_iterator nameserver_domains =
             nameservers_domains.begin();
         nameserver_domains != nameservers_domains.end();
         ++nameserver_domains, ++i)
    {
        result[i].nameserver = Corba::wrap_string_to_corba_string(nameserver_domains->first);
        result[i].nameserver_domains = wrap_Domains(nameserver_domains->second);
    }

    return result;
}

TechContactSeq_var wrap_TechContacts(const Fred::AutomaticKeysetManagement::TechContacts& tech_contacts)
{
    TechContactSeq_var result(new TechContactSeq());
    result->length(tech_contacts.size());

    CORBA::ULong i = 0;
    for (Fred::AutomaticKeysetManagement::TechContacts::const_iterator tech_contact =
             tech_contacts.begin();
         tech_contact != tech_contacts.end();
         ++tech_contact, ++i)
    {
        result[i] = Corba::wrap_string_to_corba_string(*tech_contact);
    }

    return result;
}

Fred::AutomaticKeysetManagement::Nsset unwrap_Nsset(const Nsset& nsset)
{
    Fred::AutomaticKeysetManagement::Nsset result;
    for (CORBA::ULong i = 0; i < nsset.nameservers.length(); ++i) {
        result.nameservers.push_back(Corba::unwrap_string_from_const_char_ptr(nsset.nameservers[i]));
    }
    return result;
}

Fred::AutomaticKeysetManagement::DnsKey unwrap_DnsKey(const DnsKey& dns_key)
{
    unsigned short flags;
    Fred::Corba::unwrap_int(dns_key.flags, flags);
    unsigned short protocol;
    Fred::Corba::unwrap_int(dns_key.protocol, protocol);
    unsigned short alg;
    Fred::Corba::unwrap_int(dns_key.alg, alg);
    const std::string key = Corba::unwrap_string(dns_key.public_key);

    return Fred::AutomaticKeysetManagement::DnsKey(
            flags,
            protocol,
            alg,
            key);
}

Fred::AutomaticKeysetManagement::DnsKeys unwrap_DnsKeys(const DnsKeySeq& dns_key_seq)
{
    Fred::AutomaticKeysetManagement::DnsKeys result;
    for (CORBA::ULong i = 0; i < dns_key_seq.length(); ++i) {
        result.push_back(unwrap_DnsKey(dns_key_seq[i]));
    }
    return result;
}

Fred::AutomaticKeysetManagement::Keyset unwrap_Keyset(const Keyset& keyset)
{
    Fred::AutomaticKeysetManagement::Keyset result;
    result.dns_keys = unwrap_DnsKeys(keyset.dns_keys);
    return result;
}

} // namespace {anonymous}

Server_i::Server_i(
        const std::string& _server_name,
        const std::string& _automatically_managed_keyset_prefix,
        const std::string& _automatically_managed_keyset_registrar,
        const std::string& _automatically_managed_keyset_tech_contact,
        const std::string& _automatically_managed_keyset_zones,
        const bool _disable_notifier)
    : pimpl_(new Fred::AutomaticKeysetManagement::AutomaticKeysetManagementImpl(
                      _server_name,
                      _automatically_managed_keyset_prefix,
                      _automatically_managed_keyset_registrar,
                      _automatically_managed_keyset_tech_contact,
                      _automatically_managed_keyset_zones,
                      _disable_notifier))
{
}

Server_i::~Server_i() {
  delete pimpl_;
}

//
//   Methods corresponding to IDL attributes and operations
//

NameserverDomainsSeq* Server_i::get_nameservers_with_automatically_managed_domain_candidates()
{
    try
    {
        const Fred::AutomaticKeysetManagement::NameserversDomains nameservers_domains =
                pimpl_->get_nameservers_with_automatically_managed_domain_candidates();

        return wrap_NameserversDomains(nameservers_domains)._retn();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

NameserverDomainsSeq* Server_i::get_nameservers_with_automatically_managed_domains()
{
    try
    {
        const Fred::AutomaticKeysetManagement::NameserversDomains nameservers_domains =
                pimpl_->get_nameservers_with_automatically_managed_domains();

        return wrap_NameserversDomains(nameservers_domains)._retn();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

void Server_i::update_domain_automatic_keyset(
        ::CORBA::ULongLong _domain_id,
        const Registry::AutomaticKeysetManagement::Nsset& _current_nsset,
        const Registry::AutomaticKeysetManagement::Keyset& _new_keyset)
{
    try
    {
        const unsigned long long domain_id = Fred::Corba::wrap_int<unsigned long long>(_domain_id);
        Fred::AutomaticKeysetManagement::Nsset current_nsset = unwrap_Nsset(_current_nsset);
        Fred::AutomaticKeysetManagement::Keyset new_keyset = unwrap_Keyset(_new_keyset);

        pimpl_->update_domain_automatic_keyset(
                domain_id,
                current_nsset,
                new_keyset);
    }
    catch (Fred::AutomaticKeysetManagement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (Fred::AutomaticKeysetManagement::NssetInvalid&)
    {
        throw NSSET_INVALID();
    }
    catch (Fred::AutomaticKeysetManagement::KeysetInvalid&)
    {
        throw KEYSET_INVALID();
    }
    catch (Fred::AutomaticKeysetManagement::NssetDiffers&)
    {
        throw NSSET_DIFFERS();
    }
    catch (Fred::AutomaticKeysetManagement::DomainHasOtherKeyset&)
    {
        throw DOMAIN_HAS_OTHER_KEYSET();
    }
    catch (Fred::AutomaticKeysetManagement::DomainStatePolicyError&)
    {
        throw DOMAIN_STATE_POLICY_ERROR();
    }
    catch (Fred::AutomaticKeysetManagement::KeysetStatePolicyError&)
    {
        throw KEYSET_STATE_POLICY_ERROR();
    }
    catch (Fred::AutomaticKeysetManagement::SystemRegistratorNotFound&)
    {
        throw SYSTEM_REGISTRATOR_NOT_FOUND();
    }
    catch (Fred::AutomaticKeysetManagement::ConfigurationError&)
    {
        throw CONFIGURATION_ERROR();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

TechContactSeq* Server_i::get_nsset_notification_emails_by_domain_id(
        ::CORBA::ULongLong _domain_id)
{
    try
    {
        const unsigned long long domain_id = Fred::Corba::wrap_int<unsigned long long>(_domain_id);

        const Fred::AutomaticKeysetManagement::TechContacts tech_contacts =
                pimpl_->get_nsset_notification_emails_by_domain_id(domain_id);

        return wrap_TechContacts(tech_contacts)._retn();
    }
    catch (Fred::AutomaticKeysetManagement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

} // namespace Registry::AutomaticKeysetManagement
} // namespace Registry
