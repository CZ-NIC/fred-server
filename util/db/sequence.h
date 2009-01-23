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
 *  @file sequence.h
 *  Sequence definition.
 */


#ifndef SEQUENCE_H_
#define SEQUENCE_H_

#include <string>
#include "types/id.h"

namespace Database {

/**
 * \class Sequence
 * \brief Implementation of database sequence manipulation
 */
template<class connection_type, class manager_type>
class Sequence_ {
public:
  typedef typename manager_type::result_type   result_type;


  /**
   * Constructor
   *
   * @param _conn  connection on which sequence will be managed
   * @param _name  database sequence name
   */
  Sequence_(connection_type &_conn,
            const std::string _name) : conn_(_conn),
                                       name_(_name) {
  }

  
  /**
   * @return current sequence value
   */
  ID getCurrent() {
    return execute_("SELECT currval('" + name_ + "')");
  }
  

  /**
   * @return next sequence value
   */
  ID getNext() {
    return execute_("SELECT nextval('" + name_ + "')");
  }


private:
  ID execute_(const std::string& _query) {
    result_type result = conn_.exec(_query);
    return result[0][0];
  }


  connection_type &conn_; /**< connection */
  std::string      name_; /**< sequence name */
};

}

#endif /*SEQUENCE_H_*/
