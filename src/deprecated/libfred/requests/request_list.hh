/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REQUEST_LIST_HH_A0994E3FC4F749F8A4B0EEEA0514587C
#define REQUEST_LIST_HH_A0994E3FC4F749F8A4B0EEEA0514587C

#include "src/deprecated/libfred/requests/request.hh"

namespace LibFred {
namespace Logger {



enum MemberType {
  MT_TIME_BEGIN,
  MT_TIME_END,
  MT_SOURCE_IP,
  MT_SERVICE,
  MT_ACTION,
  MT_SESSION_ID,
  MT_USER_NAME,
  MT_MONITORING,
  MT_RESULT_CODE,
  MT_RAW_REQUEST,
  MT_RAW_RESPONSE
};


class List : virtual public LibFred::CommonList {

public:
  virtual void setPartialLoad(bool partialLoad) = 0;
  virtual Request* get(unsigned _idx) const = 0;
  virtual void reload(Database::Filters::Union& _filter) = 0;
  virtual void sort(MemberType _member, bool _asc) = 0;

  /// from CommonList; propably will be removed in future
  virtual const char* getTempTableName() const = 0;
  virtual void makeQuery(bool, bool, std::stringstream&) const = 0;
  virtual void reload() = 0;
};

}
}


#endif /* REQUEST_LIST_H_ */
