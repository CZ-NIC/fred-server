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

#include "model_log_action_type.h"
#include "model_log_entry.h"
#include "model_log_property_name.h"
#include "model_log_property_value.h"
#include "model_log_raw_content.h"
#include "model_log_session.h"

#include <fstream>
#include <iostream>

#include <stdlib.h>

// logger
#include "old_utils/log.h"
//config
#include "old_utils/conf.h"
// util (for escape)
#include "util.h"

// FRED logging
#include "log/logger.h"
#include "log/context.h"

using namespace Database;

inline void log_ctx_init()
{
#ifdef HAVE_LOGGER
	Logging::Context::clear();		
	Logging::Context ctx("logd");	
#endif
}

inline void logger_notice(const char *str);
inline void logger_error(const char *str); 
inline void logger_notice(boost::format fmt);
inline void logger_error(boost::format fmt);

inline void logger_notice(const char *str) 
{
	logger_notice(boost::format(str));
}

inline void logger_error(const char *str) 
{
	logger_error(boost::format(str));
}

inline void logger_notice(boost::format fmt) 
{
#ifdef HAVE_LOGGER
	LOGGER("fred-server").notice(fmt);
#endif
}

inline void logger_error(boost::format fmt) 
{
#ifdef HAVE_LOGGER
	LOGGER("fred-server").error(fmt);
#endif
}

// Impl_Log ctor: connect to the database and fill property_names map
Impl_Log::Impl_Log(const std::string database, const std::string &monitoring_hosts_file)
      throw (DB_CONNECT_FAILED) 
{
    	std::ifstream file;

  	log_ctx_init();

	try {
		Manager::init(new ConnectionFactory(database));
	} catch (Database::Exception &ex) {
		logger_error(boost::format("cannot connect to database %1% : %2%") % database.c_str() % ex.what());
	}
	
	Database::Connection conn = Manager::acquire();	

	logger_notice(boost::format("successfully  connect to DATABASE %1%") % database.c_str());

	// set constraint exclusion (needed for faster queries on partitioned tables)
	try {
		// TODO this could be probably done elsewhere (maybe database create sql script) - find out
		conn.exec("set constraint_exclusion=on");
	} catch (Database::Exception &ex) {
		logger_error(boost::format("couldn't set constraint exclusion on database %1% : %2%") % database.c_str() % ex.what());
	}

	if (!monitoring_hosts_file.empty()) {
		try {
			file.open(monitoring_hosts_file.c_str());
			file.exceptions(std::ios::badbit | std::ios::failbit);

			while(file) {
				std::string input;
				file >> input;
				monitoring_ips.push_back(input);
			}

		} catch(std::exception &e) {
			LOGGER("fred-server").error(boost::format("Error while reading config file %1% : %2%") % monitoring_hosts_file % e.what());
		}
	}

	// now fill the property_names map:

	try {
		Result res = conn.exec("select id, name from log_property_name");

		if (res.size() > PROP_NAMES_SIZE_LIMIT) {
			logger_error(" Number of entries in log_property_name is over the limit.");

			return;
		}

		for (Result::Iterator it = res.begin(); it != res.end(); ++it) {
			Row row = *it;
			property_names[row[1]] = row[0];
		}

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
	}
}

Impl_Log::~Impl_Log() {
  // db.Disconnect();
  log_ctx_init();

  logger_notice("Impl_Log destructor");
}

// check if a log record with the specified ID exists and if it can be modified (time_end isn't set yet)
bool Impl_Log::record_check(ID id, Connection &conn)
{
	boost::format query = boost::format("select time_end from log_entry where id=%1%") % id;
	Result res = conn.exec(query.str());

	// if there is no record with specified ID
	if(res.size() == 0) {
		logger_error(boost::format("record in log_entry with ID %1% doesn't exist") % id);
		return false;
	}

	if(!res[0][0].isnull()) {
		logger_error(boost::format("record with ID %1% was already completed") % id);
		return false;
	}

	return true;
}

// find ID for the given name of a property
// if the name is tool long, it's truncated to the maximal length allowed by the database
ID Impl_Log::find_property_name_id(const std::string &name, Connection &conn)
{
	ID name_id;
	std::map<std::string, ID>::iterator iter;

	std::string name_trunc = name.substr(0, MAX_NAME_LENGTH);

	iter = property_names.find(name_trunc);

	if(iter != property_names.end()) {
		name_id = iter->second;
	} else {
		// if the name isn't cached in the memory, try to find it in the database
		std::string s_name = Util::escape(name_trunc);

		boost::format query = boost::format("select id from log_property_name where name='%1%'") % s_name;
		Result res = conn.exec(query.str());

		if (res.size() > 0) {
			// okay, it was found in the database
			name_id = res[0][0];
		} else if (res.size() == 0) {
			// if not, add it to the database
			ModelLogPropertyName pn;
			pn.setName(name_trunc);
			
			pn.insert();
			res = conn.exec(LAST_PROPERTY_NAME_ID);
			name_id = res[0][0];
		}

		// now that we know the right database id of the name
		// we can add it to the map
		property_names[name_trunc] = name_id;
	}

	return name_id;
}

