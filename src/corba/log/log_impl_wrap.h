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

  ccReg::TID new_event(const char *sourceIP, ccReg::LogServiceType service, const char *content_in, const ccReg::LogProperties& props, CORBA::Long action_type);
  CORBA::Boolean update_event(ccReg::TID id, const ccReg::LogProperties &props);
  CORBA::Boolean update_event_close(ccReg::TID id, const char *content_out, const ccReg::LogProperties &props);
  ccReg::TID new_session(ccReg::Languages lang, const char *name, const char *clTRID);
  // ccReg::TID new_dummy(const char *name, const char *clTRID);
  CORBA::Boolean end_session(ccReg::TID id, const char *clTRID);

public:
  Impl_Log::DB_CONNECT_FAILED;
};

#endif

