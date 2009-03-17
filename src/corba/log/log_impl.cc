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


// logger
#include "old_utils/log.h"
//config
#include "old_utils/conf.h"
// util (for escape)
#include "util.h"

// FRED logging
#include "log/logger.h"
#include "log/context.h"

#ifdef HAVE_LOGGER
	#define LOG_CTX_INIT()			\
	Logging::Context::clear();		\
	Logging::Context ctx("logd");	\

#else
#define LOG_CTX_INIT()
#endif


using namespace Database;

const std::string Impl_Log::LAST_PROPERTY_VALUE_ID = "select currval('log_property_value_id_seq'::regclass)";
const std::string Impl_Log::LAST_PROPERTY_NAME_ID = "select currval('log_property_name_id_seq'::regclass)";
const std::string Impl_Log::LAST_ENTRY_ID = "select currval('log_entry_id_seq'::regclass)";
const std::string Impl_Log::LAST_SESSION_ID = "select currval('log_session_id_seq'::regclass)";

// Impl_Log ctor: connect to the database and fill property_names map
Impl_Log::Impl_Log(const std::string database)
      throw (DB_CONNECT_FAILED) : db_manager(new ConnectionFactory(database)) {

    std::auto_ptr<Connection> conn;

  	LOG_CTX_INIT();

	try {
		conn.reset(db_manager.getConnection());
	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error( boost::format("cannot connect to database %1% : %2%") % database.c_str() % ex.what());
#endif
	}

#ifdef HAVE_LOGGER
	LOGGER("fred-server").notice(boost::format("successfully  connect to DATABASE %1%") % database.c_str());
#endif

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
  LOG_CTX_INIT()

#ifdef HAVE_LOGGER
  LOGGER("fred-server").notice("Impl_Log destructor");
#endif
}