/**
 * Find last ID used in log_property_value table
 */
inline ID Impl_Log::find_last_property_value_id(Connection &conn)
{
	Result res = conn.exec(LAST_PROPERTY_VALUE_ID);
	return res[0][0];
}

inline ID Impl_Log::find_last_log_entry_id(Connection &conn)
{
	Result res = conn.exec(LAST_ENTRY_ID);
	return res[0][0];
}

// insert properties for the given log_entry record
void Impl_Log::insert_props(DateTime entry_time, ID entry_id, const LogProperties& props, Connection conn)
{
	std::string s_val;
	ID name_id, last_id = 0;

	if(props.size() == 0) {
		return;
	}

	// process the first record	
	name_id = find_property_name_id(props[0].name, conn);

	if (props[0].child) {
		// the first property is set to child - this is an error
		logger_error(boost::format("entry ID %1%: first property marked as child. Ignoring this flag ") % entry_id);

	}
		
	ModelLogPropertyValue pv_first;
	pv_first.setEntryTimeBegin(entry_time);
	pv_first.setEntryId(entry_id);
	pv_first.setNameId(name_id);
	pv_first.setValue (props[0].value);
	pv_first.setOutput(props[0].output);
	
	pv_first.insert();

	last_id = find_last_property_value_id(conn);

	// process the rest of the sequence
	for (unsigned i = 1; i < props.size(); i++) {
		name_id = find_property_name_id(props[i].name, conn);
		
		// create a new object for each iteration
		// because ParentId must alternate between NULL and some value
		ModelLogPropertyValue pv;
		pv.setEntryTimeBegin(entry_time);
		pv.setEntryId(entry_id);
		pv.setNameId(name_id);
		pv.setValue (props[i].value);
		pv.setOutput(props[i].output);

		if(props[i].child) {
			pv.setParentId(last_id);
			pv.insert();
		} else {
			pv.insert();		
			last_id = find_last_property_value_id(conn);
		}
	}
}

// log a new event, return the database ID of the record
ID Impl_Log::i_new_event(const char *sourceIP, LogServiceType service, const char *content_in, const LogProperties& props, int action_type)
{
	Connection conn = Manager::acquire();
  	log_ctx_init();

	ID entry_id;

	// get UTC with microseconds
	DateTime time(microsec_clock::universal_time());

	std::list<std::string>::iterator it;
	
	ModelLogEntry le;
	le.setIsMonitoring(false);
	for (it = monitoring_ips.begin(); it != monitoring_ips.end(); it++) {
		if(sourceIP == *it) {
			le.setIsMonitoring(true);
			break;
		}
	}
	
	le.setTimeBegin(time);
	// watch out, these 2 values are passed by reference
	le.setService(service);
	le.setActionTypeId(action_type);
	
	try {		
		if(sourceIP != NULL && sourceIP[0] != '\0') {
			// make sure these values can be safely used in an SQL statement			
			le.setSourceIp(sourceIP);			
		}
		
		le.insert();		
		entry_id = find_last_log_entry_id(conn);
		
	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return 0;
	} catch (ConversionError &ex) {
		std::cout << "Exception text: " << ex.what() << std::endl;
		std::cout << "Here it is. Time: "  << std::endl;
	}

	try {
		ModelLogRawContent raw;
		
		boost::format insert_raw;
		// insert into log_raw_content
		if(content_in != NULL && content_in[0] != '\0') {					
			raw.setEntryTimeBegin(time);
			raw.setEntryId(entry_id);
			raw.setContent(content_in);
			raw.setIsResponse(false);
			
			raw.insert();	
		}

		// inserting properties
		insert_props(time, entry_id, props, conn);

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
	}

	// t.commit();
	return entry_id;
}

