/*
 * session.h
 *
 *  Created on: Jul 24, 2009
 *      Author: jvicenik
 */

#ifndef SESSION_H_
#define SESSION_H_

#include "object.h"
#include "db_settings.h"

#include "model/model_filters.h"
#include "model/log_filter.h"
#include "model_session.h"

namespace Fred {
namespace Session {

enum MemberType {
	MT_NAME,
	MT_LOGIN_DATE,
	MT_LOGOUT_DATE,
	MT_LANG
};


class Session : virtual public Fred::CommonObject {
public:
	virtual const std::string getName() const = 0;
	virtual const boost::posix_time::ptime getLoginDate() const = 0;
	virtual const boost::posix_time::ptime getLogoutDate() const = 0;
	virtual const std::string getLang() const = 0;
};

class List : virtual public Fred::CommonList {

public:
  virtual Session* get(unsigned _idx) const = 0;
  virtual void reload(Database::Filters::Union& _filter) = 0;
  virtual void sort(MemberType _member, bool _asc) = 0;

  /// from CommonList; propably will be removed in future
  virtual const char* getTempTableName() const = 0;
  virtual void makeQuery(bool, bool, std::stringstream&) const = 0;
  virtual void reload() = 0;
};

class Manager {
public:
  virtual ~Manager() {
  }

  virtual List* createList() const = 0;
  static Manager* create();
};


};
};

#endif /* SESSION_H_ */
