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

//pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/bin/corba ~/workspace_18680/enum/idl/idl/AutomaticKeysetManagement.idl

#include "src/bin/corba/automatic_keyset_management/server_i.hh"

#include "src/backend/automatic_keyset_management/automatic_keyset_management.hh"
#include "src/bin/corba/automatic_keyset_management/corba_conversions.cc"
#include "src/bin/corba/AutomaticKeysetManagement.hh"

#include <iostream>

namespace Registry {
namespace AutomaticKeysetManagement {

Server_i::Server_i(
        const std::string& _server_name,
        const std::string& _automatically_managed_keyset_prefix,
        const std::string& _automatically_managed_keyset_registrar,
        const std::string& _automatically_managed_keyset_tech_contact,
        const std::set<std::string>& _automatically_managed_keyset_zones,
        const bool _disable_notifier,
        LibFred::Logger::LoggerClient& _logger_client)
    : impl_(new LibFred::AutomaticKeysetManagement::AutomaticKeysetManagementImpl(
                      _server_name,
                      _automatically_managed_keyset_prefix,
                      _automatically_managed_keyset_registrar,
                      _automatically_managed_keyset_tech_contact,
                      _automatically_managed_keyset_zones,
                      _disable_notifier,
                      _logger_client))
{
}

Server_i::~Server_i()
{
}

//
//   Methods corresponding to IDL attributes and operations
//

NameserverDomainsSeq* Server_i::get_nameservers_with_insecure_automatically_managed_domain_candidates()
{
    try
    {
        const LibFred::AutomaticKeysetManagement::NameserversDomains nameservers_domains =
                impl_->get_nameservers_with_insecure_automatically_managed_domain_candidates();

        return LibFred::Corba::AutomaticKeysetManagement::wrap_NameserversDomains(nameservers_domains)._retn();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

NameserverDomainsSeq* Server_i::get_nameservers_with_secure_automatically_managed_domain_candidates()
{
    try
    {
        const LibFred::AutomaticKeysetManagement::NameserversDomains nameservers_domains =
                impl_->get_nameservers_with_secure_automatically_managed_domain_candidates();

        return LibFred::Corba::AutomaticKeysetManagement::wrap_NameserversDomains(nameservers_domains)._retn();
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
        const LibFred::AutomaticKeysetManagement::NameserversDomains nameservers_domains =
                impl_->get_nameservers_with_automatically_managed_domains();

        return LibFred::Corba::AutomaticKeysetManagement::wrap_NameserversDomains(nameservers_domains)._retn();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

void Server_i::turn_on_automatic_keyset_management_on_insecure_domain(
        ::CORBA::ULongLong _domain_id,
        const Registry::AutomaticKeysetManagement::Nsset& _current_nsset,
        const Registry::AutomaticKeysetManagement::Keyset& _new_keyset)
{
    try
    {
        const unsigned long long domain_id = LibFred::Corba::wrap_int<unsigned long long>(_domain_id);
        LibFred::AutomaticKeysetManagement::Nsset current_nsset = LibFred::Corba::AutomaticKeysetManagement::unwrap_Nsset(_current_nsset);
        LibFred::AutomaticKeysetManagement::Keyset new_keyset = LibFred::Corba::AutomaticKeysetManagement::unwrap_Keyset(_new_keyset);

        impl_->turn_on_automatic_keyset_management_on_insecure_domain(
                domain_id,
                current_nsset,
                new_keyset);
    }
    catch (LibFred::AutomaticKeysetManagement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (LibFred::AutomaticKeysetManagement::NssetIsEmpty&)
    {
        throw NSSET_IS_EMPTY();
    }
    catch (LibFred::AutomaticKeysetManagement::KeysetIsInvalid&)
    {
        throw KEYSET_IS_INVALID();
    }
    catch (LibFred::AutomaticKeysetManagement::NssetIsDifferent&)
    {
        throw NSSET_IS_DIFFERENT();
    }
    catch (LibFred::AutomaticKeysetManagement::DomainHasKeyset&)
    {
        throw DOMAIN_HAS_KEYSET();
    }
    catch (LibFred::AutomaticKeysetManagement::DomainAlreadyDoesNotHaveKeyset&)
    {
        throw DOMAIN_ALREADY_DOES_NOT_HAVE_KEYSET();
    }
    catch (LibFred::AutomaticKeysetManagement::DomainStatePolicyError&)
    {
        throw DOMAIN_STATE_POLICY_ERROR();
    }
    catch (LibFred::AutomaticKeysetManagement::KeysetStatePolicyError&)
    {
        throw KEYSET_STATE_POLICY_ERROR();
    }
    catch (LibFred::AutomaticKeysetManagement::ConfigurationError&)
    {
        throw CONFIGURATION_ERROR();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

void Server_i::turn_on_automatic_keyset_management_on_secure_domain(
        ::CORBA::ULongLong _domain_id,
        const Registry::AutomaticKeysetManagement::Keyset& _new_keyset)
{
    try
    {
        const unsigned long long domain_id = LibFred::Corba::wrap_int<unsigned long long>(_domain_id);
        LibFred::AutomaticKeysetManagement::Keyset new_keyset = LibFred::Corba::AutomaticKeysetManagement::unwrap_Keyset(_new_keyset);

        impl_->turn_on_automatic_keyset_management_on_secure_domain(
                domain_id,
                new_keyset);
    }
    catch (LibFred::AutomaticKeysetManagement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (LibFred::AutomaticKeysetManagement::KeysetIsInvalid&)
    {
        throw KEYSET_IS_INVALID();
    }
    catch (LibFred::AutomaticKeysetManagement::DomainDoesNotHaveKeyset&)
    {
        throw DOMAIN_DOES_NOT_HAVE_KEYSET();
    }
    catch (LibFred::AutomaticKeysetManagement::DomainAlreadyHasAutomaticallyManagedKeyset&)
    {
        throw DOMAIN_ALREADY_HAS_AUTOMATICALLY_MANAGED_KEYSET();
    }
    catch (LibFred::AutomaticKeysetManagement::DomainStatePolicyError&)
    {
        throw DOMAIN_STATE_POLICY_ERROR();
    }
    catch (LibFred::AutomaticKeysetManagement::ConfigurationError&)
    {
        throw CONFIGURATION_ERROR();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

void Server_i::update_automatically_managed_keyset_of_domain(
        ::CORBA::ULongLong _domain_id,
        const Registry::AutomaticKeysetManagement::Keyset& _new_keyset)
{
    try
    {
        const unsigned long long domain_id = LibFred::Corba::wrap_int<unsigned long long>(_domain_id);
        LibFred::AutomaticKeysetManagement::Keyset new_keyset = LibFred::Corba::AutomaticKeysetManagement::unwrap_Keyset(_new_keyset);

        impl_->update_automatically_managed_keyset_of_domain(
                domain_id,
                new_keyset);
    }
    catch (LibFred::AutomaticKeysetManagement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (LibFred::AutomaticKeysetManagement::KeysetIsInvalid&)
    {
        throw KEYSET_IS_INVALID();
    }
    catch (LibFred::AutomaticKeysetManagement::DomainDoesNotHaveAutomaticallyManagedKeyset&)
    {
        throw DOMAIN_DOES_NOT_HAVE_AUTOMATICALLY_MANAGED_KEYSET();
    }
    catch (LibFred::AutomaticKeysetManagement::KeysetSameAsDomainKeyset&)
    {
        throw KEYSET_SAME_AS_DOMAIN_KEYSET();
    }
    catch (LibFred::AutomaticKeysetManagement::DomainStatePolicyError&)
    {
        throw DOMAIN_STATE_POLICY_ERROR();
    }
    catch (LibFred::AutomaticKeysetManagement::KeysetStatePolicyError&)
    {
        throw KEYSET_STATE_POLICY_ERROR();
    }
    catch (LibFred::AutomaticKeysetManagement::ConfigurationError&)
    {
        throw CONFIGURATION_ERROR();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

EmailAddressSeq* Server_i::get_email_addresses_by_domain_id(
        ::CORBA::ULongLong _domain_id)
{
    try
    {
        const unsigned long long domain_id = LibFred::Corba::wrap_int<unsigned long long>(_domain_id);

        const LibFred::AutomaticKeysetManagement::EmailAddresses email_addresses =
                impl_->get_email_addresses_by_domain_id(domain_id);

        return LibFred::Corba::AutomaticKeysetManagement::wrap_EmailAddresses(email_addresses)._retn();
    }
    catch (LibFred::AutomaticKeysetManagement::ObjectNotFound&)
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
