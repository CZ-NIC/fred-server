#include <algorithm>
#include <boost/utility.hpp>
#include <boost/version.hpp>

// TODO probably to remove (only for debugging pritouts
#include <fstream>
#include <iostream>

#include <stdlib.h>

#include "config.h"
#include <pthread.h>
// logger
#include "old_utils/log.h"

// util (for escape)
#include "util.h"

#include "common_impl.h"
#include "request_impl.h"

// FRED logging
#include "log/logger.h"
#include "log/context.h"

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
  // types changed from ServiceType and RequestType to std::string because pagetable should now return it as strings
  std::string serv_type;
  std::string request_type_id;
  Database::ID session_id;
  std::string user_name;
  bool is_monitoring;
  std::string raw_request;
  std::string raw_response;
  boost::shared_ptr<RequestProperties> props;

public:
  RequestImpl(Database::ID &_id, Database::DateTime &_time_begin, Database::DateTime &_time_end, std::string &_serv_type, std::string &_source_ip,  std::string &_request_type_id, Database::ID &_session_id, std::string &_user_name, bool &_is_monitoring, std::string & _raw_request, std::string & _raw_response, std::auto_ptr<RequestProperties>  _props) :
	CommonObjectImpl(_id),
	time_begin(_time_begin),
	time_end(_time_end),
	source_ip(_source_ip),
	serv_type(_serv_type),
	request_type_id(_request_type_id),
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
	return request_type_id;
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
    TRACE("[CALL] Register::Logger::ListImpl::reload()");
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
    id_query.order_by() << "id DESC";
    id_query.offset(load_offset_);
    id_query.limit(load_limit_);
    _filter.serialize(id_query);

    Database::InsertQuery tmp_table_query = Database::InsertQuery(getTempTableName(), id_query);
    LOGGER(PACKAGE).debug(boost::format("temporary table '%1%' generated sql = %2%") % getTempTableName() % tmp_table_query.str());

	// make the actual query for data
    Database::SelectQuery query;

    if(partialLoad) {
	    query.select() << "tmp.id, t_1.time_begin, t_1.time_end, t_3.name, t_1.source_ip, t_2.name, t_1.session_id, t_1.user_name, t_1.is_monitoring";
	    query.from() << getTempTableName() << " tmp join request t_1 on tmp.id=t_1.id join request_type t_2 on t_2.id=t_1.request_type_id join service t_3 on t_3.id=t_1.service_id";
	    query.order_by() << "t_1.time_begin desc";
    } else {
// hardcore optimizations have to be done on this statement
	    query.select() << "tmp.id, t_1.time_begin, t_1.time_end, t_3.name, t_1.source_ip, t_2.name, t_1.session_id, t_1.user_name, t_1.is_monitoring, "
						" (select content from request_data where request_time_begin=t_1.time_begin and request_id=tmp.id and is_response=false limit 1) as request, "
						" (select content from request_data where request_time_begin=t_1.time_begin and request_id=tmp.id and is_response=true  limit 1) as response ";
	    query.from() << getTempTableName() << " tmp join request t_1 on tmp.id=t_1.id  join request_type t_2 on t_2.id=t_1.request_type_id join service t_3 on t_3.id=t_1.service_id";
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
                std::string             request_type_id     = *(++col);
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
				request_type_id,
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

      switch(_member) {
          case  MT_TIME_BEGIN:
              stable_sort(data_.begin(), data_.end(), CompareTimeBegin(_asc));
              break;
          case MT_TIME_END:
              stable_sort(data_.begin(), data_.end(), CompareTimeEnd(_asc));
              break;
          case MT_SOURCE_IP:
              stable_sort(data_.begin(), data_.end(), CompareSourceIp(_asc));
              break;
          case MT_SERVICE:
              stable_sort(data_.begin(), data_.end(), CompareServiceType(_asc));
              break;
          case MT_ACTION: 
              stable_sort(data_.begin(), data_.end(), CompareActionType(_asc));
              break;
          case MT_SESSION_ID:
              stable_sort(data_.begin(), data_.end(), CompareSessionId(_asc));
              break;
          case MT_USER_NAME:
              stable_sort(data_.begin(), data_.end(), CompareUserName(_asc));
              break;
          case MT_MONITORING:
              stable_sort(data_.begin(), data_.end(), CompareIsMonitoring(_asc));
              break;              
          default:
              break;
      }

  }

  virtual std::auto_ptr<RequestProperties> getPropsForId(Database::ID id) {
	// TODO
	std::auto_ptr<RequestProperties> ret(new RequestProperties());
	Database::SelectQuery query;

	query.select() << "t_2.name, t_1.value, t_1.output, (t_1.parent_id is not null)";
	query.from()   << "request_property_value t_1 join request_property_name t_2 on t_1.property_name_id=t_2.id";
	query.where()  << "and t_1.request_id = " << id;
    query.order_by() << "t_1.id";

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
	 TRACE("[CALL] Register::Logger::ListImpl::reload()");
	    clear();

	// here we go
	    Database::SelectQuery query;

	 if(partialLoad) {
		    query.select() << "t_1.time_begin, t_1.time_end, t_1.service_id, t_1.source_ip, t_1.request_type_id, t_1.session_id, t_1.is_monitoring";
		    query.from() << "request t_1";
		    query.order_by() << "t_1.time_begin desc";
	    } else {
	    	   query.select() << "t_1.time_begin, t_1.time_end, t_1.service_id, t_1.source_ip, t_1.user_name, t_1.request_type_id, t_1.session_id, t_1.is_monitoring, "
	    							" (select content from request_data where request_time_begin=t_1.time_begin and request_id=t_1.id and is_response=false limit 1) as request, "
	    							" (select content from request_data where request_time_begin=t_1.time_begin and request_id=t_1.id and is_response=true  limit 1) as response ";
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
				std::string  		request_type_id = *(++col);
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
							request_type_id,
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


#ifdef HAVE_LOGGER
class logd_ctx_init {
public:

    inline logd_ctx_init() {
	Logging::Context::clear();
        pthread_t tid = pthread_self();
        boost::format fmt = boost::format("logd-%1%") % tid;
        ctx.reset(new Logging::Context(fmt.str()));
    }

private:
    std::auto_ptr<Logging::Context> ctx;
};

#else

class logd_ctx_init { };

#endif

inline void logger_notice(const char *str);
inline void logger_error(const char *str);
inline void logger_notice(boost::format &fmt);
inline void logger_error(boost::format &fmt);

inline void logger_notice(const char *str)
{
        boost::format fmt(str);
        logger_notice(fmt);
}

inline void logger_error(const char *str)
{
        boost::format fmt(str);
	logger_error(fmt);
}

inline void logger_notice(boost::format &fmt)
{
#ifdef HAVE_LOGGER
	LOGGER("fred-server").notice(fmt);
#endif
}

inline void logger_error(boost::format &fmt)
{
#ifdef HAVE_LOGGER
	LOGGER("fred-server").error(fmt);
#endif
}

/**
         *  Database access class designed for use in fred-logd with partitioned tables
         *  It ensures that the connection will have constraint_exclusion enabled
         *  and all queries will be performed in one transaction which
         *  retains the features of Database::Transaction - it has to be commited
         *  explicitly by the client, otherwise it's rollbacked.
         *  The class acquires database connection from the Database::Manager and explicitly
         *  releases in destructor.
         *  DEPENDENCIES:
         *   - It assumes that the system error logger is initialized (use of logger_error() function
         *   - there's only one connection per thread, so the DB framework receives
         *     the connection encapsulated here
         */
class logd_auto_db : public Connection {
public:

    explicit logd_auto_db() : Connection(Database::Manager::acquire()), tx(0) {
        // set constraint exclusion (needed for faster queries on partitioned tables)
        try {
            /* In postgres version older than 8.3 constraint_exclusion=on posed a risk - it could even 
             * lead to incorrect results (cached execution plans)
             * But on new versions it can only mean a decrease of performance by few % if not used properly. 
             * It was discussed that this option would be on by default 
             */
            exec("set constraint_exclusion=on");
        } catch (Database::Exception &ex) {
            logger_error(boost::format("couldn't set constraint exclusion : %2%") % ex.what());
        }
        tx = new Database::Transaction(*this);
    }

    ~logd_auto_db() {
        delete tx;
        Database::Manager::release();
    }

    void commit() {
        tx->commit();
    }

private:
    Database::Transaction *tx;
};

List *ManagerImpl::createList() const {
	return new ListImpl((Manager *)this);
}

Result ManagerImpl::i_GetServiceActions(ServiceType service)
{
        logd_ctx_init ctx;

        Connection conn = Database::Manager::acquire();
	
	TRACE("[CALL] Register::Logger::ManagerImpl::i_GetServiceActions");

	boost::format query = boost::format("select id, name from request_type where service_id = %1%") % service;

        return conn.exec(query.str());
	
}

Database::Result ManagerImpl::i_GetServices()
{
    logd_ctx_init ctx;
    TRACE("[CALL] Register::Logger::ManagerImpl::i_GetServices()");

    Database::Connection conn = Database::Manager::acquire();
    std::string query = "SELECT id, name FROM service";
    return conn.exec(query);
}

// ManagerImpl ctor: connect to the database and fill property_names map
ManagerImpl::ManagerImpl(const std::string &monitoring_hosts_file)
      throw (DB_CONNECT_FAILED)
{
    	std::ifstream file;

  	logd_ctx_init ctx;

        Connection conn = Database::Manager::acquire();

	logger_notice("successfully  connect to DATABASE ");

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
		Result res = conn.exec("select id, name from request_property_name");

		if (res.size() > PROP_NAMES_SIZE_LIMIT) {
			logger_error(" Number of entries in request_property_name is over the limit.");

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
  logd_ctx_init ctx;

  logger_notice("Logging destructor");
}

// check if a log record with the specified ID exists and if it can be modified (time_end isn't set yet)

#ifdef DEBUG_LOGD
// optimization
bool ManagerImpl::record_check(ID id, Connection &conn)
{
    TRACE("[CALL] Register::Logger::ManagerImpl::record_check");
    
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
#endif //DEBUG_LOGD

// find ID for the given name of a property
// if the name is tool long, it's truncated to the maximal length allowed by the database
ID ManagerImpl::find_property_name_id(const std::string &name, Connection &conn, boost::mutex::scoped_lock& prop_add2db)
{
        TRACE("[CALL] Register::Logger::ManagerImpl::find_property_name_id");
	ID property_name_id;
	std::map<std::string, ID>::iterator iter;

	std::string name_trunc = name.substr(0, MAX_NAME_LENGTH);
       
#if ( BOOST_VERSION < 103500 ) 
        if(!prop_add2db.locked()) 
#else 
        if(!prop_add2db.owns_lock()) 
#endif
        {
            prop_add2db.lock();
        }

        bool db_insert = false;

	iter = property_names.find(name_trunc);

	if(iter != property_names.end()) {
		property_name_id = iter->second;

                prop_add2db.unlock();
                // unlock
        } else {
            // if the name isn't cached in the memory, try to find it in the database

            std::string s_name = conn.escape(name_trunc);

            boost::format query = boost::format("select id from request_property_name where name='%1%'") % s_name;
            Result res = conn.exec(query.str());
           
            if (res.size() > 0) {
                // okay, it was found in the database
                property_name_id = res[0][0];
            } else {
                // not found, we're under lock, so we can add it now
                // and let the lock release after commiting the transaction
                ModelRequestPropertyName pn;
                pn.setName(name_trunc);

                try {
                    pn.insert();
                    property_name_id = pn.getId();
                } catch (Database::Exception &ex) {
                    logger_error(ex.what());
                    prop_add2db.unlock();
                    return 0;
                }
                db_insert = true;
            }
        
            // now that we know the right database id of the name
            // we can add it to the map
            property_names[name_trunc] = property_name_id;
            
            // if the name was inserted into database, we have to keep it locked
            // until commit
            if (!db_insert) prop_add2db.unlock();
	}
        
	return property_name_id;
}

// insert properties for the given request record
void ManagerImpl::insert_props(DateTime entry_time, ServiceType service, bool monitoring, ID request_id,  const Register::Logger::RequestProperties& props, Connection conn, boost::mutex::scoped_lock &prop_lock)
{
        TRACE("[CALL] Register::Logger::ManagerImpl::insert_props");
	ID property_name_id, last_id = 0;

	if(props.size() == 0) {
		return;
	}

	// process the first record
	property_name_id = find_property_name_id(props[0].name, conn, prop_lock);

	if (props[0].child) {
		// the first property is set to child - this is an error
		logger_error(boost::format("entry ID %1%: first property marked as child. Ignoring this flag ") % request_id);
	}

	ModelRequestPropertyValue pv_first;
	pv_first.setRequestTimeBegin(entry_time);
	pv_first.setRequestServiceId(service);
	pv_first.setRequestMonitoring(monitoring);
	pv_first.setRequestId(request_id);
	pv_first.setPropertyNameId(property_name_id);
	pv_first.setValue (props[0].value);
	pv_first.setOutput(props[0].output);

	pv_first.insert();
        last_id = pv_first.getId();	

	// process the rest of the sequence
	for (unsigned i = 1; i < props.size(); i++) {
		property_name_id = find_property_name_id(props[i].name, conn, prop_lock);

		// create a new object for each iteration
		// because ParentId must alternate between NULL and some value
		ModelRequestPropertyValue pv;
		pv.setRequestTimeBegin(entry_time);
		pv.setRequestServiceId(service);
		pv.setRequestMonitoring(monitoring);
		pv.setRequestId(request_id);
		pv.setPropertyNameId(property_name_id);
		pv.setValue (props[i].value);
		pv.setOutput(props[i].output);

		if(props[i].child) {
			pv.setParentId(last_id);
			pv.insert();
		} else {
			pv.insert();
                        last_id = pv.getId();
		}
	}
        
        logger_notice(boost::format("Inserted %1% properties") % props.size());

}

void ManagerImpl::insert_props_pub(DateTime entry_time, ServiceType request_service_id, bool monitoring, Database::ID request_id, const Register::Logger::RequestProperties& props) {
#if ( BOOST_VERSION < 103500 ) 
        boost::mutex::scoped_lock prop_lock(properties_mutex, false);
#else 
        boost::mutex::scoped_lock prop_lock(properties_mutex, boost::defer_lock);
#endif
	insert_props(entry_time, request_service_id, monitoring, request_id, props, get_connection(), prop_lock);
}


// log a new event, return the database ID of the record
ID ManagerImpl::i_CreateRequest(const char *sourceIP, ServiceType service, const char *content_in, const Register::Logger::RequestProperties& props, RequestType request_type_id, ID session_id)
{
     	logd_ctx_init ctx;        
#ifdef HAVE_LOGGER
        boost::format sess_fmt = boost::format("session-%1%") % session_id;
        Logging::Context ctx_sess(sess_fmt.str());
#endif
        TRACE("[CALL] Register::Logger::ManagerImpl::i_CreateRequest");
        std::auto_ptr<Logging::Context> ctx_entry;


#if ( BOOST_VERSION < 103500 ) 
        boost::mutex::scoped_lock prop_lock(properties_mutex, false);
#else 
        boost::mutex::scoped_lock prop_lock(properties_mutex, boost::defer_lock);
#endif

        logd_auto_db db;

	ID request_id;

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
	req.setRequestTypeId(request_type_id);

        if (session_id != 0){
                req.setSessionId(session_id);
                
                std::string user_name = getSessionUserName(db, session_id);
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
                request_id = req.getId();

#ifdef HAVE_LOGGER
                boost::format entry_fmt = boost::format("entry-%1%") % request_id;
                ctx_entry.reset(new Logging::Context(entry_fmt.str()));
                
#endif
	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return 0;
	} catch (ConversionError &ex) {
		std::cout << "Exception text: " << ex.what() << std::endl;
                return 0;
	}

	try {
		ModelRequestData data;

		// insert into request_data
		if(content_in != NULL && content_in[0] != '\0') {
			data.setRequestTimeBegin(time);
			data.setRequestServiceId(service);
			data.setRequestMonitoring(monitoring);
			data.setRequestId(request_id);
			data.setContent(content_in);
			data.setIsResponse(false);

			data.insert();
		}

		// inserting properties
		insert_props(time, service, monitoring, request_id, props, db, prop_lock);

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
                return 0;
	}

        db.commit();
	return request_id;
}

// optimization
std::string ManagerImpl::getSessionUserName(Connection &conn, Database::ID session_id) 
{
    TRACE("[CALL] Register::Logger::ManagerImpl::getSessionUserName");
    
	if (session_id != 0) {
                boost::format query = boost::format("select name from session where id = %1%") % session_id;
		Result res = conn.exec(query.str());

		if(res.size() == 0) {
			logger_error(boost::format("Session with ID %1% does not exist.") % session_id);
		}

                if(res[0][0].isnull()) return std::string(); 
                else return (std::string)res[0][0];
	}

        return std::string();
}

// update existing log record with given ID
bool ManagerImpl::i_UpdateRequest(ID id, const Register::Logger::RequestProperties &props)
{	
  	logd_ctx_init ctx;        
#ifdef HAVE_LOGGER        
        boost::format entry_fmt = boost::format("entry-%1%") % id;
        Logging::Context ctx_entry(entry_fmt.str());
#endif
        
        TRACE("[CALL] Register::Logger::ManagerImpl::i_UpdateRequest");

        logd_auto_db db;

#if ( BOOST_VERSION < 103500 ) 
        boost::mutex::scoped_lock prop_lock(properties_mutex, false);
#else 
        boost::mutex::scoped_lock prop_lock(properties_mutex, boost::defer_lock);
#endif



	try {
		// perform check
#ifdef DEBUG_LOGD
		if (!record_check(id, db)) return false;
#endif

		// TODO think about some other way - i don't like this
                // optimization
		boost::format query = boost::format("select time_begin, service_id, is_monitoring from request where id = %1%") % id;
		Result res = db.exec(query.str());

		if(res.size() == 0) {
			logger_error(boost::format("Record with ID %1% in request table not found.") % id);
                        return false;
		}
		// end of TODO

		DateTime time = res[0][0].operator ptime();
		ServiceType service_id = (ServiceType)(int)res[0][1];
		bool monitoring        = (bool)res[0][2];
		insert_props(time, service_id, monitoring, id, props, db, prop_lock);
	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}

        db.commit();
	return true;

}

bool ManagerImpl::close_request_worker(Connection &conn, ID id, const char *content_out, const Register::Logger::RequestProperties &props)
{
    TRACE("[CALL] Register::Logger::ManagerImpl::close_request_worker");
	std::string time;
	ServiceType service_id;
	bool monitoring;
	long result_code = 0;//change dummy init value

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

#if ( BOOST_VERSION < 103500 ) 
        boost::mutex::scoped_lock prop_lock(properties_mutex, false);
#else 
        boost::mutex::scoped_lock prop_lock(properties_mutex, boost::defer_lock);
#endif



	try {
		// first perform checks:
#ifdef DEBUG_LOGD
		if (!record_check(id, conn)) return false;
#endif

                boost::format update = boost::format("update request set time_end=E'%1%' where id=%2%") % time % id;
		conn.exec(update.str());
                
                bool has_content = content_out != NULL && content_out[0] != '\0';
                
                if (has_content || props.size() > 0) {
                    // optimization
                    boost::format select = boost::format("select time_begin, service_id, is_monitoring from request where id = %1%") % id;
                    Result res = conn.exec(select.str());
                    if(res.size() == 0) {
                            logger_error(boost::format("Record  with ID %1% not found in request table.") % id );
                            return false;
                    }

                    DateTime entry_time = res[0][0].operator ptime();
                    service_id = (ServiceType)(int) res[0][1];
                    monitoring = (bool)res[0][2];

                    boost::format update_result_code_id = boost::format(
                            "update request set result_code_id=get_result_code_id( %2% , %3% )"
                            "    where id=%1%")
                        % id % service_id % result_code;
                    conn.exec(update_result_code_id.str());

                    

                    if(has_content) {
                            ModelRequestData data;

                            // insert into request_data
                            data.setRequestTimeBegin(entry_time);
                            data.setRequestServiceId(service_id);
                            data.setRequestMonitoring(monitoring);
                            data.setRequestId(id);
                            data.setContent(content_out);
                            data.setIsResponse(true);

                            data.insert();
                    }

                    // inserting properties
                    insert_props(res[0][0].operator ptime(), service_id, monitoring, id, props, conn, prop_lock);

                }

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}
	return true;
}

// close the record with given ID (end time is filled thus no further modification is possible after this call )
bool ManagerImpl::i_CloseRequest(ID id, const char *content_out, const Register::Logger::RequestProperties &props)
{	
	logd_ctx_init ctx;
#ifdef HAVE_LOGGER       
        boost::format entry_fmt = boost::format("entry-%1%") % id;
        Logging::Context ctx_entry(entry_fmt.str());
#endif        
        TRACE("[CALL] Register::Logger::ManagerImpl::i_CloseRequest");

        logd_auto_db db;

        if (close_request_worker(db, id, content_out, props)) {
            db.commit();
            return true;
        } else {
            return false;
        }
}


bool ManagerImpl::i_CloseRequestLogin(ID id, const char *content_out, const Register::Logger::RequestProperties &props, ID session_id)
{	
	logd_ctx_init ctx;
#ifdef HAVE_LOGGER
        boost::format sess_fmt = boost::format("session-%1%") % session_id;
        Logging::Context ctx_sess(sess_fmt.str());
        boost::format entry_fmt = boost::format("entry-%1%") % id;
        Logging::Context ctx_entry(entry_fmt.str());
#endif
        
        TRACE("[CALL] Register::Logger::ManagerImpl::i_CloseRequestLogin");

        bool ret;

	logd_auto_db db;

	ret = close_request_worker(db, id, content_out, props);
	if (!ret) return false;

	// fill in the session ID

        boost::format query;
#ifdef DEUBG_LOGD
	query = boost::format("select session_id from request where id=%1%") % id;
#endif

	try {
#ifdef DEBUG_LOGD
		Result res = db.exec(query.str());

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
#endif // DEBUG_LOGD

                std::string user_name = getSessionUserName(db, session_id);

                if(user_name != std::string()) {
                    query = boost::format("update request set session_id = %1%, user_name='%2%' where id=%3%") % session_id % user_name % id;
                } else {
                    query = boost::format("update request set session_id = %1% where id=%2%") % session_id % id;
                }
		db.exec(query.str());

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}

        db.commit();
        return true;
}

ID ManagerImpl::i_CreateSession(Languages lang, const char *name)
{	
	logd_ctx_init ctx;
        TRACE("[CALL] Register::Logger::ManagerImpl::i_CreateSession");

        std::auto_ptr<Logging::Context> ctx_sess;
       
	std::string time;
	ID id;

	logger_notice(boost::format("CreateSession: username-> [%1%] lang [%2%]") % name %  lang);

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	ModelSession sess;

	if (name != NULL && *name != '\0') {
		sess.setName(name);
	} else {
		logger_error("CreateSession: name is empty!");
		return 0;
	}

        if(lang == CS) {
                sess.setLang("cs");
        }

	try {
                sess.insert();

                id = sess.getId();

#ifdef HAVE_LOGGER
                boost::format sess_fmt = boost::format("session-%1%") % id;
                ctx_sess.reset(new Logging::Context(sess_fmt.str()));
#endif
                
        } catch (Database::Exception &ex) {
                logger_error(ex.what());
                return 0;
	}
	return id;
}

bool ManagerImpl::i_CloseSession(ID id)
{	        
	logd_ctx_init ctx;
#ifdef HAVE_LOGGER
        boost::format sess_fmt = boost::format("session-%1%") % id;
        Logging::Context ctx_sess(sess_fmt.str());       
#endif

        TRACE("[CALL] Register::Logger::ManagerImpl::i_CloseSession");

        logd_auto_db db;
	std::string  time;

	logger_notice(boost::format("CloseSession: session_id -> [%1%] ") % id );

#ifdef DEBUG_LOGD
	boost::format query = boost::format("select logout_date from session where id=%1%") % id;
#endif

	try {

#ifdef DEBUG_LOGD
		Result res = db.exec(query.str());

		if(res.size() == 0) {
			logger_error(boost::format("record in session with ID %1% doesn't exist") % id);
			return false;
		}

		if(!res[0][0].isnull()) {
			logger_error(boost::format("record in session with ID %1% already closed") % id);
			return false;
		}
#endif //DEBUG_LOGD

		boost::format update;
		time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

		update = boost::format("update session set logout_date = '%1%' where id=%2%") % time % id;

		db.exec(update.str());
	} catch (Database::Exception &ex) {
		logger_error(ex.what());
		return false;
	}

        db.commit();
	return true;
}

Manager* Manager::create() {
	TRACE("[CALL] Register::Logger::Manager::create()");
	return new ManagerImpl();
}

Manager *Manager::create(const std::string conn_db, const std::string &monitoring_hosts_file)
throw (Manager::DB_CONNECT_FAILED) {
	TRACE("[CALL] Register::Logger::Manager::create(std::string, std::string)");
	return new ManagerImpl(monitoring_hosts_file);
}

const int ManagerImpl::MAX_NAME_LENGTH = 30;



}
}

