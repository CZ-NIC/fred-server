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
// util (for escape)
#include "util.h"

// FRED logging
#include "log/logger.h"
#include "log/context.h"


using namespace Database;

const std::string ccReg_Log_i::LAST_PROPERTY_VALUE_ID = "select currval('log_property_value_id_seq'::regclass)";
const std::string ccReg_Log_i::LAST_PROPERTY_NAME_ID = "select currval('log_property_name_id_seq'::regclass)";
const std::string ccReg_Log_i::LAST_ENTRY_ID = "select currval('log_entry_id_seq'::regclass)";

// ccReg_Log_i ctor: connect to the database and fill property_names map
ccReg_Log_i::ccReg_Log_i(const std::string database, NameService *ns, Config::Conf& _cfg)
      throw (DB_CONNECT_FAILED) : db_manager(new ConnectionFactory(database)) {
    std::auto_ptr<Connection> conn;

	try {
		conn.reset(db_manager.getConnection());
	} catch (Database::Exception &ex) {
		LOG(ALERT_LOG, "can not connect to DATABASE %s : %s", database.c_str(),
				ex.what());
		// LOGGER("db").error(ex.what());
	}

	LOG(NOTICE_LOG, "successfully  connect to DATABASE %s", database.c_str());

	// now fill the property_names map:

	try {
		Result res = conn->exec("select id, name from log_property_name");

		if (res.size() > PROP_NAMES_SIZE_LIMIT) {
			LOGGER("fred-logd").error(
					" Number of entries in log_property_name is over the limit.");
			return;
		}

		for (Result::Iterator it = res.begin(); it != res.end(); ++it) {
			Row row = *it;
			property_names[row[1]] = row[0];
		}

	} catch (Database::Exception &ex) {
		LOGGER("fred-logd").error(ex.what());
	}

}

ccReg_Log_i::~ccReg_Log_i() {
  // db.Disconnect();

  LOG( ERROR_LOG, "ccReg_Log_i destructor");
}

// check if a log record with the specified ID exists and if it can be modified (time_end isn't set yet)
bool ccReg_Log_i::record_check(ccReg::TID id, Connection &conn)
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
	if(res[0][0].isnull()) {
		LOGGER("fred-logd").error(boost::format("record with ID %1% was already completed") % id);
		return false;
	}

	return true;
}

// find ID for the given name of a property
ccReg::TID ccReg_Log_i::find_property_name_id(const char *name, Connection &conn)
{
	ccReg::TID name_id;
	std::ostringstream query;
	std::map<std::string, ccReg::TID>::iterator iter;

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
inline ccReg::TID ccReg_Log_i::find_last_property_value_id(Connection &conn)
{
	Result res = conn.exec(LAST_PROPERTY_VALUE_ID);
	// TODO faster way how to find last_id....
	return res[0][0];
}

// insert properties for the given log_entry record
void ccReg_Log_i::insert_props(ccReg::TID entry_id, const ccReg::LogProperties& props, Connection &conn)
{
	std::string s_val;
	std::ostringstream query;
	ccReg::TID name_id, last_id = 0;

	if(props.length() == 0) {
		return;
	}

	// process the first record
	s_val = Util::escape((const char*) props[0].value);
	name_id = find_property_name_id(props[0].name, conn);

	query.str("");
	if (props[0].child) {
		std::ostringstream msg;
		msg << "entry ID " << entry_id << ": first property marked as child. Ignoring this flag ";
		// the first property is set to child - this is an error
		LOGGER("fred-logd").error(msg.str());
	}
	query   << "insert into log_property_value (entry_id, name_id, value, output, parent_id) values ("
			<< entry_id << ", " << name_id << ", '" << s_val << "', "
			<< (props[0].output ? "true" : "false") << ", null)";
	conn.exec(query.str());

	// obtain last_id
	last_id = find_last_property_value_id(conn);

	// process the rest of the sequence
	for (unsigned i = 1; i < props.length(); i++) {

		s_val = Util::escape((const char*) props[i].value);
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
ccReg::TID ccReg_Log_i::new_event(const char *sourceIP, ccReg::LogServiceType service, const char *content_in, const ccReg::LogProperties& props)
{
	std::auto_ptr<Connection> conn(db_manager.getConnection());
	Logging::Context ctx("logd");

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
		conn->exec(query.str());

		// get the id of the new entry
		query.str("");
		// query << "select id from log_entry where time_begin='" << time << "'";

		query << LAST_ENTRY_ID;
		Result res = conn->exec(query.str());
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
			query << "insert into log_raw_content (entry_id, content, is_response) values (" << entry_id << ", E'" << s_content << "', false)";

			// LOGGER("fred-logd").debug(query.str());
			conn->exec(query.str());
		}

		// inserting properties
		insert_props(entry_id, props, *conn);

	} catch (Database::Exception &ex) {
		LOGGER("fred-logd").error(ex.what());
	}

	// t.commit();
	return entry_id;
}

// update existing log record with given ID
CORBA::Boolean ccReg_Log_i::update_event(ccReg::TID id, const ccReg::LogProperties &props)
{
	std::auto_ptr<Connection> conn(db_manager.getConnection());
	Logging::Context ctx("logd");

	std::ostringstream query;

	try {
		// perform check
		if (!record_check(id, *conn)) return false;

		insert_props(id, props, *conn);
	} catch (Database::Exception &ex) {
		LOGGER("fred-logd").error(ex.what());
		return false;
	}
	return true;

}

// close the record with given ID (end time is filled thus no further modification is possible after this call )
CORBA::Boolean ccReg_Log_i::update_event_close(ccReg::TID id, const char *content_out, const ccReg::LogProperties &props)
{
	std::auto_ptr<Connection> conn(db_manager.getConnection());
	Logging::Context ctx("logd");

	std::ostringstream query;
	std::string s_content, time;

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	try {
		// first perform checks:
		if (!record_check(id, *conn)) return false;

		query << "update log_entry set time_end='" << time << "' where id=" << id;
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
		LOGGER("fred-logd").error(ex.what());
		return false;
	}
	return true;
}


