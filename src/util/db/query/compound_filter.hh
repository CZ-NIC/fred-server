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
#ifndef COMPOUND_FILTER_HH_BC4D47B004444C85B74275F4E40932B4
#define COMPOUND_FILTER_HH_BC4D47B004444C85B74275F4E40932B4

#include <string>
#include <deque>
#include <vector>
#include <map>
#include <algorithm>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/map.hpp>

#include "src/util/db/query/filter.hh"
#include "src/util/db/query/simple_filter.hh"
#include "src/util/db/query/query_old.hh"
#include "util/types/data_types.hh"
#include "src/util/db/query/sql_helper_objects.hh"
#include "src/util/db/query/sql_operators.hh"
#include "src/util/db/query/filter_it.hh"

namespace Database {

class SelectQuery;

namespace Filters {

class Compound : public Filter {
public:
  Compound() :
    Filter(), join_on(0), polymorphic_joined_(false) {
  }
  Compound(const std::string& _conj) :
    Filter(_conj), join_on(0), polymorphic_joined_(false) {
  }
  virtual ~Compound();
  virtual bool isSimple() const {
    return false;
  }
  virtual void add(Filter *_f) {
    filter_list.push_back(_f);
  }

  virtual void _joinPolymorphicTables() {
    polymorphic_joined_ = true;
  }

  template<class Tp> Tp* find();
  template<class Tp> Tp* find(const std::string& _name);
  void clear();
  Table& joinTable(const std::string& _t);
  Table* findTable(const std::string& _t);
  void joinOn(Join *_j) {
    join_on = _j;
  }
  void addJoin(Join *_j) {
    joins.push_back(_j);
  }
  bool isActive() const;

  // DEPRECATED; should be use std::vector<> drawed out iterator
  Iterator *createIterator() {
    return new Iterator(filter_list);
  }

  typedef std::vector<Filter *>::iterator iterator;
  typedef std::vector<Filter *>::const_iterator const_iterator;
  iterator begin() {
    return filter_list.begin();
  }
  iterator end() {
    return filter_list.end();
  }
  const_iterator begin() const {
    return filter_list.begin();
  }
  const_iterator end() const {
    return filter_list.end();
  }

  virtual void serialize(Database::SelectQuery& _sq, const Settings *_settings = 0); 

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Filter);
    _ar & BOOST_SERIALIZATION_NVP(join_on);
    _ar & BOOST_SERIALIZATION_NVP(filter_list);
    _ar & BOOST_SERIALIZATION_NVP(joins);
    _ar & BOOST_SERIALIZATION_NVP(tables);
    _ar & BOOST_SERIALIZATION_NVP(alias_suffix);
  }

  std::string getContent() const;

protected:
  Join *join_on;
  std::vector<Filter*> filter_list;
  std::deque<Join*> joins;
  std::map<std::string, Table> tables;
  static unsigned alias_suffix;
  bool polymorphic_joined_;
};

}
}

#endif /*COMPOUND_FILTER_H_*/
