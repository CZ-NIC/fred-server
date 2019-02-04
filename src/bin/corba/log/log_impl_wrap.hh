#ifndef LOG_IMPL_WRAP_HH_0D1DFB40B54E427BBDEC954326FCE2CC
#define LOG_IMPL_WRAP_HH_0D1DFB40B54E427BBDEC954326FCE2CC

#include "src/bin/corba/Logger.hh"

#include "src/deprecated/libfred/requests/request_manager.hh"
#include "src/bin/corba/admin/usertype_conv.hh"
#include "src/bin/corba/admin/pagetable_logger.hh"
#include "src/bin/corba/admin/common.hh"



using namespace LibFred::Logger;


class ccReg_Log_i : public POA_ccReg::Logger,
  public PortableServer::RefCountServantBase
{

public:

  ccReg_Log_i(const std::string database, const std::string &monitoring_hosts_file = std::string()); 

  // ccReg_Log_i(const std::string database) throw (Impl_Log_If::DB_CONNECT_FAILED): Impl_Log_If(database) {};
  virtual ~ccReg_Log_i();

  ccReg::TID createRequest(const char *source_ip, ccReg::RequestServiceType service, const char *content, const ccReg::RequestProperties& props, const ccReg::ObjectReferences &refs, CORBA::Long request_type_id, ccReg::TID session_id);

  void closeRequest(ccReg::TID id, const char *content, const ccReg::RequestProperties &props, const ccReg::ObjectReferences &refs, const CORBA::Long result_code, ccReg::TID session_id);
  ccReg::TID createSession(ccReg::TID user_id, const char *name);
  // ccReg::TID new_dummy(const char *name, const char *clTRID);
  void closeSession(ccReg::TID id);
  ccReg::RequestTypeList *getRequestTypesByService(ccReg::RequestServiceType service);
  ccReg::RequestServiceList* getServices();
  ccReg::ResultCodeList* getResultCodesByService(ccReg::RequestServiceType service);
  ccReg::Logger::ObjectTypeList* getObjectTypes();
  Registry::PageTable_ptr createPageTable(const char *session_id);
  void deletePageTable(const char *session_id);

  ccReg::Logger::Detail* getDetail(ccReg::TID _id);
  ccReg::Logger::Detail* createRequestDetail(LibFred::Logger::Request *req);

  CORBA::ULongLong getRequestCount(const char *datetime_from, const char *datetime_to,
          const char *service, const char *user);
  ccReg::RequestCountInfo* getRequestCountUsers(const char *datetime_from, const char *datetime_to,
          const char *service);


private:
  typedef std::map<std::string, ccReg_Logger_i*> pagetables_list;

  pagetables_list pagetables;
  std::unique_ptr<LibFred::Logger::Manager> back;

  boost::mutex pagetables_mutex;

public:
  typedef LibFred::Logger::Manager::DB_CONNECT_FAILED DB_CONNECT_FAILED ;
};

#endif

