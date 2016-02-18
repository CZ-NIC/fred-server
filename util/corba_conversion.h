/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  @file
 *  declaration for CORBA conversion
 */

#ifndef CORBA_CONVERSION_H_4402374c71c24ba88a4dfc1886eeebb5
#define CORBA_CONVERSION_H_4402374c71c24ba88a4dfc1886eeebb5

#include <stdexcept>
#include <string>
#include <omniORB4/CORBA.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/integer_traits.hpp>

/**
 * CORBA conversions
 */
namespace CorbaConversion
{

/**
 * Converted value is out of range
 */
class IntegralConversionOutOfRange : public std::invalid_argument
{
public:
    IntegralConversionOutOfRange();
    explicit IntegralConversionOutOfRange(const std::string &msg);
    virtual ~IntegralConversionOutOfRange() throw() {}
};

/**
 * Basic integral types conversion with overflow detection
 */
template < class SOURCE_INTEGRAL_TYPE, class TARGET_INTEGRAL_TYPE >
void int_to_int(SOURCE_INTEGRAL_TYPE src, TARGET_INTEGRAL_TYPE &dst);

/**
 * Converts C++ string into CORBA specific string class.
 * @note In most cases is c_str() enough.
 */
CORBA::String_var wrap_string(const std::string &src);

}

#endif//CORBA_CONVERSION_H_4402374c71c24ba88a4dfc1886eeebb5
