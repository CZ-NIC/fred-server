/*
 *  Copyright (C) 2008  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "log_impl.h"

#include <fstream>
#include <iostream>

#include <stdlib.h>

#include <corba/ccReg.hh>
/*
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"

#include <string.h>
#include <time.h>
#include "epp_impl.h"

#include "config.h"
// database functions
#include "old_utils/dbsql.h"
#include "db/manager.h"

// support function
#include "old_utils/util.h"

*/
// logger
#include "old_utils/log.h"
//config
#include "old_utils/conf.h"
// database 
#include "db/database.h"

using namespace Database;

// stolen & modified from admin/common.cc

// 
/*
const char *
formatTime(ptime p, bool date)
{
  if (p.is_special()) return "--special--";
  std::ostringstream stime;
  stime << std::setfill('0') << std::setw(2)
        << p.date().day() << "." 
        << std::setw(2)
        << (int)p.date().month() << "." 
        << std::setw(2)
        << p.date().year();
  if (date) 
   stime << " "
         << std::setw(2)
         << p.time_of_day().hours() << ":"
         << std::setw(2)
         << p.time_of_day().minutes() << ":"
         << std::setw(2)
         << p.time_of_day().seconds() << "."
	 << std::setw(3)
	 << p.time_of_day().milliseconds();
  return stime.str().c_str();
}
*/

ccReg_Log_i::ccReg_Log_i(const std::string database, NameService *ns, Conf& _cfg, bool _session_garbage)
      throw (DB_CONNECT_FAILED) {

// objects are shared between threads!!!
  // init at the beginning and do not change
  /*
  strncpy(database, _db, sizeof(database)-1);
  if (!db.OpenDatabase(database)) {
    LOG(ALERT_LOG, "can not connect to DATABASE %s", database);
    throw DB_CONNECT_FAILED();
  }

  LOG(NOTICE_LOG, "successfully  connect to DATABASE %s", database);
*/

  try {
	conn.open(database);  
  } catch (Database::Exception &ex) {
	LOGGER("db").error(ex.what());	
  }

  LOG(NOTICE_LOG, "ccReg_Log_i constructor successful ");

}

  ccReg_Log_i::~ccReg_Log_i() {
	  // db.Disconnect();
	  LOG( ERROR_LOG, "EPP_i destructor");
  } 


  int getPropID(Database::Connection &c, ::CORBA::String_member name) throw (Database::Exception) {
	std::ostringstream sel;

	sel << "select id from property where name = '" << name << "'";
	Result r = c.exec(sel.str());
	if (r.size() == 0) return -1;
	return r[0][0];
  }

  void insProperty(Database::Connection &c, ::CORBA::String_member name) throw (Database::Exception) {
	std::ostringstream ins;
	ins << "insert into property (name) values ('" << name << "')"; 
	c.exec(ins.str());
  }
  
  CORBA::Boolean ccReg_Log_i::message(const char* sourceIP, ccReg::LogComponent comp, ccReg::LogEventType event, const char* content, const ccReg::Properties& props, CORBA::Long clientID) {

	std::ostringstream query;
	std::string time;

	// get formatted UTC with microseconds
	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());
	
	try {
		query << "insert into log_entry (time, source_ip, flag, component, content, client_id) values ('" << time << "', '" << sourceIP << "', "
			<< event << ", " << comp << ", '" << content << "', " << clientID << ")";
		// snprintf(query, QUERY_BUF_SIZE, "insert into log_entry (time, source_ip, flag, component, content, client_id) values ('%s', '%s', %i, %i, '%s', %li)", time.c_str(), sourceIP, event, comp, content, (long)clientID);

		conn.exec(query.str());
		// log entry inserted
		
		// Transaction t(conn);
		
		// snprintf(query, QUERY_BUF_SIZE, "select id from log_entry where	time='%s' and content='%s'", time.c_str(), content);

		query.str("");
		query << "select id from log_entry where time='" << time << "' and content='" << content << "'";
		Result res=conn.exec(query.str());
		int entry_id = res[0][0]; 

		std::ostringstream os;
		os << "Log entry id: " << entry_id;
		LOGGER("db").info(os.str());

		// insert the property into the table
		for (unsigned i=0;i<props.length();i++) {

			int prop_id = getPropID(conn, props[i].name);

			if(prop_id == -1) {
				insProperty(conn, props[i].name);
				prop_id = getPropID(conn, props[i].name);
			}
			// prop_id really shouldn't be -1 now

			std::ostringstream msg;
			msg << "Prop entry id: " << prop_id;
			LOGGER("db").info(msg.str());

			query.str("");
			query << "insert into property_value (entry_id, property_id, value) values (" <<  entry_id << ", " << prop_id << ", '" << props[i].value << "')";
			conn.exec(query.str());
		}	

		// t.commit();
	} catch (Database::Exception &ex) {
		LOGGER("db").error(ex.what());	
	}
	LOGGER("corba").info ( "-----inside message. "); // source IP: %s, content: %s ", sourceIP, content);
	return true;
  }

  void ccReg_Log_i::testconn(const char *message) {
	  LOGGER("corba").info("-------- Test connection. ");
  }

