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
#ifndef MODEL_FIELD_RELATED_HH_C46334B7D5274481BF01CFDE8ADEAE90
#define MODEL_FIELD_RELATED_HH_C46334B7D5274481BF01CFDE8ADEAE90

#include "src/util/db/model/model_field.hh"
#include "util/log/logger.hh"


namespace Model {
namespace Field {
namespace Related {


template<class _class, class _type, class _class_ref>
class OneToOne {
public:
  OneToOne(ForeignKey<_class, _type, _class_ref> &_fk,
           const MemPtr<_class, ::Field::Field<_type> > &_related_to,
           const MemPtr<_class, ::Field::Lazy::Field<_class_ref*> > &_related) 
         : fk_(_fk),
           related_to_(_related_to),
           related_(_related) {
  }


  virtual ~OneToOne() {
  }


  void setRelatedTo(_class *_obj, const _type &_value) {
    related_to_(*_obj) = _value;
    related_(*_obj) = 0;
  }


  void setRelated(_class *_obj, _class_ref *_value) {
    related_(*_obj) = _value;
    related_to_(*_obj) = fk_.getReferencedField().getValue(_value);
  }


  _class_ref* getRelated(_class *_obj) {
    if (related_to_(*_obj).isSet() && related_(*_obj).get() == 0) {
      _class_ref *loading = new _class_ref();

      Field::PrimaryKey<_class_ref, _type> *pk = _class_ref::getFields().template getPrimaryKey<_type>();
      pk->setValue(loading, related_to_(*_obj).get());
      loading->reload();

      setRelated(_obj, loading);
    }
    return related_(*_obj);
  }


protected:
  ForeignKey<_class, _type, _class_ref>               &fk_;
  MemPtr<_class, ::Field::Field<_type> >               related_to_;
  MemPtr<_class, ::Field::Lazy::Field<_class_ref*> >   related_;
};



template<class _class, class _type, class _class_ref>
class OneToMany {
public:
  OneToMany(ForeignKey<_class_ref, _type, _class> &_fk,
            const MemPtr<_class, ::Field::Lazy::List<_class_ref> > &_related)
          : fk_(_fk),
            related_(_related) {
  }


  virtual ~OneToMany() {
  }


  void addRelated(_class *_obj, _class_ref *_value) {
    related_(*_obj).push_back(_value);
  }


  void getRelated(_class *_obj) {
    /* do lazy list load propably based on filter */
    std::cout << "SELECT rel_table.* FROM " << fk_.getTableName() << " rel_table WHERE rel_table." << fk_.getName()
              << " = " << _class::getFields().template getPrimaryKey<_type>()->getValue(_obj) << std::endl;
  }


protected:
  ForeignKey<_class_ref, _type, _class>              &fk_;
  MemPtr<_class, ::Field::Lazy::List<_class_ref> >    related_;
};



template<class _class, class _type, class _class_ref, class _type_ref>
class ManyToMany {
public:
  ManyToMany(PrimaryKey<_class, _type> &_pk,
             PrimaryKey<_class_ref, _type_ref> &_pk_ref,
             const std::string &_middle_table,
             const std::string &_left_name,
             const std::string &_right_name,
             const MemPtr<_class, ::Field::Lazy::List<_class_ref> > &_related)
           : pk_(_pk),
             pk_ref_(_pk_ref),
             mt_(_middle_table, _left_name, _right_name),
             related_(_related) {
  }


  void addRelated(_class *_obj, _class_ref *_value) {
    related_(*_obj).push_back(_value);
  }


  void getRelated(_class *_obj) {
    /* do lazy list load propably based on filter */
    std::cout << "SELECT rel_table.* FROM " << pk_ref_.getTableName() << " rel_table JOIN " 
              << mt_.name << " map ON (map." << mt_.right << " = rel_table." << pk_ref_.getName() << ")" 
              << " WHERE map." << mt_.left << " = " << pk_.getValue(_obj) << std::endl;
  }
             

  virtual ~ManyToMany() {
  }


protected:
  struct MiddleTable {
    MiddleTable(const std::string &_name, const std::string &_left, const std::string &_right) 
              : name(_name), left(_left), right(_right) { }

    std::string name;
    std::string left;
    std::string right;
  };


  PrimaryKey<_class, _type>                         &pk_;
  PrimaryKey<_class_ref, _type_ref>                 &pk_ref_;
  MiddleTable                                        mt_;
  MemPtr<_class, ::Field::Lazy::List<_class_ref> >   related_;
};


}
}
}


#endif /*MODEL_FIELD_RELATED_H_*/

