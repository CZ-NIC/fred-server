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
 *  @file base_exception.h
 *  base class for any user defined exception.
 */


#ifndef BASE_EXCEPTION_H_
#define BASE_EXCEPTION_H_

#include <exception>
#include <string>

/**
 * \class Exception
 * \brief Base class for user defined group of exceptions
 */
class Exception : public std::exception {
public:
  /**
   * Constructors and destructor
   */
	Exception(const Exception& _ex) throw() : std::exception(_ex), 
                                            what_(_ex.what_) {
	}


	Exception& operator=(const Exception& _ex) throw() {
		what_ = _ex.what_;
		return *this;
	}


	virtual ~Exception() throw() {
	}
	
  
  Exception(const std::string& _what) throw() :
		what_(_what) {
	}


  /**
   * @return  textual representation of exception
   */
	virtual const char* what() const throw() {
		return what_.c_str();
	}

protected:
	std::string what_; /**< exception details in text form */
};


#endif /*BASE_EXCEPTION_H_*/
