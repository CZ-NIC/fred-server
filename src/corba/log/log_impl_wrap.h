#ifndef _LOG_IMPL_WRAP_H_
#define _LOG_IMPL_WRAP_H_

#include <corba/ccReg.hh>

#include "log_impl.h"

class ccReg_Log_i : public POA_ccReg::Logger,
  public PortableServer::RefCountServantBase,
  private Impl_Log
{

public:

  ccReg_Log_i(const std::string database, const std::string &monitoring_hosts_file = std::string()) : Impl_Log(database, monitoring_hosts_file) {};

  // ccReg_Log_i(const std::string database) throw (Impl_Log::DB_CONNECT_FAILED): Impl_Log(database) {};
  virtual ~ccReg_Log_i() {};

  ccReg::TID CreateRequest(const char *sourceIP, ccReg::RequestServiceType service, const char *content_in, const ccReg::RequestProperties& props, CORBA::Long action_type, ccReg::TID session_id);

  CORBA::Boolean UpdateRequest(ccReg::TID id, const ccReg::RequestProperties &props);
  CORBA::Boolean CloseRequest(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props);
  CORBA::Boolean CloseRequestLogin(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props, ccReg::TID session_id);
  ccReg::TID CreateSession(ccReg::Languages lang, const char *name);
  // ccReg::TID new_dummy(const char *name, const char *clTRID);
  CORBA::Boolean CloseSession(ccReg::TID id);
  ccReg::RequestActionList *GetServiceActions(ccReg::RequestServiceType service);

public:
  Impl_Log::DB_CONNECT_FAILED;
};

#endif

