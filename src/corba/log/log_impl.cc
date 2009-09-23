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

#include "model_request_type.h"
#include "model_request.h"
#include "model_request_property.h"
#include "model_request_property_value.h"
#include "model_request_data.h"
#include "model_session.h"
#include "model_service.h"

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


/**
 *  Connection class designed for use in fred-logd with partitioned tables
 *  Class used to acquire database connection from the Manager, set constraint_exclusion parameter and explicitly release the connection in dtor
 *  It assumes that the system error logger is initialized (use of logger_error() function
 */
class logd_auto_conn : public Connection {
public:
	explicit logd_auto_conn() : Connection(Manager::acquire()) {

		// set constraint exclusion (needed for faster queries on partitioned tables)
		try {
			exec("set constraint_exclusion=on");
		} catch (Database::Exception &ex) {
			logger_error(boost::format("couldn't set constraint exclusion : %2%") % ex.what());
		}

	};

	~logd_auto_conn() {
		Manager::release();
	}
};

Result Impl_Log::i_GetServiceActions(RequestServiceType service)
{
	logd_auto_conn conn;
	log_ctx_init();

	boost::format query = boost::format("select id, status from request_type where service = %1%") % service;

	try {
		return conn.exec(query.str());
	} catch (Database::Exception &ex) {
		logger_error(ex.what());
	}
}

