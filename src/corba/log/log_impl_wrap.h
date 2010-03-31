#ifndef _LOG_IMPL_WRAP_H_
#define _LOG_IMPL_WRAP_H_

#include <corba/ccReg.hh>

// #include "request.h"
// even better:
#include "register/request.h"
#include "src/corba/admin/usertype_conv.h"
#include "src/corba/admin/pagetable_logger.h"
#include "src/corba/admin/common.h"



using namespace Register::Logger;

std::auto_ptr<Register::Logger::RequestProperties> convert_properties(const ccReg::RequestProperties &p);

class ccReg_Log_i : public POA_ccReg::Logger,
  public PortableServer::RefCountServantBase
{

public:

  ccReg_Log_i(const std::string database, const std::string &monitoring_hosts_file = std::string()); 

  // ccReg_Log_i(const std::string database) throw (Impl_Log_If::DB_CONNECT_FAILED): Impl_Log_If(database) {};
  virtual ~ccReg_Log_i();

  ccReg::TID CreateRequest(const char *sourceIP, ccReg::RequestServiceType service, const char *content_in, const ccReg::RequestProperties& props, CORBA::Long action_type, ccReg::TID session_id);

  CORBA::Boolean UpdateRequest(ccReg::TID id, const ccReg::RequestProperties &props);
  CORBA::Boolean CloseRequest(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props);
  CORBA::Boolean CloseRequestLogin(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props, ccReg::TID session_id);
  ccReg::TID CreateSession(ccReg::Languages lang, const char *name);
  // ccReg::TID new_dummy(const char *name, const char *clTRID);
  CORBA::Boolean CloseSession(ccReg::TID id);
  ccReg::RequestActionList *GetServiceActions(ccReg::RequestServiceType service);
  Registry::PageTable_ptr createPageTable(const char *session_id);
  void deletePageTable(const char *session_id);

  Registry::Request::Detail* getDetail(ccReg::TID _id);
  Registry::Request::Detail* createRequestDetail(Register::Logger::Request *req);

private:
  typedef std::map<std::string, ccReg_Logger_i*> pagetables_list;

  pagetables_list pagetables;
  std::auto_ptr<Register::Logger::Manager> back;

  boost::mutex pagetables_mutex;

public:
  typedef Register::Logger::Manager::DB_CONNECT_FAILED DB_CONNECT_FAILED ;
};

#endif

