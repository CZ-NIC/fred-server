#include <algorithm>
#include <boost/utility.hpp>

#include "common_impl.h"
#include "request.h"
#include "log/logger.h"


// TODO incomplete

namespace Register {
namespace Logger {

class RequestImpl : public Register::CommonObjectImpl,
                 virtual public Request {
private:
  Database::DateTime time_begin;
  Database::DateTime time_end;
  std::string source_ip
  Database::RequestServiceType serv_type;
  Database::RequestActionType action_type;
  Database::ID session_id;
  bool is_monitoring;

public:
  RequestImpl(Database::ID &_id, Database::DateTime &_time_begin, Database::DateTime &_time_end, std::string _source_ip, Database::RequestServiceType &_serv_type, Database::RequestActionType &_action_type, Database::ID &_session_id, bool &_is_monitoring) :
	CommonObjectImpl(_id),
	time_begin(_time_begin),
	time_end(_time_end),
	source_ip(_source_ip) 
	serv_type(_serv_type),
	action_type(_action_type),
	session_id(_session_id),
	is_monitoring(_is_monitoring) {
  }

  virtual const Database::DateTime&  getTimeBegin() const {
	return time_begin;
  }
  virtual const Database::DateTime&  getTimeEnd() const {
	return time_end;
  }
  virtual const std::string& getSourceIp() const {
	return source_ip;
  }
  virtual const Database::RequestServiceType& getServiceType() const {
	return serv_type;
  }
  virtual const Database::RequestActionType& getActionType() const {
	return action_type;
  }
  virtual const Database::ID& getSessionId() const {
	return session_id;
  }
  virtual const bool& getIsMonitoring() const {
	return is_monitoring;
  }

};

COMPARE_CLASS_IMPL(RequestImpl, TimeBegin)
COMPARE_CLASS_IMPL(RequestImpl, TimeEnd)
COMPARE_CLASS_IMPL(RequestImpl, SourceIp)
COMPARE_CLASS_IMPL(RequestImpl, ServiceType)
COMPARE_CLASS_IMPL(RequestImpl, ActionType)
COMPARE_CLASS_IMPL(RequestImpl, SessionId)
COMPARE_CLASS_IMPL(RequestImpl, IsMonitoring)


class ListImpl : public Register::CommonListImpl,
                 virtual public List {
private:
  Manager *manager_;

public:
  ListImpl(Database::Connection *_conn, Manager *_manager) : CommonListImpl(_conn), manager_(_manager) {
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

  // TODO properties should be displayed only in detailed view (in normal - probably only one property - or none)
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
    query.select() << "tmp.id, t_1.time_begin, t_1.time_end, t_1.service, t_1.source_ip";
    query.from() << getTempTableName() << " tmp join request t_1 on tmp.id=t_1.id";
    query.order_by() << "t_1.time_begin desc";

    try {

	// run all the queries
	Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
        conn_->exec(create_tmp_table);
        conn_->exec(tmp_table_query);

    	Database::Result res = conn_->exec(query);

    	for(Database::Result::Iterator it = res.begin(); it != res.end(); ++it) {
    		Database::Row::Iterator col = (*it).begin();

    		Database::ID 		id 		= *col;
    		Database::DateTime 	time_begin  	= *(++col);
    		Database::DateTime 	time_end  	= *(++col);
    		Database::RequestServiceType serv_type  	= (Database::RequestServiceType)((int) *(++col));
    		std::string 		source_ip  	= *(++col);

    		data_.push_back(new RequestImpl(id,
    					time_begin,
    					time_end,
    					serv_type,
    					source_ip));

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
	    query.select() << "t_1.time_begin, t_1.time_end, t_1.service, t_1.source_ip";
	    query.from() << "request t_1";
	    query.order_by() << "t_1.time_begin desc";

	    try {
		Database::Result res = conn_->exec(query);

		for(Database::Result::Iterator it = res.begin(); it != res.end(); ++it) {
			Database::Row::Iterator col = (*it).begin();

			Database::ID 		id 		= *col;
			Database::DateTime 	time_begin  	= *(++col);
			Database::DateTime 	time_end  	= *(++col);
			Database::RequestServiceType serv_type  	= (Database::RequestServiceType) ((int)*(++col));
			std::string 		source_ip  	= *(++col);

			data_.push_back(new RequestImpl(id,
						time_begin,
						time_end,
						serv_type,
						source_ip));

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
private:
  Database::Manager *db_manager_;
  Database::Connection *conn_;

public:
  ManagerImpl(Database::Manager *_db_manager) : db_manager_(_db_manager),
	conn_(db_manager_->getConnection()) {
  }
  virtual ~ManagerImpl() {
    boost::checked_delete<Database::Connection>(conn_);
  }

  virtual List* createList() const  {
	return new ListImpl(conn_, (Manager *)this);
  }
};

Manager* Manager::create(Database::Manager *_db_manager) {
  TRACE("[CALL] Register::Mail::Manager::create()");
  return new ManagerImpl(_db_manager);
}

}
}

