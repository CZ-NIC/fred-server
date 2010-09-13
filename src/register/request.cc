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
using namespace Database::Filters;
using namespace Database;

namespace Register {
namespace Logger {

#ifdef LOGD_VERIFY_INPUT
#error "LOGD_VERIFY_INPUT should be off because of performance improvement."
#endif

// TODO:
// * central configuration which types have to be treated like this
// * separate tree browsing & functor(visitor) parts

class CustomPartitioningTweak {
private:
  CustomPartitioningTweak() {};

public:

  static void process_filters(Database::Filters::Filter *f) {
        time_begin = NULL;
        service  = NULL;
        is_monitoring = NULL; 

        RequestImpl *ri = dynamic_cast<RequestImpl*> (f);
        if(ri != NULL) {
            find_values_recurs (ri);
    
            if(time_begin     != NULL || 
              service         != NULL ||
              is_monitoring   != NULL) {
              add_conds_recurs(ri);
            }
        }
  }

private:
  static void find_values_recurs (Filter *f) {
    // This check can be moved to the higher level
    if(!f->isActive()) return;

    Compound *c = dynamic_cast<Compound*>(f); 
    if (c == NULL) {
            if(f->getName() == "TimeBegin") {
                if(time_begin != NULL) {
                    LOGGER(PACKAGE).error("Duplicity TimeBegin found in filters.");
                    return;
                }
                time_begin = dynamic_cast< Interval<Database::DateTimeInterval>* >(f);
                if(time_begin == NULL) {
                    LOGGER(PACKAGE).error("TimeBegin: Inconsistency in filters.");
                    std::cout << "TimeBegin: Inconsistency in filters." << std::endl;
                }
      
            } else if(f->getName() == "ServiceType") {
                if(service != NULL) {
                    LOGGER(PACKAGE).error("Duplicity ServiceType found in filters.");
                    return;
                }
      
                service = dynamic_cast<Database::Filters::ServiceType*>(f);
                if(service == NULL) {
                    LOGGER(PACKAGE).error("ServiceType: Inconsistency in filters.");
                }
      
            } else if(f->getName() == "IsMonitoring") {
                if(is_monitoring != NULL) {
                    LOGGER(PACKAGE).error("Duplicity IsMonitoring found in filters.");
                    return;
                }
      
                is_monitoring = dynamic_cast< Database::Filters::Value<bool>* >(f);
                if(is_monitoring == NULL) {
                    LOGGER(PACKAGE).error("IsMonitoring: Inconsistency in filters.");
                }
            }
    }  else  {
        std::vector<Filter*>::iterator it = c->begin();  
      
        for(; it != c->end(); it++) {
            find_values_recurs(*it);
        }
    }
  }

