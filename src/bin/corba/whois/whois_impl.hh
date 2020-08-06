/*
 * Copyright (C) 2011-2020  CZ.NIC, z. s. p. o.
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
#ifndef WHOIS_IMPL_HH_83BEF5EF7B324EEDB301BC49529CD44A
#define WHOIS_IMPL_HH_83BEF5EF7B324EEDB301BC49529CD44A

#include "corba/Whois.hh"

#include "src/deprecated/libfred/registry.hh"
//#include "src/deprecated/util/dbsql.hh"

#include <memory>
#include <string>

class ccReg_Whois_i : public POA_ccReg::Whois,
  public PortableServer::RefCountServantBase
{
private:
  std::string m_connection_string;
  const std::string server_name_;
  bool registry_restricted_handles_;
  DBSharedPtr  db_disconnect_guard_;
  std::unique_ptr<LibFred::Manager> registry_manager_;

  void fillRegistrar(ccReg::WhoisRegistrar& creg,
                     LibFred::Registrar::Registrar *reg);

  void fillContact(ccReg::ContactDetail* cv, LibFred::Contact::Contact* c);
  void fillNSSet(ccReg::NSSetDetail* cn, LibFred::Nsset::Nsset* n);
  void fillKeySet(ccReg::KeySetDetail* cn, LibFred::Keyset::Keyset* n);
  void fillDomain(ccReg::DomainDetail* cd, LibFred::Domain::Domain* d);

public:
  ccReg_Whois_i(const std::string& database, const std::string& _server_name
          , bool _registry_restricted_handles);
  virtual ~ccReg_Whois_i();

  const std::string& get_server_name() const;

  ccReg::WhoisRegistrar* getRegistrarByHandle(const char* handle);
  ccReg::WhoisRegistrarList* getRegistrarsByZone(const char *zone);
  ccReg::ContactDetail* getContactByHandle(const char* handle);
  ccReg::NSSetDetail* getNSSetByHandle(const char* handle);
  ccReg::KeySetDetail *getKeySetByHandle(const char* handle);

  ccReg::DomainDetails* getDomainsByInverseKey(const char* key,
                                               ccReg::DomainInvKeyType type,
                                               CORBA::Long limit);
  ccReg::NSSetDetails* getNSSetsByInverseKey(const char* key,
                                             ccReg::NSSetInvKeyType type,
                                             CORBA::Long limit);
  ccReg::KeySetDetails *getKeySetsByInverseKey(
          const char *key,
          ccReg::KeySetInvKeyType type,
          CORBA::Long limit);
  ccReg::DomainDetail* getDomainByFQDN(const char* fqdn);
  Registry::ObjectStatusDescSeq* getDomainStatusDescList(const char *lang);
  Registry::ObjectStatusDescSeq* getContactStatusDescList(const char *lang);
  Registry::ObjectStatusDescSeq* getNSSetStatusDescList(const char *lang);
  Registry::ObjectStatusDescSeq* getKeySetStatusDescList(const char *lang);
};//class ccReg_Whois_i

#endif//WHOIS_IMPL_HH_83BEF5EF7B324EEDB301BC49529CD44A
