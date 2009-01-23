#ifndef _LOG_IMPL_H_

#define _LOG_IMPL_H_

// #include "old_utils/dbsql.h"

#include "register/db_settings.h"
#include "old_utils/conf.h"
#include "corba/mailer_manager.h"
#include <corba/ccReg.hh>

#include <map>
/*
 * #include <tr1/unordered_map>
 */

#include "conf/manager.h"

using namespace Database;

class NameService;

class ccReg_Log_i : public POA_ccReg::Log,
  public PortableServer::RefCountServantBase
{
private:
    struct strCmp {
		bool operator()(const std::string &s1, const std::string &s2) const {
			return s1 < s2;
		}
	};


  /** Limit the number of entries read from log_property_name table
   * (which is supposed to contain limited number of distinct property names )
   */
  static const int PROP_NAMES_SIZE_LIMIT = 10000;

  Connection conn;

  /*
  std::tr1::unordered_map<std::string, ccReg::TID> property_names
  */
  std::map<std::string, ccReg::TID, strCmp> property_names;
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


  ccReg::TID new_event(const char *sourceIP, ccReg::LogServiceType service, const char *content_in, const ccReg::LogProperties& props);
  CORBA::Boolean update_event(ccReg::TID id, const ccReg::LogProperties &props);
  CORBA::Boolean update_event_close(ccReg::TID id, const char *content_out, const ccReg::LogProperties &props);

private:
  void insert_props(ccReg::TID entry_id, const ccReg::LogProperties& props);
  bool record_check(ccReg::TID id);
  ccReg::TID find_property_name_id(const char *name);
  inline ccReg::TID find_last_property_value_id();

};

#endif
