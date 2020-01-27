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
#ifndef QUERY_COMMON_HH_C7A6FB4DAB204F0B80BD616E68DFC420
#define QUERY_COMMON_HH_C7A6FB4DAB204F0B80BD616E68DFC420

#include <string>
#include "util/db/value.hh"

namespace Database {
namespace Internal {

class Table {
public:
  Table(const std::string &_name) : name_(_name),
                                    alias_("") { }


  Table(const std::string &_name,
        const std::string &_alias) : name_(_name),
                                     alias_(_alias) { }


  Table* clone() const {
    std::cout << "clone() table '" << name_ << "'" << std::endl;
    return new Table(name_, alias_);
  }


  const std::string& getName() const {
    return name_;
  }


  void setName(const std::string &_name) {
    name_ = _name;
  }


  const std::string& getAlias() const {
    return alias_;
  }


  void setAlias(const std::string &_alias) {
    alias_ = _alias;
  }


  std::string toSql(boost::function<std::string(std::string)> _esc_func [[gnu::unused]]) const {
    return (alias_.empty() ? name_ : name_ + " " + alias_);
  }


  bool operator==(const Table &_other) const {
    return (name_ == _other.name_ && alias_ == _other.alias_);
  }


protected:
  std::string name_;
  std::string alias_;
};



class Column {
public:

	Column()
		: name_()
		, table_()
	{}

  Column(const std::string &_name,
         const Table *_table) : name_(_name),
                                table_(_table) { }


  Column* clone(const Table *_table = 0) const {
    std::cout << "clone() column '" << name_ << "' -- table " << _table << std::endl;

    return new Column(name_, (_table ? _table : table_->clone()));
  }


  const std::string& getName() const {
    return name_;
  }


  void setName(const std::string &_name) {
    name_ = _name;
  }


  const Table& getTable() const {
    return *table_;
  }


  std::string toSql(boost::function<std::string(std::string)> _esc_func [[gnu::unused]]) const {
    std::string alias = table_->getAlias();
    return (alias.empty() ? name_ : alias + "." + name_);
  }


protected:
  std::string name_;
  const Table *table_;
};


class Condition_ {
public:
  typedef std::string operator_t;

  Condition_()
	  : left_()
	  , op_()
  {}

  Condition_(const Column &_l,
             const operator_t &_op) : left_(_l),
                                      op_(_op) { }


  const Column& getLeft() const {
    return left_;
  }


  const operator_t& getOperator() const {
    return op_;
  }


  std::string toSql(boost::function<std::string(std::string)> _esc_func) const {
    return left_.toSql(_esc_func) + " " + op_;
  }


protected:
  Column     left_;
  operator_t op_;
};



class Condition : public Condition_ {
public:
  Condition(const Column &_l,
            const operator_t &_op,
            const Column &_r) : Condition_(_l, _op),
                                right_(_r) { }


  Condition* clone(const Table *_left, const Table *_right) {
    return new Condition(*left_.clone(_left), op_, *right_.clone(_right));
  }


  const Column& getRight() const {
    return right_;
  }


  std::string toSql(boost::function<std::string(std::string)> _esc_func) const {
    return Condition_::toSql(_esc_func) + " " + right_.toSql(_esc_func);
  }


protected:
  Column     right_;
};



class ValueCondition : public Condition_ {
public:
  typedef Database::Value value_type;

  ValueCondition()
	  : value_()
  {}

  ValueCondition(const Column &_l,
                 const operator_t &_op,
                 const value_type &_value) : Condition_(_l, _op),
                                             value_(_value) { }



  const value_type& getRight() const {
    return value_;
  }


  std::string toSql(boost::function<std::string(std::string)> _esc_func) const {
    return Condition_::toSql(_esc_func) + " " + value_.toSql(_esc_func);
  }


protected:
  value_type value_;
};


}
}


#endif /*QUERY_COMMON_*/

