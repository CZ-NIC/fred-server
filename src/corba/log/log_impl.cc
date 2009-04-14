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
#include "manage_part_table.h"

#include <fstream>
#include <iostream>
#include <ctime>

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

/* TODO remove
#ifdef HAVE_LOGGER
	#define LOG_CTX_INIT()			\
	Logging::Context::clear();		\
	Logging::Context ctx("logd");	\

#else
#define LOG_CTX_INIT()
#endif
*/

using namespace Database;

const std::string Impl_Log::LAST_PROPERTY_VALUE_ID = "select currval('log_property_value_id_seq'::regclass)";
const std::string Impl_Log::LAST_PROPERTY_NAME_ID = "select currval('log_property_name_id_seq'::regclass)";
const std::string Impl_Log::LAST_ENTRY_ID = "select currval('log_entry_id_seq'::regclass)";
const std::string Impl_Log::LAST_SESSION_ID = "select currval('log_session_id_seq'::regclass)";
const int Impl_Log::MAX_NAME_LENGTH = 30;


inline void log_ctx_init()
{
#ifdef HAVE_LOGGER
	Logging::Context::clear();		
	Logging::Context ctx("logd");	
#endif
}

// Impl_Log ctor: connect to the database and fill property_names map
Impl_Log::Impl_Log(const std::string database, const std::string &monitoring_hosts_file)
      throw (DB_CONNECT_FAILED) : db_manager(new ConnectionFactory(database))
{
    	std::auto_ptr<Connection> conn;
    	std::ifstream file;

  	log_ctx_init();

	try {
		conn.reset(db_manager.getConnection());
	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(boost::format("cannot connect to database %1% : %2%") % database.c_str() % ex.what());
#endif
	}
#ifdef HAVE_LOGGER
	LOGGER("fred-server").notice(boost::format("successfully  connect to DATABASE %1%") % database.c_str());
#endif

	// set constraint exclusion (needed for faster queries on partitioned tables)
	try {
			conn->exec("set constraint_exclusion=on");
	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error( boost::format("couldn't set constraint exclusion on database %1% : %2%") % database.c_str() % ex.what());
#endif
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
		Result res = conn->exec("select id, name from log_property_name");

		if (res.size() > PROP_NAMES_SIZE_LIMIT) {
#ifdef HAVE_LOGGER
			LOGGER("fred-server").error(
					" Number of entries in log_property_name is over the limit.");
#endif
			return;
		}

		for (Result::Iterator it = res.begin(); it != res.end(); ++it) {
			Row row = *it;
			property_names[row[1]] = row[0];
		}

	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(ex.what());
#endif
	}
}

Impl_Log::~Impl_Log() {
  // db.Disconnect();
  log_ctx_init();

#ifdef HAVE_LOGGER
  LOGGER("fred-server").notice("Impl_Log destructor");
#endif
}

// check if a log record with the specified ID exists and if it can be modified (time_end isn't set yet)
bool Impl_Log::record_check(ID id, Connection &conn)
{
	boost::format query = boost::format("select time_end from log_entry where id=%1%") % id;
	Result res = conn.exec(query.str());

	// if there is no record with specified ID
	if(res.size() == 0) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(boost::format("record in log_entry with ID %1% doesn't exist") % id);
#endif
		return false;
	}

	if(!res[0][0].isnull()) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(boost::format("record with ID %1% was already completed") % id);
