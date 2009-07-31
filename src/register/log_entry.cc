#include <algorithm>
#include <boost/utility.hpp>

#include "common_impl.h"
#include "log_entry.h"
#include "log/logger.h"


// TODO incomplete

namespace Register {
namespace Logger {

class LogEntryImpl : public Register::CommonObjectImpl,
                 virtual public LogEntry {
private:
  Database::DateTime time_begin;
  Database::DateTime time_end;
  Database::LogServiceType serv_type;
  std::string source_ip;

public:
  LogEntryImpl(Database::ID &_id, Database::DateTime &_time_begin, Database::DateTime &_time_end, Database::LogServiceType &_serv_type, std::string _source_ip) :
	CommonObjectImpl(_id),
	time_begin(_time_begin),
	time_end(_time_end),
	serv_type(_serv_type),
	source_ip(_source_ip) {
  }

  virtual const Database::DateTime&  getTimeBegin() const {
	return time_begin;
  }
  virtual const Database::DateTime&  getTimeEnd() const {
	return time_end;
  }
  virtual const Database::LogServiceType& getServiceType() const {
	return serv_type;
  }
  virtual const std::string& getSourceIp() const {
	return source_ip;
  }

};

COMPARE_CLASS_IMPL(LogEntryImpl, TimeBegin)
COMPARE_CLASS_IMPL(LogEntryImpl, TimeEnd)
COMPARE_CLASS_IMPL(LogEntryImpl, ServiceType)
COMPARE_CLASS_IMPL(LogEntryImpl, SourceIp)

class ListImpl : public Register::CommonListImpl,
                 virtual public List {
private:
  Manager *manager_;

public:
  ListImpl(Manager *_manager) : CommonListImpl(), manager_(_manager) {
  }

  virtual LogEntry* get(unsigned _idx) const {
    try {
      LogEntry *log_entry = dynamic_cast<LogEntry*>(data_.at(_idx));
      if (log_entry)
        return log_entry;
      else
        throw std::exception();
    }
    catch (...) {
      throw std::exception();
    }
  }

  // TODO properties should be displayed only in detailed view (in normal - probably only one property - or none)
  virtual void reload(Database::Filters::Union& _filter) {
    TRACE("[CALL] Register::LogEntry::ListImpl::reload()");
    clear();
    _filter.clearQueries();

	// iterate through all the filters
    bool at_least_one = false;
    Database::SelectQuery id_query;
    Database::Filters::Compound::iterator fit = _filter.begin();
    for (; fit != _filter.end(); ++fit) {
      Database::Filters::LogEntry *mf = dynamic_cast<Database::Filters::LogEntry* >(*fit);
      if (!mf)
        continue;

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect(new Database::Column("id", mf->joinLogEntryTable(), "DISTINCT"));
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
    query.from() << getTempTableName() << " tmp join log_entry t_1 on tmp.id=t_1.id";
    query.order_by() << "t_1.time_begin desc";

    try {

	// run all the queries
	Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
        Database::Connection conn = Database::Manager::acquire();
        conn.exec(create_tmp_table);
        conn.exec(tmp_table_query);

    	Database::Result res = conn.exec(query);

    	for(Database::Result::Iterator it = res.begin(); it != res.end(); ++it) {
    		Database::Row::Iterator col = (*it).begin();

    		Database::ID 		id 		= *col;
    		Database::DateTime 	time_begin  	= *(++col);
    		Database::DateTime 	time_end  	= *(++col);
    		Database::LogServiceType serv_type  	= (Database::LogServiceType)((int) *(++col));
    		std::string 		source_ip  	= *(++col);

    		data_.push_back(new LogEntryImpl(id,
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
	return "tmp_log_entry_filter_result";
	// TODO
  }

  virtual void makeQuery(bool, bool, std::stringstream&) const {
	// TODO maybe - stub in Mail class
  }

  virtual void reload() {
	 TRACE("[CALL] Register::LogEntry::ListImpl::reload()");
	    clear();

	// here we go
	    Database::SelectQuery query;
	    query.select() << "t_1.time_begin, t_1.time_end, t_1.service, t_1.source_ip";
	    query.from() << "log_entry t_1";
	    query.order_by() << "t_1.time_begin desc";

	    try {
        Database::Connection conn = Database::Manager::acquire();
		Database::Result res = conn.exec(query);

		for(Database::Result::Iterator it = res.begin(); it != res.end(); ++it) {
			Database::Row::Iterator col = (*it).begin();

			Database::ID 		id 		= *col;
			Database::DateTime 	time_begin  	= *(++col);
			Database::DateTime 	time_end  	= *(++col);
			Database::LogServiceType serv_type  	= (Database::LogServiceType) ((int)*(++col));
			std::string 		source_ip  	= *(++col);

			data_.push_back(new LogEntryImpl(id,
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
  TRACE("[CALL] Register::Mail::Manager::create()");
  return new ManagerImpl();
}

}
}

