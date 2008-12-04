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

  // now fill the property_names map:

	try {
		Result res = conn.exec("select id, name from log_property_name");

		if (res.size() > PROP_NAMES_SIZE_LIMIT) {
			LOGGER("fred-logd").error(" Number of entries in log_property_name is over the limit.");
			return;
		}

		for(Result::Iterator it = res.begin(); it != res.end(); ++it) {
			Row row = *it;

			property_names[row[1]] = row[0];

			// std::cout << "member " << row[0] << row[1] << std::endl;
		}
	} catch (Database::Exception &ex) {
		LOGGER("fred-logd").error(ex.what());
	}

}

ccReg_Log_i::~ccReg_Log_i() {
  // db.Disconnect();
  LOG( ERROR_LOG, "EPP_i destructor");
}


void ccReg_Log_i::insert_props(ccReg::TID entry_id, const ccReg::LogProperties& props)
{
	std::string s_val, s_name;
	std::ostringstream query;
	ccReg::TID name_id;
	// std::tr1::unordered_map<std::string, ccReg::TID>::iterator; 	// TODO
	std::map<std::string, ccReg::TID>::iterator iter;

	for (unsigned i = 0; i < props.length(); i++) {
		s_name = Util::escape((const char*) props[i].name);
		s_val = Util::escape((const char*) props[i].value);

		// TODO this should be changed
		iter = property_names.find(s_name);

		if(iter != property_names.end()) {
			name_id = iter->second;
		} else {
			query.str("");
			query << "select id from log_property_name where name='" << s_name
					<< "'";
			LOG(NOTICE_LOG, query.str().c_str());
			Result res = conn.exec(query.str());

			if (res.size() > 0) {
				name_id = res[0][0];
			} else if (res.size() == 0) {
				query.str("");
				query << "insert into log_property_name (name) values ('" << s_name
						<< "')";
				LOG(NOTICE_LOG, query.str().c_str());
				conn.exec(query.str());

				// TODO sequence
				query.str("");

				// query << "select id from log_property_name where name='" << s_name
				//		<< "'";

				query << "select currval('log_property_name_id_seq'::regclass)";
				LOG(NOTICE_LOG, query.str().c_str());
				Result res2 = conn.exec(query.str());

				name_id = res2[0][0];
			}
		}
		// end of TODO zone

		query.str("");
		query
				<< "insert into log_property_value (entry_id, name_id, value, output) values ("
				<< entry_id << ", " << name_id << ", '" << s_val << "', "
				<< (props[i].output ? "true" : "false") << ")";

		LOG(NOTICE_LOG, query.str().c_str());
		conn.exec(query.str());
	}
}

ccReg::TID ccReg_Log_i::new_event(const char *sourceIP, ccReg::LogServiceType service, const char *content_in, const ccReg::LogProperties& props)
{
	std::ostringstream query;
	std::string time, s_sourceIP, s_content;
	ccReg::TID entry_id;

	// get formatted UTC with microseconds
	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	try {
		// Transaction t(conn);
		if(sourceIP != NULL) {
			// make sure these values can be safely used in an SQL statement
			s_sourceIP = Util::escape(std::string(sourceIP));
			query << "insert into log_entry (time_begin, source_ip, service) values ('" << time << "', '" << s_sourceIP << "', "
				<< service << ")";
		} else {
			query << "insert into log_entry (time_begin, service) values ('"
				<< time << "', " << service << ")";
		}
	 	LOG(DEBUG_LOG, query.str().c_str());
		conn.exec(query.str());

		// get the id of the new entry
		query.str("");
		// query << "select id from log_entry where time_begin='" << time << "'";
		query << "select currval('log_entry_id_seq'::regclass)";
		Result res = conn.exec(query.str());
		entry_id = res[0][0];

		// "select

		// insert into log_raw_content
		if(content_in != NULL) {
			s_content = Util::escape(std::string(content_in));
			query.str("");
			query << "insert into log_raw_content (entry_id, request) values (" << entry_id << ", '" << s_content << "')";

			LOG(DEBUG_LOG, query.str().c_str());
			// LOGGER("fred-logd").debug(query.str());
			conn.exec(query.str());
		}

		// inserting properties
		insert_props(entry_id, props);

		// t.commit();
	} catch (Database::Exception &ex) {
		LOGGER("fred-logd").error(ex.what());
		return 0; 	// TODO throw a corba exception
	}
	return entry_id;
}

CORBA::Boolean ccReg_Log_i::update_event(ccReg::TID id, const ccReg::LogProperties &props)
{
	std::ostringstream query;

	try {
		// TODO perform debug check. this function mustn't be call on a complete record
		/*
		query << "select time_end from log_entry id=" << id;
		Result res = conn.exec(query.str());

		// false should be returned if time_end is not null
		*/

		insert_props(id, props);
	} catch (Database::Exception &ex) {
		LOGGER("fred-logd").error(ex.what());
		return false;
	}
	return true;

}

CORBA::Boolean ccReg_Log_i::update_event_close(ccReg::TID id, const char *content_out, const ccReg::LogProperties &props)
{
	std::ostringstream query;
	std::string s_content, time;

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	try {
		query << "update log_entry set time_end='" << time << "' where id=" << id;
		Result res = conn.exec(query.str());

		/* TODO
		query << "select time_end from log_entry where id=" << id;
		Result res = conn.exec(query.str());

		// false should be returned if time_end is not null
		*/

		if(content_out != NULL) {
			s_content = Util::escape(std::string(content_out));

			query.str("");
			query << "select * from log_raw_content where entry_id = " << id;
			Result res = conn.exec(query.str());

			query.str("");
			if(res.size() > 0) {
				query << "update log_raw_content set response = '" << s_content << "' where entry_id=" << id;
			} else {
				query << "insert into log_raw_content (entry_id, response) values (" << id << ", '" << s_content << "')";
			}
			conn.exec(query.str());
		}

		// inserting properties
		insert_props(id, props);

	} catch (Database::Exception &ex) {
		LOGGER("fred-logd").error(ex.what());
		return false;
	}
	return true;
}


