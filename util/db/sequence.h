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
#include "query.h"
#include "types/id.h"

namespace Database {

/**
 * \class Sequence
 * \brief Implementation of database sequence manipulation
 */
class Sequence {
public:
  /**
   * Constructor
   *
   * @param _conn  connection on which sequence will be managed
   * @param _name  database sequence name
   */
  Sequence(Connection &_conn, const std::string _name) : conn_(_conn),
                                                         name_(_name) {
  }

  
  /**
   * @return current sequence value
   */
  ID getCurrent() {
    Query q;
    q.buffer() << "SELECT currval('" << name_ << "')";
    return execute_(q);
  }
  

  /**
   * @return next sequence value
   */
  ID getNext() {
    Query q;
    q.buffer() << "SELECT nextval('" << name_ << "')";
    return execute_(q);
  }
  
private:
  ID execute_(Query& _query) {
    Result result = conn_.exec(_query);
    return (*(result.begin()))[0];
  }

  Connection &conn_; /**< connection */
  std::string name_; /**< sequence name */
};

}

#endif /*SEQUENCE_H_*/
