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
 *  \brief List for model field with helper method retrieving
 *         special kind of fields
 */

#ifndef MODEL_FIELD_LIST_H_
#define MODEL_FIELD_LIST_H_

#include <boost/foreach.hpp>
#include <vector>


namespace Model {
namespace Field {


template<class _class>
class List : public std::vector<Base_<_class>* > {
public:
  typedef std::vector<Base_<_class>* >  super;
  typedef typename super::value_type    value_type;

  List() {
  }


  virtual ~List() {
  }


  const value_type getPrimaryKey() const {
    BOOST_FOREACH(value_type _field, this) {
      if (_field->getAttrs().isPrimaryKey()) {
        return _field;
      }
    }
  }
};


}
}


#endif /*MODEL_FIELD_LIST_H_*/

