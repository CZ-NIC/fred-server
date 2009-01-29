/*  
* Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

/**
*  @file model_field_list.h
*  \class List 
*  \brief Container for fields for given model
*/

#ifndef MODEL_FIELD_LIST_H_
#define MODEL_FIELD_LIST_H

#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "model_field.h"


namespace Model {
namespace Field {

template<class model_class>
class List : public std::vector<Base_<model_class>* > {
public:
  typedef std::vector<Base_<model_class>* >  super;


  /**
   * Constuctors and destructor
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


  virtual ~List() {
  }


  template<class _pk_type>
  Field::PrimaryKey<model_class, _pk_type>* getPrimaryKey() const {
    BOOST_FOREACH(typename super::value_type field, *this) {
      if (field->getAttrs().isPrimaryKey()) {
        return dynamic_cast<Field::PrimaryKey<model_class, _pk_type>* >(field);
      }
    }
    return 0;
  }
};

}
}


#endif /*MODEL_FIELD_LIST_H_*/

