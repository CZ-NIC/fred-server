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
#ifndef SQL_HELPER_OBJECTS_HH_5369E5980855414FAB91578B89A09459
#define SQL_HELPER_OBJECTS_HH_5369E5980855414FAB91578B89A09459

#include <string>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>

#include "src/util/db/query/sql_operators.hh"

namespace Database {

class Table {
public:
  Table() {
  }
  Table(const std::string& _name) :
    name(_name) {
  }
  Table(const std::string& _name, const std::string& _alias) :
    name(_name), alias(_alias) {
  }
  std::string getName() const {
    return name;
  }
  void setName(const std::string& _name) {
    name = _name;
  }
  std::string getAlias() const {
    return alias;
  }
  void setAlias(const std::string& _alias) {
    alias = _alias;
  }
  std::string str() const {
    return std::string(name + " " + alias);
  }

  friend std::ostream& operator<<(std::ostream& _os, Table& _t);

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(name);
    _ar & BOOST_SERIALIZATION_NVP(alias);
  }

private:
  std::string name;
  std::string alias;
};

class Column {
public:
  Column() {
  }
  Column(const std::string& _name, Table& _table, const std::string& _funct = "") :
    name(_name), table(_table), funct(_funct) {
  }
  std::string getName() const {
    return name;
  }
  Table getTable() const {
    return table;
  }
  std::string getFunct() const {
    return funct;
  }
  bool hasFunct() const {
    return !(funct.empty());
  }
  void castTo(const std::string& _type, const std::string& _param = "") {
    cast_to = _type;
    cast_to_param = _param;
  }
  std::string str() const {
    bool f = hasFunct();
    bool c = !cast_to.empty();
    std::string tmp = (f ? (funct + "(") : "") + table.getAlias() + ".";
    if (c) {
      tmp = tmp + name + "::" + cast_to;
      if ((cast_to == "timestamptz") && !(cast_to_param.empty())) {
        tmp = tmp + " AT TIME ZONE '" + cast_to_param + "'";
        // other cast types with parameters
      }
    } else {
      tmp = tmp + name;
    }
    tmp = tmp + (f ? ")" : "");
    return tmp;
  }

  friend std::ostream& operator<<(std::ostream& _os, Column& _c);

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(name);
    _ar & BOOST_SERIALIZATION_NVP(table);
    _ar & BOOST_SERIALIZATION_NVP(funct);
    _ar & BOOST_SERIALIZATION_NVP(cast_to);
    _ar & BOOST_SERIALIZATION_NVP(cast_to_param);
  }

private:
  std::string name;
  Table table;

  std::string funct;
  std::string cast_to;
  std::string cast_to_param;
};

class Condition {
public:
  Condition(Column _l, const std::string& _op, Column _r) :
    left(_l), op(_op), right(_r) {
  }
  Condition() {
  }
  const Column* getLeft() const {
    return &left;
  }
  const Column* getRight() const {
    return &right;
  }
  std::string getOperator() const {
    return op;
  }
  std::string str() const {
    return std::string(left.str() + op + right.str());
  }

  friend std::ostream& operator<<(std::ostream& _os, Condition& _c);

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(left);
    _ar & BOOST_SERIALIZATION_NVP(op);
    _ar & BOOST_SERIALIZATION_NVP(right);
  }

protected:
  Column left;
  std::string op;
  Column right;
};

class Join : public Condition {
public:
  Join(Column _l, const std::string& _op, Column _r,
      const std::string& _join_type = SQL_JOIN) :
    Condition(_l, _op, _r), join_type(_join_type) {
  }
  Join() {
  }
  std::string str() const {
    return std::string(left.getTable().str() + join_type + right.getTable().str() + " ON (" + Condition::str() + ")");
  }
  std::string str_without_left() const {
    return std::string(join_type + right.getTable().str() + " ON (" + Condition::str() + ")");
  }
  std::string getType() const {
    return join_type;
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition);
    _ar & BOOST_SERIALIZATION_NVP(join_type);
  }

protected:
  std::string join_type;
};

}

#endif /*SQL_HELPER_OBJECTS_H_*/
