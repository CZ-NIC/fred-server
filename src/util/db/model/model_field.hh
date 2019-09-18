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
/**
 *  @file field_model.h
 *  Implementation of model field types and
 *  attributes.
 */

#ifndef MODEL_FIELD_HH_3532D2E18F574B9AA316A0048C7F53D8
#define MODEL_FIELD_HH_3532D2E18F574B9AA316A0048C7F53D8

#include <string>
#include <deque>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>

#include "src/util/db/model/field.hh"
#include "src/util/db/model/model_field_attrs.hh"
#include "src/util/db/model/model_exception.hh"
#include "src/util/db/model/mem_ptr.hh"
#include "src/util/db/model/job_queue.hh"

#include "src/util/db/query/query.hh"
#include "util/db/value.hh"


using namespace boost::posix_time;
using namespace boost::gregorian;


namespace Model {
namespace Field {


/**
 * \class Base_
 * \brief Top class for all model fields
 */
template<class _class>
class Base_ {
public:
  typedef _class  class_name;


  Base_(const std::string &_table_name,
        const std::string &_name,
        const Attributes &_attrs = Attributes())
      : table_name_(_table_name),
        name_(_name),
        attrs_(_attrs) { }


  virtual ~Base_() { }


  const std::string& getTableName() const {
    return table_name_;
  }


  const std::string& getName() const {
    return name_;
  }


  const Attributes& getAttrs() const {
    return attrs_;
  }


  virtual void serialize(std::ostream &_os, const class_name *_object) = 0;


  virtual void serialize(::Model::JobQueue &_jobs,
                         Database::InsertQuery &_query,
                         class_name *_object) = 0;


  virtual void serialize(::Model::JobQueue &_jobs,
                         Database::UpdateQuery &_query,
                         class_name *_object) = 0;


  virtual void setValue(class_name *_object,
                        const Database::Value &_value,
                        bool _is_set = true) = 0;


  virtual void markSerialized(class_name *_object) = 0;


protected:
  const std::string      &table_name_;
  std::string            name_;
  Attributes             attrs_;
};


/**
 * \class Basic
 * \brief The most general type of field
 */
template<class _class, class _type, class _field_type = ::Field::Field<_type> >
class Basic : public Base_<_class> {
public:
  typedef Base_<_class>                                   super;

  typedef typename super::class_name                      class_name;
  typedef _type                                           value_type;
  typedef _field_type                                     field_type;
  typedef MemPtr<class_name, field_type>                  variable_pointer;


  Basic(const variable_pointer &_ptr,
        const std::string &_table_name,
        const std::string &_name,
        const Attributes &_attrs = Attributes())
      : Base_<class_name>(_table_name, _name, _attrs),
        value_(_ptr) { }


  virtual ~Basic() { }


  void serialize(std::ostream &_os, const class_name *_object) {
    _os << this->getName() << "=" << Database::Value(value_(*_object));
  }


  void serialize(::Model::JobQueue &_jobs [[gnu::unused]], Database::InsertQuery &_query, class_name *_object) {
    if (!value_(*_object).isChanged()) {
      if (this->attrs_.isNotNull() && !this->attrs_.isDefault()) {
        throw SerializationError("INSERT", this->getTableName(), this->getName(),
                                 "attrs { isNotNull, !isDefault } && value { !isChanged }");
      }

      if (this->attrs_.isDefault()) {
        return;
      }

      _query.add(this->getName(), Database::Value());
    }
    else {
      _query.add(this->getName(), Database::Value(value_(*_object)));
    }
  }


  void serialize(::Model::JobQueue &_jobs [[gnu::unused]], Database::UpdateQuery &_query, class_name *_object) {
    if (value_(*_object).isChanged()) {
      _query.add(this->getName(), Database::Value(value_(*_object)));
    }
  }


  void markSerialized(class_name *_object) {
      if (value_(*_object).isChanged()) {
          value_(*_object).changed(false);
      }
  }


  virtual void setValue(class_name *_object, const Database::Value &_value, bool _is_set) {
    value_(*_object) = _value;
    value_(*_object).changed(_is_set);
  }


  const value_type& getValue(class_name *_object) {
    return value_(*_object).get();
  }


  const ::Field::Field<value_type>& getField(class_name *_object) {
    return value_(*_object);
  }

       
protected:
  variable_pointer value_;
};



/**
 * \class PrimaryKey
 * \brief Specialization of Basic field to meet criteria for Primary key
 */
template<class _class, class _type>
class PrimaryKey : public Basic<_class, _type> {
public:
  typedef Basic<_class, _type>                            super;

  typedef typename super::class_name       class_name;
  typedef typename super::value_type       value_type;
  typedef typename super::variable_pointer variable_pointer;
  

