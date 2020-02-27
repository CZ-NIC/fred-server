/*
 * Copyright (C) 2014-2020  CZ.NIC, z. s. p. o.
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
#ifndef WHOIS2_IMPL_HH_4FB0D9C852264D3F8B9706DFA7E2873E
#define WHOIS2_IMPL_HH_4FB0D9C852264D3F8B9706DFA7E2873E

#include "corba/Whois2.hh"
#include "src/backend/whois/whois.hh"

#include <string>

namespace CorbaConversion
{
namespace Whois
{

class Server_impl : public POA_Registry::Whois::WhoisIntf
{
public:
    Server_impl(const std::string& server_name_)
    : pimpl_(new Fred::Backend::Whois::Server_impl(server_name_))
    {
    }

    virtual ~Server_impl()
    {
    }

    Registry::Whois::Registrar* get_registrar_by_handle(const char* handle);

    Registry::Whois::RegistrarSeq* get_registrars();

    Registry::Whois::RegistrarGroupList* get_registrar_groups();

    Registry::Whois::RegistrarCertificationList* get_registrar_certification_list();

    Registry::Whois::ZoneFqdnList* get_managed_zone_list();

    Registry::Whois::Contact* get_contact_by_handle(const char* handle);

    Registry::Whois::NSSet* get_nsset_by_handle(const char* handle);

    Registry::Whois::NSSetSeq* get_nssets_by_ns(
        const char* handle,
        CORBA::ULong limit,
        CORBA::Boolean& limit_exceeded);

    Registry::Whois::NSSetSeq* get_nssets_by_tech_c(
        const char* handle,
        CORBA::ULong limit,
        CORBA::Boolean& limit_exceeded);

    Registry::Whois::NameServer* get_nameserver_by_fqdn(const char* handle);

    Registry::Whois::KeySet* get_keyset_by_handle(const char* handle);

    Registry::Whois::KeySetSeq* get_keysets_by_tech_c(
        const char* handle,
        CORBA::ULong limit,
        CORBA::Boolean& limit_exceeded);

    Registry::Whois::Domain* get_domain_by_handle(const char* handle);

    Registry::Whois::DomainSeq* get_domains_by_registrant(
        const char* handle,
        CORBA::ULong limit,
        CORBA::Boolean& limit_exceeded);

    Registry::Whois::DomainSeq* get_domains_by_admin_contact(
        const char* handle,
        CORBA::ULong limit,
        CORBA::Boolean& limit_exceeded);

    Registry::Whois::DomainSeq* get_domains_by_nsset(
        const char* handle,
        CORBA::ULong limit,
        CORBA::Boolean& limit_exceeded);

    Registry::Whois::DomainSeq* get_domains_by_keyset(
        const char* handle,
        CORBA::ULong limit,
        CORBA::Boolean& limit_exceeded);

    Registry::Whois::ObjectStatusDescSeq* get_domain_status_descriptions(const char* lang);
    Registry::Whois::ObjectStatusDescSeq* get_contact_status_descriptions(const char* lang);
    Registry::Whois::ObjectStatusDescSeq* get_nsset_status_descriptions(const char* lang);
    Registry::Whois::ObjectStatusDescSeq* get_keyset_status_descriptions(const char* lang);

private:
    const std::unique_ptr<Fred::Backend::Whois::Server_impl> pimpl_;
};

} // namespace Registry::Whois
} // namespace Registry
#endif
