#ifndef _LOG_IMPL_H_

#define _LOG_IMPL_H_

// #include "old_utils/dbsql.h"

#include "db/manager.h"
#include "old_utils/conf.h"
#include "corba/mailer_manager.h"
#include <corba/ccReg.hh>

#include "conf/manager.h"

using namespace Database;

class NameService;

class ccReg_Log_i : public POA_ccReg::Log,
  public PortableServer::RefCountServantBase
{
private:

  Connection conn;
/*
  std::string m_connection_string;
  NameService *ns;
  Conf& cfg;
  DB db;
  Database::Manager m_db_manager;
*/

/*
  typedef std::map<std::string, ccReg_Session_i*> SessionListType;
  SessionListType m_session_list;
  boost::mutex m_session_list_mutex;
  boost::condition cond_;
  bool session_garbage_active_;
  boost::thread *session_garbage_thread_;
*/


public:
//  void garbageSession();
  struct DB_CONNECT_FAILED
  {
  };

  ccReg_Log_i(const std::string database, NameService *ns, Config::Conf& _cfg, bool _session_garbage = true)
      throw (DB_CONNECT_FAILED);
  virtual ~ccReg_Log_i();

  CORBA::Boolean message(const char* sourceIP, ccReg::LogComponent comp, ccReg::LogEventType event, const char* content, const ccReg::LogProperties& props);

};

#endif
