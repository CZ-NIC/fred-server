/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
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
