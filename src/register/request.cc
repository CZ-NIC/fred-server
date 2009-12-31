#include <algorithm>
#include <boost/utility.hpp>

// TODO probably to remove (only for debugging pritouts
#include <fstream>
#include <iostream>

// logger
#include "old_utils/log.h"
//config
#include "old_utils/conf.h"
// util (for escape)
#include "util.h"


// FRED logging
#include "log/logger.h"
#include "log/context.h"

#include <stdlib.h>

#include "common_impl.h"
#include "request_impl.h"
#include "log/logger.h"

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace Database;

namespace Register {
namespace Logger {

class RequestImpl : public Register::CommonObjectImpl,
                 virtual public Request {
private:
  Database::DateTime time_begin;
  Database::DateTime time_end;
  std::string source_ip;
  // types changed from RequestServiceType and RequestActionType to std::string because pagetable should now return it as strings
  std::string serv_type;
  std::string action_type;
  Database::ID session_id;
  std::string user_name;
  bool is_monitoring;
  std::string raw_request;
  std::string raw_response;
  boost::shared_ptr<RequestProperties> props;

public:
  RequestImpl(Database::ID &_id, Database::DateTime &_time_begin, Database::DateTime &_time_end, std::string &_serv_type, std::string &_source_ip,  std::string &_action_type, Database::ID &_session_id, std::string &_user_name, bool &_is_monitoring, std::string & _raw_request, std::string & _raw_response, std::auto_ptr<RequestProperties>  _props) :
	CommonObjectImpl(_id),
	time_begin(_time_begin),
	time_end(_time_end),
	source_ip(_source_ip),
	serv_type(_serv_type),
	action_type(_action_type),
	session_id(_session_id),
        user_name(_user_name),
	is_monitoring(_is_monitoring),
	raw_request(_raw_request),
	raw_response(_raw_response),
	props(_props) {
  }

  virtual const ptime  getTimeBegin() const {
	return time_begin;
  }
  virtual const ptime  getTimeEnd() const {
	return time_end;
  }
  virtual const std::string& getServiceType() const {
	return serv_type;
  }
  virtual const std::string& getSourceIp() const {
	return source_ip;
  }
  virtual const std::string& getUserName() const {
        return user_name;
  }
  virtual const std::string& getActionType() const {
	return action_type;
  }
  virtual const Database::ID& getSessionId() const {
	return session_id;
  }
  virtual const bool& getIsMonitoring() const {
	return is_monitoring;
  }
  virtual const std::string& getRawRequest() const {
	return raw_request;
  }
  virtual const std::string& getRawResponse() const {
	return raw_response;
  }
  virtual       boost::shared_ptr<RequestProperties> getProperties() {
	return props;
  }
};

COMPARE_CLASS_IMPL(RequestImpl, TimeBegin)
COMPARE_CLASS_IMPL(RequestImpl, TimeEnd)
COMPARE_CLASS_IMPL(RequestImpl, SourceIp)
COMPARE_CLASS_IMPL(RequestImpl, ServiceType)
COMPARE_CLASS_IMPL(RequestImpl, ActionType)
COMPARE_CLASS_IMPL(RequestImpl, SessionId)
COMPARE_CLASS_IMPL(RequestImpl, UserName)
COMPARE_CLASS_IMPL(RequestImpl, IsMonitoring)
COMPARE_CLASS_IMPL(RequestImpl, RawRequest);
COMPARE_CLASS_IMPL(RequestImpl, RawResponse);





class ListImpl : public Register::CommonListImpl,
                 virtual public List {
private:
  Manager *manager_;
  bool partialLoad;

public:
  ListImpl(Manager *_manager) : CommonListImpl(), manager_(_manager), partialLoad(false) {
  }

  virtual Request* get(unsigned _idx) const {
    try {
      Request *request = dynamic_cast<Request*>(data_.at(_idx));
      if (request)
        return request;
      else
        throw std::exception();
    }
    catch (...) {
      throw std::exception();
    }
  }

  virtual void setPartialLoad(bool _partialLoad) {
	partialLoad = _partialLoad;
  }

  // TODO properties should be displayed only in detailed view
  virtual void reload(Database::Filters::Union& _filter) {
    TRACE("[CALL] Register::Request::ListImpl::reload()");
    clear();
    _filter.clearQueries();

	// iterate through all the filters
    bool at_least_one = false;
    Database::SelectQuery id_query;
    Database::Filters::Compound::iterator fit = _filter.begin();
    for (; fit != _filter.end(); ++fit) {
      Database::Filters::Request *mf = dynamic_cast<Database::Filters::Request* >(*fit);
      if (!mf)
        continue;

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect(new Database::Column("id", mf->joinRequestTable(), "DISTINCT"));
      _filter.addQuery(tmp);
      at_least_one = true;
    }
    if (!at_least_one) {
      LOGGER(PACKAGE).error("wrong filter passed for reload!");
      return;
    }

	// make an id query according to the filters
    id_query.limit(load_limit_);
    _filter.serialize(id_query);

    Database::InsertQuery tmp_table_query = Database::InsertQuery(getTempTableName(), id_query);
    LOGGER(PACKAGE).debug(boost::format("temporary table '%1%' generated sql = %2%") % getTempTableName() % tmp_table_query.str());

	// make the actual query for data
    Database::SelectQuery query;

    if(partialLoad) {
	    query.select() << "tmp.id, t_1.time_begin, t_1.time_end, t_3.name, t_1.source_ip, t_2.status, t_1.session_id, t_1.user_name, t_1.is_monitoring";
	    query.from() << getTempTableName() << " tmp join request t_1 on tmp.id=t_1.id join request_type t_2 on t_2.id=t_1.action_type join service t_3 on t_3.id=t_1.service";
	    query.order_by() << "t_1.time_begin desc";
    } else {
// hardcore optimizations have to be done on this statement
	    query.select() << "tmp.id, t_1.time_begin, t_1.time_end, t_3.name, t_1.source_ip, t_2.status, t_1.session_id, t_1.user_name, t_1.is_monitoring, "
						" (select content from request_data where entry_time_begin=t_1.time_begin and entry_id=tmp.id and is_response=false limit 1) as request, "
						" (select content from request_data where entry_time_begin=t_1.time_begin and entry_id=tmp.id and is_response=true  limit 1) as response ";
	    query.from() << getTempTableName() << " tmp join request t_1 on tmp.id=t_1.id  join request_type t_2 on t_2.id=t_1.action_type join service t_3 on t_3.id=t_1.service";
	    query.order_by() << "t_1.time_begin desc";
    }

    Database::Connection conn = Database::Manager::acquire();
    try {

	// run all the queries
    	Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
        conn.exec(create_tmp_table);
        conn.exec(tmp_table_query);

    	Database::Result res = conn.exec(query);

    	for(Database::Result::Iterator it = res.begin(); it != res.end(); ++it) {
    		Database::Row::Iterator col = (*it).begin();

    		Database::ID 		id 		= *col;
    		Database::DateTime 	time_begin  	= *(++col);
    		Database::DateTime 	time_end  	= *(++col);
                std::string             serv_type  	= *(++col);
    		std::string 		source_ip  	= *(++col);
                std::string             action_type     = *(++col);
		Database::ID		session_id	= *(++col);
                std::string             user_name       = *(++col);
		bool			is_monitoring	= *(++col);
		// fields dependent on partialLoad
		std::string			request;
		std::string			response;

		std::auto_ptr<RequestProperties> props;

		if(!partialLoad) {
			request		= (std::string)*(++col);
			response	= (std::string)*(++col);
			props 		= getPropsForId(id);
		}

		data_.push_back(new RequestImpl(id,
				time_begin,
				time_end,
				serv_type,
				source_ip,
				action_type,
				session_id,
                                user_name,
				is_monitoring,
				request,
				response,
				props));

    	}

    	if(data_.empty()) {
    		return;
    	}
    }
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
      clear();
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
      clear();
    }
  }

