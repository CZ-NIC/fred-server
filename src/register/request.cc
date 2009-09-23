#include <algorithm>
#include <boost/utility.hpp>

#include "common_impl.h"
#include "request.h"
#include "log/logger.h"

using namespace boost::posix_time;
using namespace boost::gregorian;

namespace Register {
namespace Logger {

class RequestImpl : public Register::CommonObjectImpl,
                 virtual public Request {
private:
  Database::DateTime time_begin;
  Database::DateTime time_end;
  std::string source_ip;
  Database::Filters::RequestServiceType serv_type;
  Database::Filters::RequestActionType action_type;
  Database::ID session_id;
  bool is_monitoring;
  std::string raw_request;
  std::string raw_response;
  std::auto_ptr<RequestProperties> props;

public:
  RequestImpl(Database::ID &_id, Database::DateTime &_time_begin, Database::DateTime &_time_end, Database::Filters::RequestServiceType &_serv_type, std::string &_source_ip,  Database::Filters::RequestActionType &_action_type, Database::ID &_session_id, bool &_is_monitoring, std::string & _raw_request, std::string & _raw_response, std::auto_ptr<RequestProperties>  _props) :
	CommonObjectImpl(_id),
	time_begin(_time_begin),
	time_end(_time_end),
	source_ip(_source_ip),
	serv_type(_serv_type),
	action_type(_action_type),
	session_id(_session_id),
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
  virtual const Database::Filters::RequestServiceType& getServiceType() const {
	return serv_type;
  }
  virtual const std::string& getSourceIp() const {
	return source_ip;
  }
  virtual const Database::Filters::RequestActionType& getActionType() const {
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
  virtual       std::auto_ptr<RequestProperties> getProperties() {
	return props;
  }
};

COMPARE_CLASS_IMPL(RequestImpl, TimeBegin)
COMPARE_CLASS_IMPL(RequestImpl, TimeEnd)
COMPARE_CLASS_IMPL(RequestImpl, SourceIp)
COMPARE_CLASS_IMPL(RequestImpl, ServiceType)
COMPARE_CLASS_IMPL(RequestImpl, ActionType)
COMPARE_CLASS_IMPL(RequestImpl, SessionId)
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
	    query.select() << "tmp.id, t_1.time_begin, t_1.time_end, t_1.service, t_1.source_ip, t_1.action_type, t_1.session_id, t_1.is_monitoring";
	    query.from() << getTempTableName() << " tmp join request t_1 on tmp.id=t_1.id ";
	    query.order_by() << "t_1.time_begin desc";
    } else {
// hardcore optimizations have to be done on this statement
	    query.select() << "tmp.id, t_1.time_begin, t_1.time_end, t_1.service, t_1.source_ip, t_1.action_type, t_1.session_id, t_1.is_monitoring, "
						" (select content from request_data where entry_time_begin=t_1.time_begin and entry_id=tmp.id and is_response=false limit 1) as request, "
						" (select content from request_data where entry_time_begin=t_1.time_begin and entry_id=tmp.id and is_response=true  limit 1) as response ";
	    query.from() << getTempTableName() << " tmp join request t_1 on tmp.id=t_1.id";
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
    		Database::Filters::RequestServiceType serv_type  	= (Database::Filters::RequestServiceType) (long)*(++col);
    		std::string 		source_ip  	= *(++col);
    		Database::Filters::RequestActionType  action_type = (Database::Filters::RequestActionType)(long)*(++col);
		Database::ID		session_id	= *(++col);
		bool				is_monitoring	= *(++col);
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
	    	   query.select() << "t_1.time_begin, t_1.time_end, t_1.service, t_1.source_ip, t_1.action_type, t_1.session_id, t_1.is_monitoring, "
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
				Database::Filters::RequestServiceType serv_type  = (Database::Filters::RequestServiceType) (long)*(++col);
				std::string 		source_ip  	= *(++col);
				Database::Filters::RequestActionType  action_type = (Database::Filters::RequestActionType) (long) *(++col);
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

class ManagerImpl : virtual public Manager {
public:
  ManagerImpl(){
  }
  virtual ~ManagerImpl() {
  }

  virtual List* createList() const  {
	return new ListImpl((Manager *)this);
  }
};

Manager* Manager::create() {
  TRACE("[CALL] Register::Logger::Manager::create()");
  return new ManagerImpl();
}

}
}