#endif
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
			conn.exec( (boost::format("insert into log_property_name (name) values ('%1%')") % s_name).str() );

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
void Impl_Log::insert_props(std::string entry_time, ID entry_id, const LogProperties& props, Connection &conn)
{
	std::string s_val;
	ID name_id, last_id = 0;

	if(props.length() == 0) {
		return;
	}

	// process the first record
	s_val = Util::escape(props[0].value.c_str());
	name_id = find_property_name_id(props[0].name, conn);

	if (props[0].child) {
		// the first property is set to child - this is an error
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(boost::format("entry ID %1%: first property marked as child. Ignoring this flag ") % entry_id);
#endif

	}
	boost::format query = boost::format("insert into log_property_value (entry_time_begin, entry_id, name_id, value, output, parent_id) values ('%1%', %2%, %3%, '%4%', %5%, %6%)")
			% entry_time % entry_id % name_id % s_val % (props[0].output ? "true" : "false") % "null" ;

	conn.exec(query.str());

	// obtain last_id
	last_id = find_last_property_value_id(conn);

	// process the rest of the sequence
	for (unsigned i = 1; i < props.length(); i++) {

		s_val = Util::escape(props[i].value.c_str());
		name_id = find_property_name_id(props[i].name, conn);

		if(props[i].child) {
			// child property set and parent id available
			query % entry_time % entry_id % name_id % s_val % (props[i].output ? "true" : "false") % last_id;

			conn.exec(query.str());
		} else {
			// not a child property
			query % entry_time % entry_id % name_id % s_val % (props[i].output ? "true" : "false") % "null";

			conn.exec(query.str());

			last_id = find_last_property_value_id(conn);
		}
	}
}

// log a new event, return the database ID of the record
ID Impl_Log::i_new_event(const char *sourceIP, LogServiceType service, const char *content_in, const LogProperties& props, int action_type)
{
	std::auto_ptr<Connection> conn(db_manager.getConnection());
  	log_ctx_init();

	std::string time, s_sourceIP, s_content;
	ID entry_id;
	boost::posix_time::ptime micro_time;
	tm str_time;

	// get formatted UTC with microseconds
	micro_time = microsec_clock::universal_time();
	time = boost::posix_time::to_iso_string(micro_time);

	str_time = boost::posix_time::to_tm(micro_time);

#ifdef _TESTING_
	/* TODO -this is only a test.... */
	check_and_create_all(*conn, (1900 + str_time.tm_year), str_time.tm_mday);

/*
	if(!exist_tables(*conn, (1900 + str_time.tm_year), str_time.tm_mday)) {
		// TODO this doesn't work (especially for testing) - these two functions are not compatible
		create_table_set(*conn, (1900 + str_time.tm_year), str_time.tm_mday);
	}
*/
#else 
	// TODO - this should be used
	check_and_create_all(*conn, (1900 + str_time.tm_year), str_time.tm_mon); 

/*
	if(!exist_tables(*conn, (1900 + str_time.tm_year), str_time.tm_mon)) {
		// TODO this doesn't work (especially for testing) - these two functions are not compatible
		create_table_set(*conn, (1900 + str_time.tm_year), str_time.tm_mon);
	}
*/

#endif

	std::list<std::string>::iterator it;

	std::string monitoring = std::string("false");
	for (it = monitoring_ips.begin(); it != monitoring_ips.end(); it++) {
		if(sourceIP == *it) {
			monitoring = std::string("true");
			break;
		}
	}

	try {
		boost::format insert_log_entry;
		// Transaction t(conn);
		if(sourceIP != NULL && sourceIP[0] != '\0') {
			// make sure these values can be safely used in an SQL statement
			s_sourceIP = Util::escape(std::string(sourceIP));
			insert_log_entry = boost::format("insert into log_entry (time_begin, source_ip, service, action_type, is_monitoring) values ('%1%', '%2%', %3%, %4%, %5%) ") % time % s_sourceIP % service % action_type % monitoring;
		} else {
			insert_log_entry = boost::format("insert into log_entry (time_begin, service, action_type, is_monitoring) values ('%1%', %2%, %3%, %4%) ") % time % service % action_type % monitoring;
		}
		conn->exec(insert_log_entry.str());

		entry_id = find_last_log_entry_id(*conn);

	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(ex.what());
#endif
		return 0;
	}

	try {
		boost::format insert_raw;
		// insert into log_raw_content
		if(content_in != NULL && content_in[0] != '\0') {
			s_content = Util::escape(std::string(content_in));
		
			insert_raw = boost::format("insert into log_raw_content (entry_time_begin, entry_id, content, is_response) values ('%1%', %2%, E'%3%', %4%)") % time % entry_id % s_content % "false";

			conn->exec(insert_raw.str());
		}

		// inserting properties
		insert_props(time, entry_id, props, *conn);

	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(ex.what());
#endif
	}

	// t.commit();
	return entry_id;
}