  virtual void sort(MemberType _member, bool _asc) {

	// TODO
  }

  virtual std::auto_ptr<RequestProperties> getPropsForId(Database::ID id) {
	// TODO
	std::auto_ptr<RequestProperties> ret(new RequestProperties());
	Database::SelectQuery query;

	query.select() << "t_2.name, t_1.value, t_1.output, (t_1.parent_id is not null)";
	query.from()   << "request_property_value t_1 join request_property t_2 on t_1.name_id=t_2.id";
	query.where()  << "and t_1.entry_id = " << id;

    Database::Connection conn = Database::Manager::acquire();
 	Database::Result res = conn.exec(query);

    for(Database::Result::Iterator it = res.begin(); it != res.end(); ++it) {
    	Database::Row::Iterator col = (*it).begin();

		std::string 		name   = *col;
		std::string 		value  = *(++col);
		bool 			output = *(++col);
		bool 			is_child = *(++col);

		ret->push_back(RequestProperty(name, value, output, is_child));

	}
	return ret;
  }

  /// from CommonList; propably will be removed in future
  virtual const char* getTempTableName() const {
	return "tmp_request_filter_result";
	// TODO
  }

  virtual void makeQuery(bool, bool, std::stringstream&) const {
	// TODO maybe - stub in Mail class
  }

