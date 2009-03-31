#ifndef _MIGRATE_H_
#define _MIGRATE_H_

#include <iostream>
#include <fstream>
#include <map>

#include "register/db_settings.h"
#include "db/transaction.h"

#include "types.h"


using namespace Database;

typedef unsigned long long TID;

class Backend
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
  static const unsigned int PROP_NAMES_SIZE_LIMIT = 1000;
  // static const unsigned int TRANS_LIMIT = 2000;
  static const unsigned int TRANS_LIMIT = 100;

  Connection conn;
  Transaction *trans; 
  int transaction_counter;

  std::map<std::string, TID, strCmp> property_names;

public:
//  void garbageSession();
  struct DB_CONNECT_FAILED
  {
  };

  Backend(const std::string database);
  ~Backend() {
	try {
		trans->commit();
		logger(boost::format("Commit after %1% records") % transaction_counter);
	} catch (Database::Exception &ex) {
		logger(ex.what());
		logger("Exception caught in ~Backend() ");
	}
  };

  void commit () { 
	if(!transaction_counter) {
		try {
			trans->commit();
		} catch (Database::Exception &ex) {
			logger(ex.what());
			throw;
		}
		transaction_counter = TRANS_LIMIT;
	} else {
		transaction_counter--;
	}
  };

  void abort() {
	try {
		delete trans;
		trans = new Transaction(conn);
	} catch (Database::Exception &ex) {
		logger(ex.what());
		throw;
	}
  }

  TID new_event(const char *sourceIP, LogServiceType service, const char *content_in, const ccProperties& props);
  bool update_event(TID id, const ccProperties &props);
  bool update_event_close(TID id, const char *content_out, const ccProperties &props) { return false; };

  inline void logger(std::string message) {
	std::cerr << message << std::endl; 
  };
  inline void logger(const boost::format& fmt) {
	logger(fmt.str());
  }
  void test();

private:
  void insert_props(TID entry_id, const ccProperties& props);
  bool record_check(TID id);
  TID find_property_name_id(const char *name);
  inline TID find_last_property_value_id();
  
  static const std::string LAST_PROPERTY_VALUE_ID;
  static const std::string LAST_PROPERTY_NAME_ID;
  static const std::string LAST_ENTRY_ID;
};

#endif

