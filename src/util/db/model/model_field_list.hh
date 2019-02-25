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
*  @file model_field_list.h
*  \class List 
*  \brief Container for fields for given model
*/

#ifndef MODEL_FIELD_LIST_HH_76E66F0B263B4A7C88B855F5D2B38F68
#define MODEL_FIELD_LIST_HH_76E66F0B263B4A7C88B855F5D2B38F68

#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "src/util/db/model/model_field.hh"
#include "src/util/db/model/model_exception.hh"


namespace Model {
namespace Field {

template<class model_class>
class List : public std::vector<Base_<model_class>* > {
public:
  typedef std::vector<Base_<model_class>* >  super;


  /**
   * Constuctors
   */
  List() {
  }

  
  /**
   * This constructor is for support boost::assign::list_of
   * initialization
   */
  template<class InputIterator>
  List(InputIterator _first, InputIterator _last) 
     : std::vector<Base_<model_class>*>(_first, _last) {
  }


  /**
   * Destructor
   */
  virtual ~List() {
  }


  /**
   * Search list for primary key field of type <T>
   *
   * @return  primary key field from list casted to Field::PrimayKey<model_type, T>
   */
  template<class T>
  Field::PrimaryKey<model_class, T>* getPrimaryKey() const {
    Field::PrimaryKey<model_class, T> *ret   = 0;
    Field::Base_<model_class>         *field = this->getPrimaryKey();

    ret = dynamic_cast<Field::PrimaryKey<model_class, T>* >(field);
    if (!ret) {
      throw Model::DefinitionError(field->getTableName() + "::" + field->getName(),
                                   "requested data type of primary key field doesn't fit"); 
    }
    return ret;
  }


  /**
   * Search list for primary key
   *
   * @return  primary key field from list without cast to specific type
   */
  Field::Base_<model_class>* getPrimaryKey() const {
    typename super::value_type field = 0;
    BOOST_FOREACH(field, *this) {
      if (field->getAttrs().isPrimaryKey()) {
        return field;
      }
    }

    throw Model::DefinitionError(field ? field->getTableName() : "<can't-get-table-name>",
                                 "can't find primary key field");
  }
};

}
}


#endif /*MODEL_FIELD_LIST_H_8216E00AA0974D79AB7F3A27B26DC6DF*/

