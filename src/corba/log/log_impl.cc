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
		LOG(ALERT_LOG, "can not connect to DATABASE %s : %s", database.c_str(),
				ex.what());
		// LOGGER("db").error(ex.what());
	}

	LOG(NOTICE_LOG, "successfully  connect to DATABASE %s", database.c_str());

	// now fill the property_names map:

	try {
		Result res = conn.exec("select id, name from log_property_name");

		if (res.size() > PROP_NAMES_SIZE_LIMIT) {
			LOGGER("fred-logd").error(
					" Number of entries in log_property_name is over the limit.");
			return;
		}

		for (Result::Iterator it = res.begin(); it != res.end(); ++it) {
			Row row = *it;

			property_names[row[1]] = row[0];
		}
		// debug TODO remove
		std::cout << "---------- Map content: " << std::endl;

		for (std::map<std::string, ccReg::TID, strCmp>::iterator iter = property_names.begin();
			iter != property_names.end(); ++iter) {

			std::cout << "key: " << iter->first << " value: " << iter->second << std::endl;
		}

	} catch (Database::Exception &ex) {
		LOGGER("fred-logd").error(ex.what());
	}

}

ccReg_Log_i::~ccReg_Log_i() {
  // db.Disconnect();
  LOG( ERROR_LOG, "ccReg_Log_i destructor");
}

bool ccReg_Log_i::record_check(ccReg::TID id)
{
	std::ostringstream query;

	query << "select time_end from log_entry where id=" << id;
	Result res = conn.exec(query.str());

	// if there is no record with specified ID
	if(res.size() == 0) {
		LOGGER("fred-logd").error(boost::format("record with ID %1% doesn't exist") % id);
		return false;
	}

	// if the time_end is already filled (so the record
	// is complete and cannot be modified)
	if(!res[0][0]) {
		LOGGER("fred-logd").error(boost::format("record with ID %1% was already completed") % id);
		return false;
	}

	return true;
}

void ccReg_Log_i::insert_props(ccReg::TID entry_id, const ccReg::LogProperties& props)
{
	std::string s_val, s_name;
	std::ostringstream query;
	ccReg::TID name_id;
	// std::tr1::unordered_map<std::string, ccReg::TID>::iterator;
	std::map<std::string, ccReg::TID>::iterator iter;

	for (unsigned i = 0; i < props.length(); i++) {
		s_name = Util::escape((const char*) props[i].name);
		s_val = Util::escape((const char*) props[i].value);

		iter = property_names.find(s_name);

		if(iter != property_names.end()) {
			name_id = iter->second;
		} else {
			// if the name isn't cached in the memory, try to find it in the database
			query.str("");
			query << "select id from log_property_name where name='" << s_name
					<< "'";
			Result res = conn.exec(query.str());

			if (res.size() > 0) {
				// okay, it was found in the database
				name_id = res[0][0];
			} else if (res.size() == 0) {
				// if not, add it to the database
				query.str("");
				query << "insert into log_property_name (name) values ('" << s_name
						<< "')";
				conn.exec(query.str());

				query.str("");
				query << "select currval('log_property_name_id_seq'::regclass)";
				res = conn.exec(query.str());

				name_id = res[0][0];
			}

			// now that we know the right database id of the name
			// we can add it to the map
			property_names[s_name] = name_id;
		}

		query.str("");
		query   << "insert into log_property_value (entry_id, name_id, value, output) values ("
				<< entry_id << ", " << name_id << ", '" << s_val << "', "
				<< (props[i].output ? "true" : "false") << ")";

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

	} catch (Database::Exception &ex) {
		LOGGER("fred-logd").error(ex.what());
		return 0; 	// TODO throw a corba exception
	}

	try {
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

	} catch (Database::Exception &ex) {
		LOGGER("fred-logd").error(ex.what());
	}

	// t.commit();
	return entry_id;
}

CORBA::Boolean ccReg_Log_i::update_event(ccReg::TID id, const ccReg::LogProperties &props)
{
	std::ostringstream query;

	try {
		// perform check
		if (!record_check(id)) return false;

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
		// first perform checks:
		if (!record_check(id)) return false;

		query << "update log_entry set time_end='" << time << "' where id=" << id;
		conn.exec(query.str());

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