// Impl_Log ctor: connect to the database and fill property_names map
Impl_Log::Impl_Log(const std::string database, const std::string &monitoring_hosts_file)
      throw (DB_CONNECT_FAILED)
{
    	std::ifstream file;

  	log_ctx_init();

	try {
		Manager::init(new ConnectionFactory(database, 4, 20));
	} catch (Database::Exception &ex) {
		logger_error(boost::format("cannot connect to database %1% : %2%") % database.c_str() % ex.what());
	}

	logd_auto_conn conn;

	logger_notice(boost::format("successfully  connect to DATABASE %1%") % database.c_str());

	if (!monitoring_hosts_file.empty()) {
		try {
			file.open(monitoring_hosts_file.c_str());
			// TODO
			// Error while reading config file test_log_monitoring.conf : basic_ios::clear
			// file.exceptions(std::ios::badbit | std::ios::failbit);

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
		Result res = conn.exec("select id, name from request_property");

		if (res.size() > PROP_NAMES_SIZE_LIMIT) {
			logger_error(" Number of entries in request_property is over the limit.");

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
  log_ctx_init();

  logger_notice("Impl_Log destructor");
}

// check if a log record with the specified ID exists and if it can be modified (time_end isn't set yet)
bool Impl_Log::record_check(ID id, Connection &conn)
{
	boost::format query = boost::format("select time_end from request where id=%1%") % id;
	Result res = conn.exec(query.str());

	// if there is no record with specified ID
	if(res.size() == 0) {
		logger_error(boost::format("record in request with ID %1% doesn't exist") % id);
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

		boost::format query = boost::format("select id from request_property where name='%1%'") % s_name;
		Result res = conn.exec(query.str());

		if (res.size() > 0) {
			// okay, it was found in the database
			name_id = res[0][0];
		} else if (res.size() == 0) {
			// if not, add it to the database
			ModelRequestProperty pn;
			pn.setName(name_trunc);

			try {
				pn.insert();
				res = conn.exec(LAST_PROPERTY_NAME_ID);
				name_id = res[0][0];
			} catch (Database::Exception &ex) {
				logger_error(ex.what());
			}
		}

		// now that we know the right database id of the name
		// we can add it to the map
		property_names[name_trunc] = name_id;
	}

	return name_id;
}

/**
 * Find last ID used in request_property_value table
 */
inline ID Impl_Log::find_last_property_value_id(Connection &conn)
{
	Result res = conn.exec(LAST_PROPERTY_VALUE_ID);
	return res[0][0];
}

inline ID Impl_Log::find_last_request_id(Connection &conn)
{
	Result res = conn.exec(LAST_ENTRY_ID);
	return res[0][0];
}

// insert properties for the given request record
void Impl_Log::insert_props(DateTime entry_time, RequestServiceType service, bool monitoring, ID entry_id,  const Register::Logger::RequestProperties& props, Connection conn)
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

	ModelRequestPropertyValue pv_first;
	pv_first.setEntryTimeBegin(entry_time);
	pv_first.setEntryService(service);
	pv_first.setEntryMonitoring(monitoring);
	pv_first.setEntry(entry_id);
	pv_first.setName(name_id);
	pv_first.setValue (props[0].value);
	pv_first.setOutput(props[0].output);

	pv_first.insert();

	last_id = find_last_property_value_id(conn);

	// process the rest of the sequence
	for (unsigned i = 1; i < props.size(); i++) {
		name_id = find_property_name_id(props[i].name, conn);

		// create a new object for each iteration
		// because ParentId must alternate between NULL and some value
		ModelRequestPropertyValue pv;
		pv.setEntryTimeBegin(entry_time);
		pv.setEntryService(service);
		pv.setEntryMonitoring(monitoring);
		pv.setEntry(entry_id);
		pv.setName(name_id);
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
ID Impl_Log::i_CreateRequest(const char *sourceIP, RequestServiceType service, const char *content_in, const Register::Logger::RequestProperties& props, RequestActionType action_type, ID session_id)
{
	logd_auto_conn conn;
  	log_ctx_init();

	ID entry_id;

	// get UTC with microseconds
	DateTime time(microsec_clock::universal_time());

	std::list<std::string>::iterator it;

	ModelRequest req;
	bool monitoring = false;
	for (it = monitoring_ips.begin(); it != monitoring_ips.end(); it++) {
		if(sourceIP == *it) {
			monitoring = true;
			break;
		}
	}

	req.setIsMonitoring(monitoring);
	req.setTimeBegin(time);
	// watch out, these 2 values are passed by reference
	req.setServiceId(service);
	req.setActionTypeId(action_type);
	if (session_id != 0) {
		req.setSessionId(session_id);
	}

	try {
		if(sourceIP != NULL && sourceIP[0] != '\0') {
			// make sure these values can be safely used in an SQL statement
			req.setSourceIp(sourceIP);
		}

		req.insert();
		entry_id = find_last_request_id(conn);

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return 0;
	} catch (ConversionError &ex) {
		std::cout << "Exception text: " << ex.what() << std::endl;
		std::cout << "Here it is. Time: "  << std::endl;
	}

	try {
		ModelRequestData data;

		// insert into request_data
		if(content_in != NULL && content_in[0] != '\0') {
			data.setEntryTimeBegin(time);
			data.setEntryService(service);
			data.setEntryMonitoring(monitoring);
			data.setEntryId(entry_id);
			data.setContent(content_in);
			data.setIsResponse(false);

			data.insert();
		}

		// inserting properties
		insert_props(time, service, monitoring, entry_id, props, conn);

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
	}

	return entry_id;
}

// update existing log record with given ID
bool Impl_Log::i_UpdateRequest(ID id, const Register::Logger::RequestProperties &props)
{
	logd_auto_conn conn;

  	log_ctx_init();

	try {
		// perform check
		if (!record_check(id, conn)) return false;

		// TODO think about some other way - i don't like this
		boost::format query = boost::format("select time_begin, service, is_monitoring from request where id = %1%") % id;
		Result res = conn.exec(query.str());

		if(res.size() == 0) {
			logger_error("Record in request table not found.");
		}
		// end of TODO

		DateTime time = res[0][0].operator ptime();
		RequestServiceType service = (RequestServiceType)(int)res[0][1];
		bool monitoring        = (bool)res[0][2];
		insert_props(time, service, monitoring, id, props, conn);
	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}
	return true;

}

bool Impl_Log::close_request_worker(Connection &conn, ID id, const char *content_out, const Register::Logger::RequestProperties &props)
{
	std::string s_content, time;
	long int service;
	bool monitoring;

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	try {
		// first perform checks:
		if (!record_check(id, conn)) return false;

		// TODO the same case as before
		boost::format select = boost::format("select time_begin, service, is_monitoring from request where id = %1%") % id;
		Result res = conn.exec(select.str());
		DateTime entry_time = res[0][0].operator ptime();
		service = (long int) res[0][1];
		monitoring = (bool)res[0][2];

		if(res.size() == 0) {
			logger_error("Record in request table not found.");
		}
		// end of TODO

		boost::format update = boost::format("update request set time_end=E'%1%' where id=%2%") % time % id;
		conn.exec(update.str());

		if(content_out != NULL && content_out[0] != '\0') {
			ModelRequestData data;

			// insert into request_data
			data.setEntryTimeBegin(entry_time);
			data.setEntryService(service);
			data.setEntryMonitoring(monitoring);
			data.setEntryId(id);
			data.setContent(content_out);
			data.setIsResponse(true);

			data.insert();
		}

		// inserting properties
		insert_props(res[0][0].operator ptime(), service, monitoring, id, props, conn);

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}
	return true;
}




// close the record with given ID (end time is filled thus no further modification is possible after this call )
bool Impl_Log::i_CloseRequest(ID id, const char *content_out, const Register::Logger::RequestProperties &props)
{
	logd_auto_conn conn;

	log_ctx_init();

	return close_request_worker(conn, id, content_out, props);
}


bool Impl_Log::i_CloseRequestLogin(ID id, const char *content_out, const Register::Logger::RequestProperties &props, ID session_id)
{
	bool ret;

	logd_auto_conn conn;

	log_ctx_init();

	ret = close_request_worker(conn, id, content_out, props);
	if (!ret) return false;

	// fill in the session ID

	boost::format query = boost::format("select session_id from request where id=%1%") % id;

	try {
		Result res = conn.exec(query.str());

		// if there is no record with specified ID
		if(res.size() == 0) {
			logger_error(boost::format("record in request with ID %1% doesn't exist") % id);
			return false;
		}

		if(!res[0][0].isnull()) {
			ID filled = res[0][0]; 
			if(filled != 0) {
				logger_error(boost::format("record with ID %1% already has session_id filled") % id);
				return false;
			}
		}

		query = boost::format("update request set session_id = %1% where id=%2%") % session_id % id;
		conn.exec(query.str());

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}

}

ID Impl_Log::i_CreateSession(Languages lang, const char *name)
{
	logd_auto_conn conn;
	log_ctx_init();

	std::string time;
	ID id;

	logger_notice(boost::format("CreateSession: username-> [%1%] lang [%2%]") % name %  lang);

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	ModelSession ns;

	if (name != NULL && *name != '\0') {
		ns.setName(name);
	} else {
		logger_error("CreateSession: name is empty!");
		return 0;
	}

	try {
		ns.insert();

		Result res = conn.exec(LAST_SESSION_ID);

		id = res[0][0];

		if (lang == CS) {
			boost::format query = boost::format("update session set lang = 'cs' where id=%1%") % id;
			conn.exec(query.str());
		}

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return 0;
	}
	return id;
}

bool Impl_Log::i_CloseSession(ID id)
{
	logd_auto_conn conn;

	log_ctx_init();
	std::string  time;

	logger_notice(boost::format("CloseSession: session_id -> [%1%] ") % id );

	boost::format query = boost::format("select logout_date from session where id=%1%") % id;

	try {
		Result res = conn.exec(query.str());

		if(res.size() == 0) {
			logger_error(boost::format("record in session with ID %1% doesn't exist") % id);
			return false;
		}

		if(!res[0][0].isnull()) {
			logger_error(boost::format("record in session with ID %1% already closed") % id);
			return false;
		}

		boost::format update;
		time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

		update = boost::format("update session set logout_date = '%1%' where id=%2%") % time % id;

		conn.exec(update.str());
	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}

	return true;
}

const std::string Impl_Log::LAST_PROPERTY_VALUE_ID = "select currval('request_property_value_id_seq'::regclass)";
const std::string Impl_Log::LAST_PROPERTY_NAME_ID = "select currval('request_property_id_seq'::regclass)";
const std::string Impl_Log::LAST_ENTRY_ID = "select currval('request_id_seq'::regclass)";
const std::string Impl_Log::LAST_SESSION_ID = "select currval('session_id_seq'::regclass)";
const int Impl_Log::MAX_NAME_LENGTH = 30;



