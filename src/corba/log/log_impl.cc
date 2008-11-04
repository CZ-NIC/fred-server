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
// util (for escape)
#include "util.h"

using namespace Database;


ccReg_Log_i::ccReg_Log_i(const std::string database, NameService *ns, Config::Conf& _cfg, bool _session_garbage)
      throw (DB_CONNECT_FAILED) {

  try {
	conn.open(database);  
  } catch (Database::Exception &ex) {
	LOG(ALERT_LOG, "can not connect to DATABASE %s : %s", database.c_str(), ex.what());
	// LOGGER("db").error(ex.what());	
  }

  LOG(NOTICE_LOG, "successfully  connect to DATABASE %s", database.c_str());

}

  ccReg_Log_i::~ccReg_Log_i() {
	  // db.Disconnect();
	  LOG( ERROR_LOG, "EPP_i destructor");
  } 

  CORBA::Boolean ccReg_Log_i::message(const char* sourceIP, ccReg::LogComponent comp, ccReg::LogEventType event, const char* content, const ccReg::LogProperties& props) {

	std::ostringstream query;
	std::string time, s_sourceIP, s_content;

	// get formatted UTC with microseconds
	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());
	
	try {
		// make sure these values can be safely used in an SQL statement
		s_sourceIP = Util::escape(std::string(sourceIP));
		s_content = Util::escape(std::string(content));

		query << "insert into log_entry (time, source_ip, flag, component, content) values ('" << time << "', '" << s_sourceIP << "', "
			<< event << ", " << comp << ", '" << s_content << "')";

		conn.exec(query.str());
		// log entry inserted
		
		// Transaction t(conn);
		
		// snprintf(query, QUERY_BUF_SIZE, "select id from log_entry where	time='%s' and content='%s'", time.c_str(), content);

		query.str("");
		query << "select id from log_entry where time='" << time << "' and content='" << Util::escape(std::string(content)) << "'";

		LOGGER("fred-logd").debug(query.str());

		Result res=conn.exec(query.str());
		int entry_id = res[0][0]; 

		// LOGGER("fred-logd").error(ex.what());	

		// insert the property into the table log_property
		for (unsigned i=0;i<props.length();i++) {

			query.str("");
			query << "insert into log_property (entry_id, name, value) values (" << entry_id << ", '" <<  Util::escape((const char*)props[i].name)
				<< "', '" << Util::escape((const char*)props[i].value) << "')";

			LOGGER("fred-logd").debug(query.str());
			conn.exec(query.str());
		}	

		// t.commit();
	} catch (Database::Exception &ex) {
		LOGGER("fred-logd").error(ex.what());	
	}
	return true;
  }