// update existing log record with given ID
bool Impl_Log::i_update_event(ID id, const LogProperties &props)
{
	std::auto_ptr<Connection> conn(db_manager.getConnection());

  	log_ctx_init();

	try {
		// perform check
		if (!record_check(id, *conn)) return false;

		// TODO this is temporary workaround for partitioing
		//  - we need the time_begin of the entry in question in order for log_property_value to find the right partition
		//  or do it in some other way
		boost::format query = boost::format("select time_begin from log_entry where id = %1%") % id;
		Result res = conn->exec(query.str());

		if(res.size() == 0) {
#ifdef HAVE_LOGGER
			LOGGER("fred-server").error("Impossible has just happened... ");
#endif
		}
		// end of TODO

		insert_props((std::string)res[0][0], id, props, *conn);
	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(ex.what());
#endif
		return false;
	}
	return true;

}

// close the record with given ID (end time is filled thus no further modification is possible after this call )
bool Impl_Log::i_update_event_close(ID id, const char *content_out, const LogProperties &props)
{
	std::auto_ptr<Connection> conn(db_manager.getConnection());

	log_ctx_init();

	std::string s_content, time;

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	try {
		// first perform checks:
		if (!record_check(id, *conn)) return false;

		// TODO this is temporary workaround for partitioing
		boost::format select = boost::format("select time_begin from log_entry where id = %1%") % id;
		Result res = conn->exec(select.str());

		if(res.size() == 0) {
#ifdef HAVE_LOGGER
			LOGGER("fred-server").error("Impossible has just happened... ");
#endif
		}
		// end of TODO

		boost::format update = boost::format("update log_entry set time_end=E'%1%' where id=%2%") % time % id;
		conn->exec(update.str());

		if(content_out != NULL) {
			s_content = Util::escape(std::string(content_out));

			boost::format insert = boost::format( "insert into log_raw_content (entry_time_begin, entry_id, content, is_response) values ('%1%', %2%, E'%3%', %4%) ") % (std::string)res[0][0] % id % s_content % "true";
			conn->exec(insert.str());
		}

		// inserting properties
		insert_props((std::string)res[0][0], id, props, *conn);

	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(ex.what());
#endif
		return false;
	}
	return true;
}

ID Impl_Log::i_new_session(Languages lang, const char *name, const char *clTRID)
{
	std::auto_ptr<Connection> conn(db_manager.getConnection());
	log_ctx_init();

	std::string s_name, s_clTRID, time;
	ID id;

#ifdef HAVE_LOGGER
	LOGGER("fred-server").notice(boost::format("new_session: username-> [%1%] clTRID [%2%] lang [%3%]") % name % clTRID % lang);
#endif

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	if (name != NULL && *name != '\0') {
		s_name = Util::escape(name);
	} else {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error("new_session: name is empty!");
#endif
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
		conn->exec(insert.str());

		Result res = conn->exec(LAST_SESSION_ID);

		id = res[0][0];

		if (lang == CS) {
			boost::format query = boost::format("update log_session set lang = 'cs' where id=%1%") % id;
			conn->exec(query.str());
		}

	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(ex.what());
#endif
		return 0;
	}
	return id;

}

bool Impl_Log::i_end_session(ID id, const char *clTRID)
{
	std::auto_ptr<Connection> conn(db_manager.getConnection());

	log_ctx_init();
	std::string  time;

#ifdef HAVE_LOGGER
	LOGGER("fred-server").notice(boost::format("end_session: session_id -> [%1%] clTRID [%2%]") % id  % clTRID );
#endif

	boost::format query = boost::format("select logout_date from log_session where id=%1%") % id;

	try {
		Result res = conn->exec(query.str());

		if(res.size() == 0) {
#ifdef HAVE_LOGGER
			LOGGER("fred-server").error(boost::format("record in log_session with ID %1% doesn't exist") % id);
#endif
			return false;
		}

		if(!res[0][0].isnull()) {
#ifdef HAVE_LOGGER
			LOGGER("fred-server").error(boost::format("record in log_session with ID %1% already closed") % id);
#endif
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

		conn->exec(update.str());
	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(ex.what());
#endif
		return false;
	}

	return true;
}


