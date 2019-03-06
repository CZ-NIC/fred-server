/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#include <algorithm>
#include <boost/tokenizer.hpp>
#include <boost/utility.hpp>

#include "src/deprecated/model/union_filter.hh"
#include "util/log/logger.hh"

namespace Database {
namespace Filters {

void Union::serialize(Database::SelectQuery& _q) {
  TRACE("[CALL] Union::serialize()");
  if (_q.initialized()) {
    return;
  }

  if (filter_list.size() > query_list.size()) {
      boost::format msg = boost::format("assert(filter_list.size() > query_list.size());  %1% <! %2%")
        % filter_list.size() % query_list.size();
    LOGGER.error(msg);
    throw std::runtime_error(msg.str());
  }

  std::vector<Filter*>::iterator it_f = filter_list.begin();
  std::vector<SelectQuery*>::iterator it_q = query_list.begin();
  for (; it_f != filter_list.end(); ++it_f, ++it_q) {
    if ((*it_f)->isActive()) {
      (*it_f)->serialize(*(*it_q), settings_ptr_);
      (*it_q)->make();
      _q.buffer() << (it_f != filter_list.begin() ? " UNION " : "" );
      _q.buffer() << (*it_q)->buffer().str();
    }
  }
  _q.finalize();
  TRACE(boost::format("[IN] Union::serialize(): total %1% filters serialized")
      % filter_list.size());
  TRACE(boost::format("[IN] Union::serialize(): generated SQL = %1%")
      % _q.buffer().str());
}

void Union::clearQueries() {
  std::for_each(query_list.begin(), query_list.end(), boost::checked_deleter<SelectQuery>());
  query_list.clear();
}

void Union::clearFilters() {
	std::for_each(filter_list.begin(), filter_list.end(), boost::checked_deleter<Filter>());
	filter_list.clear();
}

void Union::clear() {
	TRACE("[CALL] Union::clear()");
	clearFilters();
	clearQueries();
}

}
}
