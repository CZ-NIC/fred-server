#ifndef _LOG_IMPL_H_

#define _LOG_IMPL_H_

// #include "old_utils/dbsql.h"

#include "register/db_settings.h"
#include "old_utils/conf.h"
#include "corba/mailer_manager.h"
#include <corba/ccReg.hh>

#include <map>

#include "conf/manager.h"

using namespace Database;

class NameService;

class ccReg_Log_i : public POA_ccReg::Logger,
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

  Manager db_manager;

  /*
  std::tr1::unordered_map<std::string, ccReg::TID> property_names
  */
  std::map<std::string, ccReg::TID, strCmp> property_names;


public:
//  void garbageSession();
  struct DB_CONNECT_FAILED
  {
  };

  ccReg_Log_i(const std::string database, NameService *ns, Config::Conf& _cfg)
      throw (DB_CONNECT_FAILED);
  virtual ~ccReg_Log_i();


  ccReg::TID new_event(const char *sourceIP, ccReg::LogServiceType service, const char *content_in, const ccReg::LogProperties& props);
  CORBA::Boolean update_event(ccReg::TID id, const ccReg::LogProperties &props);
  CORBA::Boolean update_event_close(ccReg::TID id, const char *content_out, const ccReg::LogProperties &props);
  ccReg::TID new_session(ccReg::Languages lang, const char *name, const char *clTRID);
  // ccReg::TID new_dummy(const char *name, const char *clTRID);
  CORBA::Boolean end_session(ccReg::TID id, const char *clTRID);

private:
  void insert_props(ccReg::TID entry_id, const ccReg::LogProperties& props, Connection &conn);
  bool record_check(ccReg::TID id, Connection &conn);
  ccReg::TID find_property_name_id(const char *name, Connection &conn);
  inline ccReg::TID find_last_property_value_id(Connection &conn);

  static const std::string LAST_PROPERTY_VALUE_ID;
  static const std::string LAST_PROPERTY_NAME_ID;
  static const std::string LAST_ENTRY_ID;
  static const std::string LAST_SESSION_ID;
};

#endif
