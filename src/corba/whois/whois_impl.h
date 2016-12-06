#ifndef WHOIS_IMPL_H_65FAB81BA55942308A1648E27C426A25
#define WHOIS_IMPL_H_65FAB81BA55942308A1648E27C426A25

#include <memory>
#include <string>

#include "src/fredlib/registry.h"
//#include "src/old_utils/dbsql.h"

#include "src/corba/Whois.hh"

class ccReg_Whois_i : public POA_ccReg::Whois,
  public PortableServer::RefCountServantBase
{
private:
  std::string m_connection_string;
  const std::string server_name_;
  bool registry_restricted_handles_;
  DBSharedPtr  db_disconnect_guard_;
  std::auto_ptr<Fred::Manager> registry_manager_;

  void fillRegistrar(ccReg::WhoisRegistrar& creg,
                     Fred::Registrar::Registrar *reg);

  void fillContact(ccReg::ContactDetail* cv, Fred::Contact::Contact* c);
  void fillNSSet(ccReg::NSSetDetail* cn, Fred::Nsset::Nsset* n);
  void fillKeySet(ccReg::KeySetDetail* cn, Fred::Keyset::Keyset* n);
  void fillDomain(ccReg::DomainDetail* cd, Fred::Domain::Domain* d);

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

#endif
