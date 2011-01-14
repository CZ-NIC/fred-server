#ifndef _WHOIS_IMPL_H_
#define _WHOIS_IMPL_H_

#include <memory>
#include <string>

#include "fredlib/registry.h"
#include "old_utils/dbsql.h"

class ccReg_Whois_i : public POA_ccReg::Whois,
  public PortableServer::RefCountServantBase
{
private:
  std::string m_connection_string;
  std::string server_name_;
  bool registry_restricted_handles_;
  DB db;
  std::auto_ptr<Fred::Manager> registry_manager_;

  void fillRegistrar(ccReg::AdminRegistrar& creg,
                     Fred::Registrar::Registrar *reg);

  void fillContact(ccReg::ContactDetail* cv, Fred::Contact::Contact* c);
  void fillNSSet(ccReg::NSSetDetail* cn, Fred::NSSet::NSSet* n);
  void fillKeySet(ccReg::KeySetDetail* cn, Fred::KeySet::KeySet* n);
  void fillDomain(ccReg::DomainDetail* cd, Fred::Domain::Domain* d);

public:
  ccReg_Whois_i(const std::string& database, const std::string& _server_name
          , bool _registry_restricted_handles);
  virtual ~ccReg_Whois_i();

  ccReg::AdminRegistrar* getRegistrarByHandle(const char* handle);
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
};//class ccReg_Whois_i

#endif //WHOIS_IMPL_H