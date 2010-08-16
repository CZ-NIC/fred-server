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

  ccReg::TID createRequest(const char *source_ip, ccReg::RequestServiceType service, const char *content, const ccReg::RequestProperties& props, const ccReg::Logger::ObjectReferences &refs, CORBA::Long request_type_id, ccReg::TID session_id);

  void addRequestProperties(ccReg::TID id, const ccReg::RequestProperties &props);
  void closeRequest(ccReg::TID id, const char *content, const ccReg::RequestProperties &props, const ccReg::Logger::ObjectReferences &refs, const CORBA::Long result_code, ccReg::TID session_id);
  ccReg::TID createSession(ccReg::TID user_id, const char *name);
  // ccReg::TID new_dummy(const char *name, const char *clTRID);
  void closeSession(ccReg::TID id);
  ccReg::RequestTypeList *getRequestTypesByService(ccReg::RequestServiceType service);
  ccReg::RequestServiceList* getServices();
  ccReg::ResultCodeList* getResultCodesByService(ccReg::RequestServiceType service);
  ccReg::Logger::ObjectTypeList* getObjectTypes();
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