// update existing log record with given ID
bool Impl_Log::i_update_event(ID id, const LogProperties &props)
{
	Connection conn = Manager::acquire();

  	log_ctx_init();

	try {
		// perform check
		if (!record_check(id, conn)) return false;

		// TODO this is temporary workaround for partitioing
		//  - we need the time_begin of the entry in question in order for log_property_value to find the right partition
		//  or do it in some other way
		boost::format query = boost::format("select time_begin from log_entry where id = %1%") % id;
		Result res = conn.exec(query.str());

		if(res.size() == 0) {
			logger_error("Impossible has just happened... ");
		}
		// end of TODO

		DateTime time = res[0][0].operator ptime();
		insert_props(time, id, props, conn);
	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}
	return true;

}

// close the record with given ID (end time is filled thus no further modification is possible after this call )
bool Impl_Log::i_update_event_close(ID id, const char *content_out, const LogProperties &props)
{
	Connection conn = Manager::acquire();

	log_ctx_init();

	std::string s_content, time;

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	try {
		// first perform checks:
		if (!record_check(id, conn)) return false;

		// TODO this is temporary workaround for partitioing
		boost::format select = boost::format("select time_begin from log_entry where id = %1%") % id;
		Result res = conn.exec(select.str());

		if(res.size() == 0) {
			logger_error("Impossible has just happened... ");
		}
		// end of TODO

		boost::format update = boost::format("update log_entry set time_end=E'%1%' where id=%2%") % time % id;
		conn.exec(update.str());

		if(content_out != NULL) {
			s_content = Util::escape(std::string(content_out));

			boost::format insert = boost::format( "insert into log_raw_content (entry_time_begin, entry_id, content, is_response) values ('%1%', %2%, E'%3%', %4%) ") % (std::string)res[0][0] % id % s_content % "true";
			conn.exec(insert.str());
		}

		// inserting properties
		insert_props(res[0][0].operator ptime(), id, props, conn);

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}
	return true;
}

ID Impl_Log::i_new_session(Languages lang, const char *name, const char *clTRID)
{
	Connection conn = Manager::acquire();
	log_ctx_init();

	std::string s_name, s_clTRID, time;
	ID id;

	logger_notice(boost::format("new_session: username-> [%1%] clTRID [%2%] lang [%3%]") % name % clTRID % lang);

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	if (name != NULL && *name != '\0') {
		s_name = Util::escape(name);
	} else {
		logger_error("new_session: name is empty!");
		return 0;
	}
	
	boost::format insert;
	if (clTRID != NULL && *clTRID != '\0') {
		s_clTRID = Util::escape(clTRID);
		insert = boost::format ("insert into log_session (name, login_date, login_TRID) values (E'%1%', '%2%', E'%3%')")
				% s_name % time % s_clTRID;
	} else {
		insert = boost::format ("insert into log_session (name, login_date) values (E'%1%', '%2%')") % s_name % time;
	}

	try {
		conn.exec(insert.str());

		Result res = conn.exec(LAST_SESSION_ID);

		id = res[0][0];

		if (lang == CS) {
			boost::format query = boost::format("update log_session set lang = 'cs' where id=%1%") % id;
			conn.exec(query.str());
		}

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return 0;
	}
	return id;

}

bool Impl_Log::i_end_session(ID id, const char *clTRID)
{
	Connection conn = Manager::acquire();

	log_ctx_init();
	std::string  time;

	logger_notice(boost::format("end_session: session_id -> [%1%] clTRID [%2%]") % id  % clTRID );

	boost::format query = boost::format("select logout_date from log_session where id=%1%") % id;

	try {
		Result res = conn.exec(query.str());

		if(res.size() == 0) {
			logger_error(boost::format("record in log_session with ID %1% doesn't exist") % id);
			return false;
		}

		if(!res[0][0].isnull()) {
			logger_error(boost::format("record in log_session with ID %1% already closed") % id);
			return false;
		}

		boost::format update;
		time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

		if (clTRID != NULL && clTRID != '\0') {
			std::string s_clTRID;
			s_clTRID = Util::escape(clTRID);
			update = boost::format("update log_session set (logout_date, logout_TRID) = ('%1%', E'%2%') where id=%3%")
					% time % s_clTRID % id;
		} else {
			update = boost::format("update log_session set logout_date = '%1%' where id=%2%") % time % id;
		}

		conn.exec(update.str());
	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}

	return true;
}

const std::string Impl_Log::LAST_PROPERTY_VALUE_ID = "select currval('log_property_value_id_seq'::regclass)";
const std::string Impl_Log::LAST_PROPERTY_NAME_ID = "select currval('log_property_name_id_seq'::regclass)";
const std::string Impl_Log::LAST_ENTRY_ID = "select currval('log_entry_id_seq'::regclass)";
const std::string Impl_Log::LAST_SESSION_ID = "select currval('log_session_id_seq'::regclass)";
const int Impl_Log::MAX_NAME_LENGTH = 30;



