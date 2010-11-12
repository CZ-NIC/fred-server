/*
 * session.cc
 *
 *  Created on: Jul 24, 2009
 *      Author: jvicenik
 */

#include <boost/utility.hpp>

#include "common_impl.h"
#include "session.h"
#include "log/logger.h"

using namespace boost::posix_time;
using namespace boost::gregorian;

namespace Fred {
namespace Session {

class SessionImpl : public Fred::CommonObjectImpl,
			virtual public Session {

private:
  std::string name;
  Database::DateTime login_date;
  Database::DateTime logout_date;
  std::string lang;

public:
  SessionImpl(Database::ID &_id, std::string &_name, Database::DateTime &_login_date, Database::DateTime &_logout_date, std::string &_lang) :
	  CommonObjectImpl(_id),
	  name(_name),
	  login_date(_login_date),
	  logout_date(_logout_date),
	  lang(_lang) {
  }

  virtual const std::string getName() const {
	  return name;
  }
  virtual const ptime getLoginDate() const {
	  return login_date;
  }
  virtual const ptime getLogoutDate() const {
	  return logout_date;
  }
  virtual const std::string getLang() const {
	  return lang;
  }

};

COMPARE_CLASS_IMPL(SessionImpl, Name);
COMPARE_CLASS_IMPL(SessionImpl, LoginDate);
COMPARE_CLASS_IMPL(SessionImpl, LogoutDate);
COMPARE_CLASS_IMPL(SessionImpl, Lang);


class ListImpl : public Fred::CommonListImpl,
				virtual public List {
private:
	Manager *manager_;

public:
  ListImpl(Manager *_manager) : CommonListImpl(), manager_(_manager) {
  }

  virtual Session* get(unsigned _idx) const {
	try {
	  Session *s = dynamic_cast<Session*>(data_.at(_idx));
	  if (s)
		return s;
	  else
		throw std::exception();
	}
	catch (...) {
	  throw std::exception();
	}
  }

  // TODO properties should be displayed only in detailed view
  virtual void reload(Database::Filters::Union& _filter) {
	TRACE("[CALL] Fred::Session::ListImpl::reload(Database::Filters::Union&)");
	clear();
	_filter.clearQueries();

	// iterate through all the filters
	bool at_least_one = false;
	Database::SelectQuery id_query;
	Database::Filters::Compound::iterator fit = _filter.begin();
	for (; fit != _filter.end(); ++fit) {
	  Database::Filters::Session *mf = dynamic_cast<Database::Filters::Session* >(*fit);
	  if (!mf)
		continue;

	  Database::SelectQuery *tmp = new Database::SelectQuery();
	  tmp->addSelect(new Database::Column("id", mf->joinSessionTable(), "DISTINCT"));
	  _filter.addQuery(tmp);
	  at_least_one = true;
	}
	if (!at_least_one) {
	  LOGGER(PACKAGE).error("wrong filter passed for reload!");
	  return;
	}

	// make an id query according to the filters
        id_query.order_by() << "id DESC";
	id_query.limit(load_limit_);
	_filter.serialize(id_query);

	Database::InsertQuery tmp_table_query = Database::InsertQuery(getTempTableName(), id_query);
	LOGGER(PACKAGE).debug(boost::format("temporary table '%1%' generated sql = %2%") % getTempTableName() % tmp_table_query.str());

	// make the actual query for data
	Database::SelectQuery query;

	query.select() << "tmp.id, t_1.name, t_1.login_date, t_1.logout_date, t_1.lang";
	query.from() << getTempTableName() << " tmp join session t_1 on tmp.id=t_1.id ";
	query.order_by() << "t_1.login_date desc";

    Database::Connection conn = Database::Manager::acquire();
	try {
	// run all the queries
		Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
		conn.exec(create_tmp_table);
		conn.exec(tmp_table_query);

		Database::Result res = conn.exec(query);

		for(Database::Result::Iterator it = res.begin(); it != res.end(); ++it) {
			Database::Row::Iterator col = (*it).begin();

			Database::ID 		id 			= *col;
			std::string 		name 		= *(++col);
			Database::DateTime 	login_date  = *(++col);
			Database::DateTime 	logout_date = *(++col);
			std::string			lang 		= *(++col);

			data_.push_back(new SessionImpl(id,
					name,
					login_date,
					logout_date,
					lang));
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

  virtual const char *getTempTableName() const {
	  return "tmp_session_filter_result";
  }

  virtual void makeQuery(bool, bool, std::stringstream&) const {
 	// TODO 
  }

  virtual void reload() {
	  TRACE("[CALL] Fred::Session::ListImpl::reload()");
	  clear();

	  Database::SelectQuery query;

	  query.select() << "t_1.name, t_1.login_date, t_1.logout_date, t_1.lang";
	  query.from() << "session t_1";
	  query.order_by() << "t_1.login_date desc";

      Database::Connection conn = Database::Manager::acquire();
	  try {
		// run all the queries
			Database::Result res = conn.exec(query);

			for(Database::Result::Iterator it = res.begin(); it != res.end(); ++it) {
				Database::Row::Iterator col = (*it).begin();

				Database::ID 		id 			= *col;
				std::string 		name 		= *(++col);
				Database::DateTime 	login_date  = *(++col);
				Database::DateTime 	logout_date = *(++col);
				std::string			lang 		= *(++col);

				data_.push_back(new SessionImpl(id,
						name,
						login_date,
						logout_date,
						lang));
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

	public:
	  ManagerImpl() {
	  }
	  virtual ~ManagerImpl() {
	  }

	  virtual List* createList() const  {
		return new ListImpl((Manager *)this);
	  }
};

Manager *Manager::create() {
	TRACE("[CALL] Fred::Session::Manager::create()");
	return new ManagerImpl();
}


};

};