  static void add_conds_recurs(Compound *c) {

      std::vector<Filter*>::iterator it = c->begin();
      for(; it != c->end(); it++) {
          Compound *child = dynamic_cast<Compound*> (*it);
          if(child != NULL) {
              add_conds_recurs(child);
          }
      }

      RequestDataImpl *request_data;
      RequestPropertyValueImpl *request_property_value;
      RequestObjectRefImpl *request_object_ref;

      Table *tbl = NULL;

      if((request_data = dynamic_cast<RequestDataImpl*>(c)) != NULL) {
          tbl = &request_data->joinRequestDataTable();
      } else if((request_property_value = dynamic_cast<RequestPropertyValueImpl*>(c)) != NULL) {
          tbl = &request_property_value->joinRequestPropertyValueTable();
      } else if((request_object_ref = dynamic_cast<RequestObjectRefImpl*>(c)) != NULL) {
          tbl = &request_object_ref->joinRequestObjectRefTable();
      }

      if(tbl != NULL) {

          if(time_begin != NULL) {
              Interval<Database::DateTimeInterval> *copy_time_begin = new Interval<Database::DateTimeInterval>(Column("request_time_begin", *tbl));
              copy_time_begin->setName("RequestTimeBegin");
              copy_time_begin->setValue(time_begin->getValue());
              // TODO  - this should be done via c-tors
              copy_time_begin->setNOT(time_begin->getNOT());
              // copy_time_begin->setConjuction(time_begin->getConjuction());
              // we never run into non-active record, no need to handle this field
              c->add(copy_time_begin);
          }

          if(service != NULL) {
              Database::Filters::ServiceType *copy_service = new Database::Filters::ServiceType(Column("request_service_id", *tbl));
              copy_service->setName("RequestService");
              copy_service->setValue(service->getValue());
              copy_service->setNOT(service->getNOT());
              // copy_service->setConjuction(service->getConjuction());
              // we never run into non-active record, no need to handle this field

              c->add(copy_service);
          } 

          if(is_monitoring != NULL) {
              Database::Filters::Value<bool> *copy_monitoring = new Database::Filters::Value<bool>(Column("request_monitoring", *tbl));
              copy_monitoring->setName("RequestIsMonitoring");
              copy_monitoring->setValue(is_monitoring->getValue());
              copy_monitoring->setNOT(is_monitoring->getNOT());
              // copy_monitoring->setConjuction(is_monitoring->getConjuction());
              // we never run into non-active record, no need to handle this field
              
              c->add(copy_monitoring);
          }
      }
  }

private:
  Interval<Database::DateTimeInterval> static * time_begin;
  Database::Filters::ServiceType static * service;
  Database::Filters::Value<bool> static * is_monitoring; 
  
};

Interval<Database::DateTimeInterval> * CustomPartitioningTweak::time_begin = NULL;
Database::Filters::ServiceType * CustomPartitioningTweak::service = NULL;
Database::Filters::Value<bool> * CustomPartitioningTweak::is_monitoring = NULL;




class RequestImpl : public Register::CommonObjectImpl,
                 virtual public Request {
private:
  DateTime time_begin;
  DateTime time_end;
  std::string source_ip;
  // types changed from ServiceType and RequestType to std::string because pagetable should now return it as strings
  std::string serv_type;
  std::string request_type_id;
  ID session_id;
  std::string user_name;
  ID user_id;
  bool is_monitoring;
  std::string raw_request;
  std::string raw_response;
  boost::shared_ptr<RequestProperties> props;
  boost::shared_ptr<ObjectReferences> refs;
  int rc_code;
  std::string rc_name;

public:
  RequestImpl(ID &_id, DateTime &_time_begin, DateTime &_time_end,
          std::string &_serv_type, std::string &_source_ip,
          std::string &_request_type_id, ID &_session_id,
          std::string &_user_name, ID &_user_id, 
          bool &_is_monitoring,
          std::string & _raw_request, std::string & _raw_response,
          std::auto_ptr<RequestProperties>  _props,
          std::auto_ptr<ObjectReferences>   _refs,
          const int _rc_code = 0, const std::string &_rc_name = std::string()) :
	CommonObjectImpl(_id),
	time_begin(_time_begin),
	time_end(_time_end),
	source_ip(_source_ip),
	serv_type(_serv_type),
	request_type_id(_request_type_id),
	session_id(_session_id),
        user_name(_user_name),
        user_id(_user_id),
	is_monitoring(_is_monitoring),
	raw_request(_raw_request),
	raw_response(_raw_response),
	props(_props),
        refs(_refs),
        rc_code(_rc_code),
        rc_name(_rc_name)
  {
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
  virtual const ID& getUserId() const {
        return user_id;
  }
  virtual const std::string& getActionType() const {
	return request_type_id;
  }
  virtual const ID& getSessionId() const {
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
  virtual boost::shared_ptr<RequestProperties> getProperties() {
	return props;
  }
  virtual boost::shared_ptr<ObjectReferences> getReferences() {
        return refs;
  }
  virtual const std::pair<int, std::string> getResultCode() const {
    return std::make_pair(rc_code, rc_name);
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

      CustomPartitioningTweak::process_filters(mf);

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
	    query.select() << "tmp.id, t_1.time_begin, t_1.time_end, t_3.name, "
                       << "t_1.source_ip, t_2.name, t_1.session_id, "
                       << "t_1.user_name, t_1.is_monitoring, "
                       << "t_4.result_code, t_4.name";
	    query.from() << getTempTableName() << " tmp join request t_1 on tmp.id = t_1.id "
                     << "join request_type t_2 on t_2.id = t_1.request_type_id "
                     << "join service t_3 on t_3.id = t_1.service_id "
                     << "left join result_code t_4 on t_4.id = t_1.result_code_id";
	    query.order_by() << "t_1.time_begin desc";
    } else {
// hardcore optimizations have to be done on this statement
	    query.select() << "tmp.id, t_1.time_begin, t_1.time_end, t_3.name, "
                       << "t_1.source_ip, t_2.name, t_1.session_id, "
                       << "t_1.user_name, t_1.user_id, t_1.is_monitoring, "
                       << "t_4.result_code, t_4.name, "
                       << "(select content from request_data where request_time_begin=t_1.time_begin and request_id=tmp.id and is_response=false limit 1) as request, "
                       << "(select content from request_data where request_time_begin=t_1.time_begin and request_id=tmp.id and is_response=true  limit 1) as response ";
	    query.from() << getTempTableName() << " tmp join request t_1 on tmp.id=t_1.id "
                     << "join request_type t_2 on t_2.id=t_1.request_type_id "
                     << "join service t_3 on t_3.id=t_1.service_id "
                     << "left join result_code t_4 on t_4.id = t_1.result_code_id";
	    query.order_by() << "t_1.time_begin desc";
    }

    Database::Connection conn = connectionSetup();
    
    try {

        // run all the queries
        Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
        conn.exec(create_tmp_table);
        conn.exec(tmp_table_query);

        Result res = conn.exec(query);
        for (Result::Iterator it = res.begin(); it != res.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            ID id                 = *col;
            DateTime time_begin   = *(++col);
            DateTime time_end     = *(++col);
            std::string serv_type = *(++col);
            std::string	source_ip = *(++col);
            std::string request_type_id = *(++col);
            ID session_id         = *(++col);
            std::string user_name = *(++col);

            ID user_id;
            if(!partialLoad) {
                user_id           = *(++col);
            }
            bool is_monitoring	  = *(++col);
            int rc_code           = *(++col);
            std::string rc_name   = *(++col);

            // fields dependent on partialLoad
            std::string	request;
            std::string	response;

            std::auto_ptr<RequestProperties> props;
            std::auto_ptr<ObjectReferences> refs;

            if(!partialLoad) {
                request  = (std::string)*(++col);
                response = (std::string)*(++col);
                props    = getPropsForId(id);
                refs     = getObjectRefsForId(id);
            }

            data_.push_back(new RequestImpl(
                    id,
                    time_begin,
                    time_end,
                    serv_type,
                    source_ip,
                    request_type_id,
                    session_id,
                    user_name,
                    user_id,
                    is_monitoring,
                    request,
                    response,
                    props,
                    refs,
                    rc_code,
                    rc_name));
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

        // TODO performance optimization using patitioning criteria
  virtual std::auto_ptr<RequestProperties> getPropsForId(ID id) {
	std::auto_ptr<RequestProperties> ret(new RequestProperties());
	Database::SelectQuery query;

	query.select() << "t_2.name, t_1.value, t_1.output, (t_1.parent_id is not null)";
	query.from()   << "request_property_value t_1 join request_property_name t_2 on t_1.property_name_id=t_2.id";
	query.where()  << "and t_1.request_id = " << id;
        query.order_by() << "t_1.id";

        Database::Connection conn = Database::Manager::acquire();
 	Result res = conn.exec(query);

        for(Result::Iterator it = res.begin(); it != res.end(); ++it) {
    	Database::Row::Iterator col = (*it).begin();

		std::string 		name   = *col;
		std::string 		value  = *(++col);
		bool 			output = *(++col);
		bool 			is_child = *(++col);

		ret->push_back(RequestProperty(name, value, output, is_child));

	}
	return ret;
  }

  virtual std::auto_ptr<ObjectReferences> getObjectRefsForId(ID id) {
        std::auto_ptr<ObjectReferences> ret(new ObjectReferences());

        Connection conn = Database::Manager::acquire();
        // TODO performance optimization using patitioning criteria
        Result res = conn.exec((boost::format
                ("SELECT name, object_id FROM request_object_ref oref "
                "JOIN request_object_type ot ON ot.id = oref.object_type_id "
                "WHERE request_id = %1%") % id).str());

        for(unsigned i=0;i<res.size();i++) {
                ObjectReference oref;
                oref.type = (std::string)res[i][0];
                oref.id = res[i][1];
                ret->push_back(oref);
        }

        return ret;
  }

  /// from CommonList; propably will be removed in future
  virtual const char* getTempTableName() const {
	return "tmp_request_filter_result";
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



            Database::Connection conn = connectionSetup();
	    try {
            Result res = conn.exec(query);

			for(Result::Iterator it = res.begin(); it != res.end(); ++it) {
				Database::Row::Iterator col = (*it).begin();

				ID 		id 		= *col;
				DateTime 	time_begin  	= *(++col);
				DateTime 	time_end  	= *(++col);
				std::string             serv_type  = *(++col);
				std::string 		source_ip  	= *(++col);
                                std::string             user_name       = *(++col);

                                ID              user_id;
                                if(!partialLoad) {
                                                        user_id = *(++col);
                                }
				std::string  		request_type_id = *(++col);
				ID		session_id	= *(++col);
				bool			is_monitoring	= *(++col);
				std::string		request;
				std::string		response;

				std::auto_ptr<RequestProperties> props;
                                std::auto_ptr<ObjectReferences> refs;

				if(!partialLoad) {
					request		= (std::string)*(++col);
					response	= (std::string)*(++col);
					props  		= getPropsForId(id);
                                        refs            = getObjectRefsForId(id);
				}

				data_.push_back(new RequestImpl(id,
							time_begin,
							time_end,
							serv_type,
							source_ip,
							request_type_id,
							session_id,
                                                        user_name,
                                                        user_id,
							is_monitoring,
							request,
							response,
							props,
                                                        refs));
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

private:
  Database::Connection connectionSetup() {
      Database::Connection conn = Database::Manager::acquire();

      conn.exec("set constraint_exclusion=ON");

      boost::format fmt_timeout =  boost::format("set statement_timeout=%1%") 
          % query_timeout;
      conn.exec(fmt_timeout.str());

      return conn;
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

Result ManagerImpl::i_getRequestTypesByService(ServiceType service)
{
        logd_ctx_init ctx;

        Connection conn = Database::Manager::acquire();
	
	TRACE("[CALL] Register::Logger::ManagerImpl::i_getRequestTypesByService");

	boost::format query = boost::format("select id, name from request_type where service_id = %1%") % service;

        return conn.exec(query.str());
	
}

Result ManagerImpl::i_getServices()
{
    logd_ctx_init ctx;
    TRACE("[CALL] Register::Logger::ManagerImpl::i_getServices()");

    Database::Connection conn = Database::Manager::acquire();
    std::string query = "SELECT id, name FROM service";
    return conn.exec(query);
}

Result ManagerImpl::i_getResultCodesByService(ServiceType service)
{
    logd_ctx_init ctx;
    Connection conn = Database::Manager::acquire();
    TRACE("[CALL] Register::Logger::ManagerImpl::i_getResultCodesByService");
    boost::format query = boost::format("select result_code, name from result_code where service_id = %1%") % service;
    return conn.exec(query.str());
}

Result ManagerImpl::i_getObjectTypes()
{
    // TODO - object types
    Database::Connection conn = Database::Manager::acquire();
    Result res = conn.exec("SELECT id, type FROM object_type");

    return res;
}


// ManagerImpl ctor: connect to the database and fill property_names map
ManagerImpl::ManagerImpl(const std::string &monitoring_hosts_file)
{
    	std::ifstream file;

  	logd_ctx_init ctx;

        Connection conn = Database::Manager::acquire();

	logger_notice("Logger startup - successfully connected to DATABASE ");

	if (!monitoring_hosts_file.empty()) {
		try {
                        std::string log_monitoring ("List of IP addresses of monitoring hosts: ");
			file.open(monitoring_hosts_file.c_str());
			// TODO
			// Error while reading config file test_log_monitoring.conf : basic_ios::clear
			// file.exceptions(std::ios::badbit | std::ios::failbit);

			while(file) {
				std::string input;
				file >> input;
				monitoring_ips.push_back(input);
                                log_monitoring = log_monitoring + input + " ";
			}

                        logger_notice(log_monitoring.c_str());

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

#ifdef LOGD_VERIFY_INPUT
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
#endif //LOGD_VERIFY_INPUT

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
void ManagerImpl::insert_props(DateTime entry_time, ServiceType service, bool monitoring, ID request_id,  const Register::Logger::RequestProperties& props, Connection &conn, bool output, boost::mutex::scoped_lock &prop_lock)
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
	pv_first.setOutput(output);

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
		pv.setOutput(output);

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

void ManagerImpl::insert_obj_ref(DateTime entry_time, ServiceType service, bool monitoring, ID request_id,  const Register::Logger::ObjectReferences &refs, Connection &conn) 
{

    for (unsigned i = 0; i<refs.size(); i++) {
        const std::string & obj_type = refs[i].type;
        int type_id;

        boost::format find_type_query = boost::format("select id from request_object_type where name='%1%'") % obj_type;

        Result res_type = conn.exec(find_type_query.str());
        if (res_type.size() == 0) {
                boost::format msg = boost::format("Object type with name '%1%' does not exist") % obj_type;
		logger_error(msg);
                throw InternalServerError(msg.str());
        } else {
                type_id = res_type[0][0];
        }

        boost::format insert_query  = boost::format("INSERT INTO request_object_ref (request_time_begin, request_service_id, request_monitoring, request_id, object_type_id, object_id) VALUES ('%1%', %2%, %3%, %4%, %5%, %6%)") % entry_time % service % (monitoring ? "true" : "false") % request_id % type_id % refs[i].id;

        conn.exec(insert_query.str());

    }

}

void ManagerImpl::insert_props_pub(DateTime entry_time, ServiceType request_service_id, bool monitoring, ID request_id, const Register::Logger::RequestProperties& props) {
#if ( BOOST_VERSION < 103500 ) 
        boost::mutex::scoped_lock prop_lock(properties_mutex, false);
#else 
        boost::mutex::scoped_lock prop_lock(properties_mutex, boost::defer_lock);
#endif
        Connection conn = get_connection();
        // insert_props for migration is not going to be so simple, TODO - true here is just TEMP
	insert_props(entry_time, request_service_id, monitoring, request_id, props, conn, true, prop_lock);
}


// log a new event, return the database ID of the record
ID ManagerImpl::i_createRequest(const char *sourceIP, ServiceType service, const char *content, const Register::Logger::RequestProperties& props, const Register::Logger::ObjectReferences &refs, RequestType request_type_id, ID session_id)
{
     	logd_ctx_init ctx;        
#ifdef HAVE_LOGGER
        boost::format sess_fmt = boost::format("session-%1%") % session_id;
        Logging::Context ctx_sess(sess_fmt.str());
#endif
        TRACE("[CALL] Register::Logger::ManagerImpl::i_createRequest");
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
                
                std::string user_name;
                Database::ID user_id;

                getSessionUser(db, session_id, &user_name, &user_id);

                if(user_name != std::string()) {
                    req.setUserName(user_name);
                }
                if(user_id != 0) {
                    req.setUserId(user_id);
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
		if(content != NULL && content[0] != '\0') {
			data.setRequestTimeBegin(time);
			data.setRequestServiceId(service);
			data.setRequestMonitoring(monitoring);
			data.setRequestId(request_id);
			data.setContent(content);
			data.setIsResponse(false);

			data.insert();
		}

                if(props.size() > 0) {
                        insert_props(time, service, monitoring, request_id, props, db, false, prop_lock);
                }
                if(refs.size() > 0) {
                        insert_obj_ref(time, service, monitoring, request_id, refs, db);
                }

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
                return 0;
	}

        db.commit();
	return request_id;
}

// optimization
void ManagerImpl::getSessionUser(Connection &conn, ID session_id, std::string *user_name, Database::ID *user_id) 
{
        TRACE("[CALL] Register::Logger::ManagerImpl::getSessionUser");
    
	if (session_id != 0) {
                boost::format query = boost::format("select user_name, user_id from session where id = %1%") % session_id;
		Result res = conn.exec(query.str());

		if(res.size() == 0) {
                        boost::format msg = boost::format("Session with ID %1% does not exist.") % session_id;
			logger_error(msg);
                        throw InternalServerError(msg.str());
		}

                if(res[0][0].isnull()) *user_name = std::string();
                else *user_name = (std::string)res[0][0];

                if(res[0][1].isnull()) *user_id = 0;
                else *user_id = res[0][1];

	} else {
                *user_name = std::string();
                *user_id = 0;
        }

}

// update existing log record with given ID
bool ManagerImpl::i_addRequestProperties(ID id, const Register::Logger::RequestProperties &props)
{	
  	logd_ctx_init ctx;        
#ifdef HAVE_LOGGER
        boost::format entry_fmt = boost::format("entry-%1%") % id;
        Logging::Context ctx_entry(entry_fmt.str());
#endif
        
        TRACE("[CALL] Register::Logger::ManagerImpl::i_addRequestProperties");

        logd_auto_db db;

#if ( BOOST_VERSION < 103500 ) 
        boost::mutex::scoped_lock prop_lock(properties_mutex, false);
#else 
        boost::mutex::scoped_lock prop_lock(properties_mutex, boost::defer_lock);
#endif



	try {
		// perform check
#ifdef LOGD_VERIFY_INPUT
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
		insert_props(time, service_id, monitoring, id, props, db, true, prop_lock);
	} catch (Database::Exception &ex) {
		logger_error(ex.what());
                throw InternalServerError(ex.what());
	}

        db.commit();
	return true;

}

// close the record with given ID (end time is filled thus no further modification is possible after this call )
//TODO session_id is optional - refactor code
bool ManagerImpl::i_closeRequest(ID id, const char *content, const Register::Logger::RequestProperties &props, const Register::Logger::ObjectReferences &refs, const long result_code, ID session_id)
{	
	logd_ctx_init ctx;
#ifdef HAVE_LOGGER
        boost::format sess_fmt = boost::format("session-%1%") % session_id;
        Logging::Context ctx_sess(sess_fmt.str());
        boost::format entry_fmt = boost::format("entry-%1%") % id;
        Logging::Context ctx_entry(entry_fmt.str());
#endif
        
        TRACE("[CALL] Register::Logger::ManagerImpl::i_closeRequest");
	logd_auto_db db;

        // THIS lock is used for inserting new properties. It MUST be unlocked (if it gets locked at all)
        // after the transaction is commited
#if ( BOOST_VERSION < 103500 ) 
        boost::mutex::scoped_lock prop_lock(properties_mutex, false);
#else 
        boost::mutex::scoped_lock prop_lock(properties_mutex, boost::defer_lock);
#endif

	try {
#ifdef LOGD_VERIFY_INPUT
                boost::format query_check;
                query_check = boost::format("select session_id, time_end from request where id=%1%") % id;
                // TODO you really should update the session_id

		Result res_check = db.exec(query_check.str());

		// if there is no record with specified ID
		if(res_check.size() == 0) {
			logger_error(boost::format("record in request with ID %1% doesn't exist") % id);
			return false;
		}

                if(!res_check[0][1].isnull()) {
                        logger_error(boost::format("record with ID %1% was already completed") % id);
                        return false;
                }

		if(session_id != 0 && !res_check[0][0].isnull()) {
		        ID filled = res_check[0][0];
                        if(filled != 0 && filled != session_id) {
                                logger_error(boost::format("record with ID %1% already has session_id filled") % id);
                                throw WrongUsageError(" session_id already set. ");
                        }
		}
#endif // LOGD_VERIFY_INPUT

                std::string query("UPDATE request SET time_end=now()");
                
                if(session_id != 0) {

                        query = query + ", session_id=" + boost::lexical_cast<std::string>(session_id);
                        std::string user_name;
                        Database::ID user_id;
                        getSessionUser(db, session_id, &user_name, &user_id);

                        if(!user_name.empty()) {
                                query = query +  (boost::format(", user_name='%1%'") % user_name ).str();
                        }
                        if(user_id != 0) {
                                query = query + (boost::format(", user_id=%1%") % user_id).str();
                        }
                }
                query = query + " WHERE id=" + boost::lexical_cast<std::string>(id);
                db.exec(query);

                // TODO how about a savepoint
                boost::format select = boost::format("select time_begin, service_id, is_monitoring from request where id = %1%") % id;
                Result res = db.exec(select.str());
                if(res.size() == 0) {
                        logger_error(boost::format("Record  with ID %1% not found in request table.") % id );
                        return false;
                }

                DateTime entry_time = res[0][0].operator ptime();
                ServiceType service_id = (ServiceType)(int) res[0][1];
                bool monitoring = (bool)res[0][2];

                boost::format update_result_code_id = boost::format(
                        "update request set result_code_id=get_result_code_id( %2% , %3% )"
                        "    where id=%1%")
                    % id % service_id % result_code;
                db.exec(update_result_code_id.str());

                // insert output content
                if(content != NULL && content[0] != '\0') {
                        ModelRequestData data;

                        // insert into request_data
                        data.setRequestTimeBegin(entry_time);
                        data.setRequestServiceId(service_id);
                        data.setRequestMonitoring(monitoring);
                        data.setRequestId(id);
                        data.setContent(content);
                        data.setIsResponse(true);

                        data.insert();
                }

                // insert properties 
                if(props.size() > 0) {
                        insert_props(entry_time, service_id, monitoring, id, props, db, true, prop_lock);
                }

                if(refs.size() > 0) {
                        insert_obj_ref(entry_time, service_id, monitoring, id, refs, db);
                }

	} catch (Database::Exception &ex) {
		logger_error(ex.what());
                throw InternalServerError(ex.what());
	}

        db.commit();
        return true;
}

ID ManagerImpl::i_createSession(ID user_id, const char *name)
{	
	logd_ctx_init ctx;
        TRACE("[CALL] Register::Logger::ManagerImpl::i_createSession");

        std::auto_ptr<Logging::Context> ctx_sess;
       
	std::string time;
	ID session_id;

	logger_notice(boost::format("createSession: username-> [%1%] user_id-> [%2%]") % name %  user_id);

	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

	ModelSession sess;

	if (name != NULL && *name != '\0') {
		sess.setUserName(name);
	} else {
		logger_error("createSession: name is empty!");
                throw WrongUsageError ("User name is empty");
	}

        if (user_id != 0) {
                sess.setUserId(user_id);
        }

	try {
                sess.insert();

                session_id = sess.getId();

#ifdef HAVE_LOGGER
                boost::format sess_fmt = boost::format("session-%1%") % session_id;
                ctx_sess.reset(new Logging::Context(sess_fmt.str()));
#endif
                
        } catch (Database::Exception &ex) {
                logger_error(ex.what());
                throw InternalServerError(ex.what());
	}
	return session_id;
}

bool ManagerImpl::i_closeSession(ID id)
{	        
	logd_ctx_init ctx;
#ifdef HAVE_LOGGER
        boost::format sess_fmt = boost::format("session-%1%") % id;
        Logging::Context ctx_sess(sess_fmt.str());       
#endif

        TRACE("[CALL] Register::Logger::ManagerImpl::i_closeSession");

        logd_auto_db db;
	std::string  time;

	logger_notice(boost::format("closeSession: session_id -> [%1%] ") % id );

	try {

#ifdef LOGD_VERIFY_INPUT
                boost::format query = boost::format("select logout_date from session where id=%1%") % id;
		Result res = db.exec(query.str());

		if(res.size() == 0) {
			logger_error(boost::format("record in session with ID %1% doesn't exist") % id);
			return false;
		}

		if(!res[0][0].isnull()) {
			logger_error(boost::format("record in session with ID %1% already closed") % id);
			return false;
		}
#endif //LOGD_VERIFY_INPUT

		boost::format update;
		time = boost::posix_time::to_iso_string(microsec_clock::universal_time());

		update = boost::format("update session set logout_date = '%1%' where id=%2%") % time % id;

		db.exec(update.str());
	} catch (Database::Exception &ex) {
		logger_error(ex.what());
                throw InternalServerError(ex.what());
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

