/*
 * session.h
 *
 *  Created on: Jul 24, 2009
 *      Author: jvicenik
 */

#ifndef SESSION_HH_B8086417AB0F4947BBBC52CC8CAF056D
#define SESSION_HH_B8086417AB0F4947BBBC52CC8CAF056D

#include "src/deprecated/libfred/object.hh"
#include "libfred/db_settings.hh"

#include "src/deprecated/model/model_filters.hh"
#include "src/deprecated/model/log_filter.hh"
#include "src/deprecated/libfred/requests/model_session.hh"

namespace LibFred {
namespace Session {

enum MemberType {
	MT_NAME,
	MT_LOGIN_DATE,
	MT_LOGOUT_DATE,
	MT_LANG
};


class Session : virtual public LibFred::CommonObject {
public:
	virtual const std::string getName() const = 0;
	virtual const boost::posix_time::ptime getLoginDate() const = 0;
	virtual const boost::posix_time::ptime getLogoutDate() const = 0;
	virtual const std::string getLang() const = 0;
};

class List : virtual public LibFred::CommonList {

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
