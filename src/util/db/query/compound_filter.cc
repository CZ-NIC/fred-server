#include <algorithm>
#include <boost/tokenizer.hpp>
#include <boost/utility.hpp>
#include "src/util/db/query/compound_filter.hh"
#include "src/util/db/query/filter_it.hh"
#include "util/log/logger.hh"

namespace Database {
namespace Filters {

Compound::~Compound() {
  clear();
}

/**
 * not using getConjuction() problem with first simple filter in filter_list and
 * brackets (conjuction priority)
 */
void Compound::serialize(Database::SelectQuery& _sq, const Settings *_settings) {
  TRACE("[CALL] Compound::serialize()");
  LOGGER.debug(boost::format("serializing filter '%1%'") % getName());
  
//  if (!polymorphic_joined_) 
//    _joinPolymorphicTables();

  TRACE("[IN] Compound::serialize(): (+)join_on");
  _sq.from() << (join_on ? join_on->getType() : "");
  TRACE("[IN] Compound::serialize(): (+)joins");
  if (!joins.empty())
    _sq.from() << " ( ";
  for (std::deque<Join*>::const_iterator it = joins.begin(); it != joins.end(); ++it) {
    _sq.from() << (it == joins.begin() ? (*it)->str() : (*it)->str_without_left());
  }

  if (!joins.empty())
    _sq.from() << " ) ";
  else if (join_on)
    _sq.from() << join_on->getRight()->getTable().str();
  else if (!tables.empty()) {
    _sq.from() << tables.begin()->second.str();
  }

  _sq.from() << (join_on ? " ON (" + static_cast<Condition*>(join_on)->str() + " ) " : "");

  TRACE("[IN] Compound::serialize(): (+)filters");
  for (std::vector<Filter*>::const_iterator it = filter_list.begin(); it
      != filter_list.end(); ++it) {
    if (!(*it)->isActive()) {
      LOGGER.debug(boost::format("filter '%1%' is not set; skipping")
          % (*it)->getName());
      continue;
    }
    TRACE(boost::format("[ER] Compound::serialize(): (+)inner '%1%' filter")
        % (*it)->getName());
    (*it)->serialize(_sq, _settings);
    TRACE(boost::format("[RR] Compound::serialize(): (+)inner '%1%' filter")
        % (*it)->getName());
  }
}

Table& Compound::joinTable(const std::string& _t) {
  Table *tmp;
  if ((tmp = findTable(_t)) != 0) {
    return *tmp;
  } else {
    Table in(_t);
    std::stringstream alias;
    alias << "t_" << ++alias_suffix;
    in.setAlias(alias.str());
    tables.insert(std::make_pair(_t, in));
    return *(findTable(_t));
  }
}

Table* Compound::findTable(const std::string& _t) {
  std::map<std::string, Table>::iterator it;
  if ((it = tables.find(_t)) != tables.end()) {
    return &(it->second);
  } else {
    return 0;
  }
}

void Compound::clear() {
  std::for_each(filter_list.begin(),filter_list.end(),boost::checked_deleter<Filter>());
  filter_list.clear();
  std::for_each(joins.begin(),joins.end(),boost::checked_deleter<Join>());
  joins.clear();
  if (join_on) {
      delete join_on;
      join_on = 0;
  }
}

template<class Tp> Tp* Compound::find() {
  for (std::vector<Filter*>::iterator it = filter_list.begin(); it
      != filter_list.end(); ++it) {
    Tp* tmp = dynamic_cast<Tp*>(*it);
    if (tmp)
    return tmp;
  }
  return 0;
}
template<class Tp> Tp* Compound::find(const std::string& _name) {
  for (std::vector<Filter*>::iterator it = filter_list.begin(); it
      != filter_list.end(); ++it) {
    if ((*it)->getName() == _name) {
      return dynamic_cast<Tp*>(*it);
    }
  }
  return 0;
}

bool Compound::isActive() const {
  TRACE("[CALL] Compound::isActive()");
  if (active) return true;
  std::vector<Filter*>::const_iterator it = filter_list.begin();
  for(; it != filter_list.end(); ++it) {
    if ((*it)->isActive()) return true;
  }
  return false;
}

std::string Compound::getContent() const {
  std::string name = getName();
  std::string result;
  const_iterator it = begin();
  for(; it != end(); ++it) {
    Compound *test = dynamic_cast<Compound* >(*it);
    if (test) {
      result += name + "/" + test->getContent() + " ";
    }
    else {
      result += name + "/" + (*it)->getName() + " ";
    }
  }
  
  return result;
}

unsigned Compound::alias_suffix = 0;

}
}