  virtual void reload() {
	 TRACE("[CALL] Register::Request::ListImpl::reload()");
	    clear();

	// here we go
	    Database::SelectQuery query;

	 if(partialLoad) {
		    query.select() << "t_1.time_begin, t_1.time_end, t_1.service, t_1.source_ip, t_1.action_type, t_1.session_id, t_1.is_monitoring";
		    query.from() << "request t_1";
		    query.order_by() << "t_1.time_begin desc";
	    } else {
	    	   query.select() << "t_1.time_begin, t_1.time_end, t_1.service, t_1.source_ip, t_1.user_name, t_1.action_type, t_1.session_id, t_1.is_monitoring, "
	    							" (select content from request_data where entry_time_begin=t_1.time_begin and entry_id=t_1.id and is_response=false limit 1) as request, "
	    							" (select content from request_data where entry_time_begin=t_1.time_begin and entry_id=t_1.id and is_response=true  limit 1) as response ";
	    		    query.from() << getTempTableName() << "request t_1";
	    		    query.order_by() << "t_1.time_begin desc";

	    }

        Database::Connection conn = Database::Manager::acquire();
	    try {
            Database::Result res = conn.exec(query);

			for(Database::Result::Iterator it = res.begin(); it != res.end(); ++it) {
				Database::Row::Iterator col = (*it).begin();

				Database::ID 		id 		= *col;
				Database::DateTime 	time_begin  	= *(++col);
				Database::DateTime 	time_end  	= *(++col);
				std::string             serv_type  = *(++col);
				std::string 		source_ip  	= *(++col);
                                std::string             user_name       = *(++col);
				std::string  		action_type = *(++col);
				Database::ID		session_id	= *(++col);
				bool			is_monitoring	= *(++col);
				std::string		request;
				std::string		response;

				std::auto_ptr<RequestProperties> props;

				if(!partialLoad) {
					request		= (std::string)*(++col);
					response	= (std::string)*(++col);
					props  		= getPropsForId(id);
				}

				data_.push_back(new RequestImpl(id,
							time_begin,
							time_end,
							serv_type,
							source_ip,
							action_type,
							session_id,
                                                        user_name,
							is_monitoring,
							request,
							response,
							props));
			}

			if(data_.empty()) {
				return;
			}
	    }
	    catch (Database::Exception& ex) {
	      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
	      clear();
	    }
	    catch (std::exception& ex) {
	      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
	      clear();
	    }
  }
};

// ------ new home for Impl_Log

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
 *  Class used to acquire database connection from the Database::Manager, set constraint_exclusion parameter and explicitly release the connection in dtor
 *  It assumes that the system error logger is initialized (use of logger_error() function
 */
class logd_auto_conn : public Connection {
public:
	explicit logd_auto_conn() : Connection(Database::Manager::acquire()) {

		// set constraint exclusion (needed for faster queries on partitioned tables)
		try {
			exec("set constraint_exclusion=on");
		} catch (Database::Exception &ex) {
			logger_error(boost::format("couldn't set constraint exclusion : %2%") % ex.what());
		}

	};

	~logd_auto_conn() {
		Database::Manager::release();
	}
};

List *ManagerImpl::createList() const {
	return new ListImpl((Manager *)this);
}

Result ManagerImpl::i_GetServiceActions(RequestServiceType service)
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

ManagerImpl::ManagerImpl()
{
}

// ManagerImpl ctor: connect to the database and fill property_names map
ManagerImpl::ManagerImpl(const std::string database, const std::string &monitoring_hosts_file)
      throw (DB_CONNECT_FAILED)
{
    	std::ifstream file;

  	log_ctx_init();

	try {
		// TODO move this to server.cc
		Database::Manager::init(new ConnectionFactory(database, 4, 20));
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

ManagerImpl::~ManagerImpl() {
  log_ctx_init();

  logger_notice("Logging destructor");
}

// check if a log record with the specified ID exists and if it can be modified (time_end isn't set yet)
bool ManagerImpl::record_check(ID id, Connection &conn)
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
ID ManagerImpl::find_property_name_id(const std::string &name, Connection &conn)
{
	ID name_id;
	std::map<std::string, ID>::iterator iter;

	std::string name_trunc = name.substr(0, MAX_NAME_LENGTH);

	iter = property_names.find(name_trunc);

	if(iter != property_names.end()) {
		name_id = iter->second;
	} else {
		// if the name isn't cached in the memory, try to find it in the database
		std::string s_name = conn.escape(name_trunc);

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
inline ID ManagerImpl::find_last_property_value_id(Connection &conn)
{
	Result res = conn.exec(LAST_PROPERTY_VALUE_ID);
	return res[0][0];
}

inline ID ManagerImpl::find_last_request_id(Connection &conn)
{
	Result res = conn.exec(LAST_ENTRY_ID);
	return res[0][0];
}

// insert properties for the given request record
void ManagerImpl::insert_props(DateTime entry_time, RequestServiceType service, bool monitoring, ID entry_id,  const Register::Logger::RequestProperties& props, Connection conn)
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
ID ManagerImpl::i_CreateRequest(const char *sourceIP, RequestServiceType service, const char *content_in, const Register::Logger::RequestProperties& props, RequestActionType action_type, ID session_id)
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



        if (session_id != 0){
                req.setSessionId(session_id);
                
                std::string user_name = getSessionUserName(conn, session_id);
                if(user_name != std::string()) {
                    req.setUserName(user_name);
                }
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

std::string ManagerImpl::getSessionUserName(Connection &conn, Database::ID session_id) 
{
	if (session_id != 0) {
                boost::format query = boost::format("select name from session where id = %1%") % session_id;
		Result res = conn.exec(query.str());

		if(res.size() == 0) {
			logger_error(boost::format("Session with ID %1% does not exist.") % session_id);
		}

                if(res[0][0].isnull()) return std::string(); 
                else return (std::string)res[0][0];
	}
}

// update existing log record with given ID
bool ManagerImpl::i_UpdateRequest(ID id, const Register::Logger::RequestProperties &props)
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

bool ManagerImpl::close_request_worker(Connection &conn, ID id, const char *content_out, const Register::Logger::RequestProperties &props)
{
	std::string s_content, time;
	RequestServiceType service;
	bool monitoring;

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	try {
		// first perform checks:
		if (!record_check(id, conn)) return false;

		// TODO the same case as before
		boost::format select = boost::format("select time_begin, service, is_monitoring from request where id = %1%") % id;
		Result res = conn.exec(select.str());
		DateTime entry_time = res[0][0].operator ptime();
		service = (RequestServiceType)(int) res[0][1];
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
bool ManagerImpl::i_CloseRequest(ID id, const char *content_out, const Register::Logger::RequestProperties &props)
{
	logd_auto_conn conn;

	log_ctx_init();

	return close_request_worker(conn, id, content_out, props);
}


bool ManagerImpl::i_CloseRequestLogin(ID id, const char *content_out, const Register::Logger::RequestProperties &props, ID session_id)
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

                std::string user_name = getSessionUserName(conn, session_id);

                if(user_name != std::string()) {
                    query = boost::format("update request set session_id = %1%, user_name='%2%' where id=%3%") % session_id % user_name % id;
                } else {
                    query = boost::format("update request set session_id = %1% where id=%2%") % session_id % id;
                }
		conn.exec(query.str());

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}
}

ID ManagerImpl::i_CreateSession(Languages lang, const char *name)
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

bool ManagerImpl::i_CloseSession(ID id)
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

Manager* Manager::create() {
	TRACE("[CALL] Register::Logger::Manager::create()");
	return new ManagerImpl();
}

Manager *Manager::create(const std::string conn_db, const std::string &monitoring_hosts_file)
throw (Manager::DB_CONNECT_FAILED) {
	TRACE("[CALL] Register::Logger::Manager::create(std::string, std::string)");
	return new ManagerImpl(conn_db, monitoring_hosts_file);
}


const std::string ManagerImpl::LAST_PROPERTY_VALUE_ID = "select currval('request_property_value_id_seq'::regclass)";
const std::string ManagerImpl::LAST_PROPERTY_NAME_ID = "select currval('request_property_id_seq'::regclass)";
const std::string ManagerImpl::LAST_ENTRY_ID = "select currval('request_id_seq'::regclass)";
const std::string ManagerImpl::LAST_SESSION_ID = "select currval('session_id_seq'::regclass)";
const int ManagerImpl::MAX_NAME_LENGTH = 30;



}
}

