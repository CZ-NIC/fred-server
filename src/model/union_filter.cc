#include <algorithm>
#include <boost/tokenizer.hpp>
#include <boost/utility.hpp>

#include "union_filter.h"
#include "db/filter_it.h"
#include "log/logger.h"

namespace DBase {
namespace Filters {

void Union::serialize(DBase::SelectQuery& _q) {
  TRACE("[CALL] Union::serialize()");
  if (_q.initialized()) {
    return;
  }
  if (filter_list.size() > query_list.size()) {
    LOGGER("db").error(boost::format("assert(filter_list.size() > query_list.size());  %1% >! %2%")
        % filter_list.size() % query_list.size());
    throw; // TODO: throw something more meaningfull
  }

  bool at_least_one = false;
  std::vector<Filter*>::iterator it_f = filter_list.begin();
  std::vector<SelectQuery*>::iterator it_q = query_list.begin();
  for (; it_f != filter_list.end(); ++it_f, ++it_q) {
    if ((*it_f)->isActive()) {
      (*it_f)->serialize(*(*it_q));
      (*it_q)->make();
      _q.buffer() << (it_f != filter_list.begin() ? " UNION " : "" );
      _q.buffer() << (*it_q)->buffer().str();
      at_least_one = true;
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
