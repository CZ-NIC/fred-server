/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
 *  @file
 *  declaration for CORBA conversion
 */

#ifndef CORBA_CONVERSION_HH_387FCF00142C44A6BBF750AD5FF0E4F8
#define CORBA_CONVERSION_HH_387FCF00142C44A6BBF750AD5FF0E4F8

#include <stdexcept>
#include <string>
#include <omniORB4/CORBA.h>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/integer_traits.hpp>

/**
 * CORBA conversions
 */
namespace CorbaConversion
{

/**
 * Converted value is out of range
 */
struct IntegralConversionOutOfRange : std::invalid_argument
{
    IntegralConversionOutOfRange();
    explicit IntegralConversionOutOfRange(const std::string &msg);
    virtual ~IntegralConversionOutOfRange() {}
};

/**
 * Basic integral types conversion with under/overflow detection
 */
template < class SOURCE_INTEGRAL_TYPE, class TARGET_INTEGRAL_TYPE >
void int_to_int(SOURCE_INTEGRAL_TYPE src, TARGET_INTEGRAL_TYPE &dst)
{
    typedef boost::integer_traits< SOURCE_INTEGRAL_TYPE > source_integral_type_traits;
    typedef boost::integer_traits< TARGET_INTEGRAL_TYPE > target_integral_type_traits;

    BOOST_MPL_ASSERT_MSG(source_integral_type_traits::is_integral, source_type_have_to_be_integral, (SOURCE_INTEGRAL_TYPE));
    BOOST_MPL_ASSERT_MSG(target_integral_type_traits::is_integral, target_type_have_to_be_integral, (TARGET_INTEGRAL_TYPE));

    try {
        dst = boost::numeric_cast< TARGET_INTEGRAL_TYPE >(src);
    }
    catch (const boost::numeric::bad_numeric_cast &e) {
        throw IntegralConversionOutOfRange(e.what());
    }
}

/**
 * Basic integral types conversion with under/overflow detection
 */
template < class SOURCE_INTEGRAL_TYPE, class TARGET_INTEGRAL_TYPE >
void wrap_int(SOURCE_INTEGRAL_TYPE src, TARGET_INTEGRAL_TYPE &dst)
{
    int_to_int(src, dst);
}

/**
 * Basic integral types conversion with under/overflow detection
 */
template < class SOURCE_INTEGRAL_TYPE, class TARGET_INTEGRAL_TYPE >
void unwrap_int(SOURCE_INTEGRAL_TYPE src, TARGET_INTEGRAL_TYPE &dst)
{
    int_to_int(src, dst);
}

/**
 * Converts C++ string into CORBA specific string class.
 * @note In most cases is c_str() enough.
 */
CORBA::String_var wrap_string(const std::string &src);

}

#endif
