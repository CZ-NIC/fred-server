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
 *  @file model_list.h
 *  list of model instances loaded by filter
 */

#ifndef MODEL_LIST_HH_EE673018C30F48CB92E82531D1D495EF
#define MODEL_LIST_HH_EE673018C30F48CB92E82531D1D495EF

#include <deque>


namespace Model {


template<class _type>
class List : public std::deque<_type> {
public:
  List() {
  }


  virtual ~List() {
  }


  void reload() {
    
  }


protected:
};


}


#endif /*MODEL_LIST_H_*/