// check if a log record with the specified ID exists and if it can be modified (time_end isn't set yet)
bool Impl_Log::record_check(ID id, Connection &conn)
{
	std::ostringstream query;

	query << "select time_end from log_entry where id=" << id;
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
ID Impl_Log::find_property_name_id(const std::string &name, Connection &conn)
{
	ID name_id;
	std::ostringstream query;
	std::map<std::string, ID>::iterator iter;

	

	iter = property_names.find(name);

	if(iter != property_names.end()) {
		name_id = iter->second;
	} else {
		// if the name isn't cached in the memory, try to find it in the database
		std::string s_name = Util::escape(name);

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
			query << LAST_PROPERTY_NAME_ID;
			res = conn.exec(query.str());

			name_id = res[0][0];
		}

		// now that we know the right database id of the name
		// we can add it to the map
		property_names[name] = name_id;
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
void Impl_Log::insert_props(ID entry_id, const LogProperties& props, Connection &conn)
{
	std::string s_val;
	std::ostringstream query;
	ID name_id, last_id = 0;

	if(props.length() == 0) {
		return;
	}

	// process the first record
	s_val = Util::escape(props[0].value.c_str());
	name_id = find_property_name_id(props[0].name, conn);

	query.str("");
	if (props[0].child) {
		std::ostringstream msg;
		msg << "entry ID " << entry_id << ": first property marked as child. Ignoring this flag ";
		// the first property is set to child - this is an error
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(msg.str());
#endif

	}
	query   << "insert into log_property_value (entry_id, name_id, value, output, parent_id) values ("
			<< entry_id << ", " << name_id << ", '" << s_val << "', "
			<< (props[0].output ? "true" : "false") << ", null)";
	conn.exec(query.str());

	// obtain last_id
	last_id = find_last_property_value_id(conn);

	// process the rest of the sequence
	for (unsigned i = 1; i < props.length(); i++) {

		s_val = Util::escape(props[i].value.c_str());
		name_id = find_property_name_id(props[i].name, conn);

		query.str("");
		if(props[i].child) {
			// child property set and parent id available
			query   << "insert into log_property_value (entry_id, name_id, value, output, parent_id) values ("
					<< entry_id << ", " << name_id << ", '" << s_val << "', "
					<< (props[i].output ? "true" : "false") << ", " << last_id << ")";

			conn.exec(query.str());
		} else {
			// not a child property
			query   << "insert into log_property_value (entry_id, name_id, value, output, parent_id) values ("
					<< entry_id << ", " << name_id << ", '" << s_val << "', "
					<< (props[i].output ? "true" : "false") << ", null)";
			conn.exec(query.str());

			last_id = find_last_property_value_id(conn);
		}
	}
}

// log a new event, return the database ID of the record
ID Impl_Log::i_new_event(const char *sourceIP, LogServiceType service, const char *content_in, const LogProperties& props)
{
	std::auto_ptr<Connection> conn(db_manager.getConnection());
  	LOG_CTX_INIT()

	std::ostringstream query;
	std::string time, s_sourceIP, s_content;
	ID entry_id;

	// get formatted UTC with microseconds
	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	try {
		// Transaction t(conn);
		if(sourceIP != NULL && sourceIP[0] != '\0') {
			// make sure these values can be safely used in an SQL statement
			s_sourceIP = Util::escape(std::string(sourceIP));
			query << "insert into log_entry (time_begin, source_ip, service) values ('" << time << "', '" << s_sourceIP << "', " << service << ")";
		} else {
			query << "insert into log_entry (time_begin, service) values ('"
				<< time << "', " << service << ")";
		}
		conn->exec(query.str());

		// get the id of the new entry
		query.str("");
		// query << "select id from log_entry where time_begin='" << time << "'";

		entry_id = find_last_log_entry_id(*conn);

	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(ex.what());
#endif
		return 0;
	}

	try {
		// "select

		// insert into log_raw_content
		if(content_in != NULL && content_in[0] != '\0') {
			s_content = Util::escape(std::string(content_in));
			query.str("");
			query << "insert into log_raw_content (entry_id, content, is_response) values (" << entry_id << ", E'" << s_content << "', false)";

			conn->exec(query.str());
		}

		// inserting properties
		insert_props(entry_id, props, *conn);

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
	std::ostringstream query;

  	LOG_CTX_INIT()

	try {
		// perform check
		if (!record_check(id, *conn)) return false;

		insert_props(id, props, *conn);
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

	LOG_CTX_INIT()

	std::ostringstream query;
	std::string s_content, time;

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	try {
		// first perform checks:
		if (!record_check(id, *conn)) return false;

		query << "update log_entry set time_end=E'" << time << "' where id=" << id;
		conn->exec(query.str());

		if(content_out != NULL) {
			s_content = Util::escape(std::string(content_out));

			query.str("");

			query << "insert into log_raw_content (entry_id, content, is_response) values (" << id << ", E'" << s_content << "', true)";
			conn->exec(query.str());
		}

		// inserting properties
		insert_props(id, props, *conn);

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
	LOG_CTX_INIT()

	std::ostringstream query;
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

	if (clTRID != NULL && *clTRID != '\0') {
		s_clTRID = Util::escape(clTRID);
		query << "insert into log_session (name, login_date, login_TRID) values (E'"
		<< s_name << "', '" << time << "', E'" << s_clTRID << "') ";
	} else {
		query << "insert into log_session (name, login_date) values (E'" << s_name << "', '" << time << "') ";
	}

	try {
		conn->exec(query.str());

		Result res = conn->exec(LAST_SESSION_ID);

		id = res[0][0];

		if (lang == CS) {
			query.str("");
			query << "update log_session set lang = 'cs' where id=" << id;
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

	LOG_CTX_INIT()

	std::ostringstream query;
	std::string  time;

#ifdef HAVE_LOGGER
	LOGGER("fred-server").notice(boost::format("end_session: session_id -> [%1%] clTRID [%2%]") % id  % clTRID );
#endif

	query << "select logout_date from log_session where id=" << id;

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

		query.str("");
		time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

		if (clTRID != NULL && clTRID != '\0') {
			std::string s_clTRID;
			s_clTRID = Util::escape(clTRID);
			query << "update log_session set (logout_date, logout_TRID) = ('" <<
			time << "', E'" << s_clTRID << "') where id=" << id;
		} else {
			query << "update log_session set logout_date = '" << time << "' where id=" << id;
		}

		conn->exec(query.str());
	} catch (Database::Exception &ex) {
#ifdef HAVE_LOGGER
		LOGGER("fred-server").error(ex.what());
#endif
		return false;
	}

	return true;
}


