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
 *  @file result.h
 *  Interface definition of result object.
 */


#ifndef RESULT_H_
#define RESULT_H_

#include <string>

#include "config.h"

#ifdef HAVE_LOGGER
#include "log/logger.h"
#endif

namespace Database {

/**
 * \class Result_
 * \brief Base template class for represent database result object
 *
 * template for result object used as a return value of connection
 * exec method parametrized by concrete _ResultType
 *
 * Result object (which should be finally used) is defined in \file database.h
 * file.
 *
 */
template<class _ResultType>
class Result_ {
public:
  typedef _ResultType result_type;
  typedef typename result_type::size_type size_type;
  typedef typename result_type::Iterator Iterator;
  typedef typename result_type::Row      Row;
  

  /**
   * Constructors and destructor
   */
  Result_(const result_type &_result) : result_(_result) {
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug(boost::format("result created -- rows=%1%") % size());
#endif
  }

  Result_() : result_()
  {  }

  virtual ~Result_() {
    result_.clear();
  }

  /**
   * @return size of result (number of rows)
   */
  size_type size() const {
    return result_.size();
  }

  /**
   * @number of rows affected by non-select query
   */
  size_type rows_affected() const
  {
    return result_.rows_affected();
  }


  /**
   * @return Iterator pointing at first row
   */
  Iterator begin() const {
    return result_.begin();
  }


  /**
   * @return Iterator pointing at last row
   */
  Iterator end() const {
    return result_.end();
  }

  /**
   * @param _n  requested row from result
   * @return    row
   */
  Row operator[](size_type _n) const {
    return *(begin() + _n);
  }

protected:
  result_type result_; /**< pointer on concrete result object */

};

}


#endif /*RESULT_H_*/