  PrimaryKey(const variable_pointer &_ptr,
             const std::string &_table_name,
             const std::string &_name,
             const Attributes &_attrs = Attributes())
           : Basic<class_name, value_type>(_ptr, _table_name, _name, _attrs) {
    super::attrs_.setPrimaryKey();
  }


  virtual ~PrimaryKey() { }


  void serialize(::Model::JobQueue &_jobs [[gnu::unused]], Database::InsertQuery &_query, class_name *_object) {
    if (!this->value_(*_object).isChanged()) {
      if (!this->attrs_.isDefault()) {
        throw SerializationError("INSERT", this->getTableName(), this->getName(), "attrs { !isDefault } && value { !isChanged }");
      }
      else {
        return;
      }
    }

    _query.add(this->getName(), Database::Value(this->value_(*_object)));
  }


  void serialize(::Model::JobQueue &_jobs [[gnu::unused]], Database::UpdateQuery &_query, class_name *_object) {
    if (!this->value_(*_object).isChanged()) {
      throw SerializationError("UPDATE", this->getTableName(), this->getName(), "value { !isChanged }");
    }

    _query.where().add(this->getName(), "=", Database::Value(this->value_(*_object)), "AND");
  }


  void markSerialized(class_name *_object) {
    this->value_(*_object).changed(true);
  }


  virtual void setValue(class_name *_object, const value_type &_value, bool _is_set [[gnu::unused]]) {
    this->value_(*_object) = _value;
  }

};



/**
 * \class ForeignKey
 * \brief Specialization of Basic field to meet criteria for Foreign key
 */
template<class _class, class _type, class _class_ref>
class ForeignKey : public Basic<_class, _type> {
public:
  typedef Basic<_class, _type> super;

  typedef typename super::class_name       class_name;
  typedef typename super::value_type       value_type;
  typedef typename super::variable_pointer variable_pointer;
  typedef _class_ref                       class_name_referenced;
  typedef Basic<class_name_referenced, value_type> field_type_referenced;
  

  ForeignKey(const variable_pointer &_ptr,
             const std::string &_table_name,
             const std::string &_name,
             Basic<class_name_referenced, value_type> &_field_referenced,
             const Attributes &_attrs = Attributes())
           : Basic<class_name, value_type>(_ptr, _table_name, _name, _attrs),
             field_referenced_(_field_referenced) { 
    super::attrs_.setForeignKey();
  }


  virtual ~ForeignKey() { }


  field_type_referenced& getReferencedField() const {
    return field_referenced_;
  }


protected:
  Basic<class_name_referenced, value_type> &field_referenced_;
};



/**
 * OBSOLETE:
 * should be replaced by relations (model_field_related.h)
 *
struct MiddleTable {
  MiddleTable(const std::string &_name, 
              const std::string &_left,
              const std::string &_right)
            : name(_name),
              left(_left),
              right(_right) { }

  std::string name;
  std::string left;
  std::string right;
};


template<class _pk_class, class _pk1_vtype, class _pk2_class, class _pk2_vtype>
class ManyToMany : public Basic<_pk_class, Model::List<_pk2_class> > {
public:
  typedef Basic<_pk_class, Model::List<_pk2_class> > super;

  typedef typename super::class_name       class_name;
  typedef typename super::value_type       value_type;
  typedef typename super::variable_pointer variable_pointer;

  typedef PrimaryKey<class_name, _pk1_vtype>   pk1_type;
  typedef PrimaryKey<_pk2_class, _pk2_vtype>   pk2_type;
  

  ManyToMany(const variable_pointer &_ptr,
             const std::string &_table_name,
             const std::string &_name,
             pk1_type &_pk_l,
             pk2_type &_pk_r,
             const MiddleTable &_mt,
             const Attributes &_attrs = Attributes())
           : Basic<class_name, value_type>(_ptr, _table_name, _name, _attrs),
             pk_l_(_pk_l),
             pk_r_(_pk_r),
             mt_(_mt) { }


  virtual ~ManyToMany() { }


  void serialize(::Model::JobQueue &_jobs, Database::InsertQuery &_query, class_name *_object) {
    BOOST_FOREACH(typename value_type::value_type _v, this->value_(*_object).get()) {
      std::stringstream conv1, conv2;
      conv1 << pk_l_.getValue(_object);
      conv2 << pk_r_.getValue(&_v);

      Database::InsertQuery *tmp = new Database::InsertQuery(mt_.name);
      tmp->add(mt_.left,  Database::Value(pk_l_.getValue(_object)));
      tmp->add(mt_.right, Database::Value(pk_r_.getValue(&_v)));
      
      _jobs.push(tmp);
    }
  }


  void serialize(::Model::JobQueue &_jobs, Database::UpdateQuery &_query, class_name *_object) {
  }



protected:
  pk1_type    &pk_l_;
  pk2_type    &pk_r_;
  MiddleTable mt_;
};

*/


}
}


#endif /*FIELD_MODEL_H_*/

