/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef CORBA_CONVERSIONS_INT_HH_3297047856AB440581BA632C3EC803BB
#define CORBA_CONVERSIONS_INT_HH_3297047856AB440581BA632C3EC803BB

#include <boost/integer_traits.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <exception>

namespace LibFred {
namespace Corba {

struct IntegerConversionFailed : std::bad_cast
{
    const char* what() const noexcept {
        return "integer conversion failed";
    }
};

/**
 * Basic integral types conversion with under/overflow detection
 */
template <class SOURCE_INTEGRAL_TYPE, class TARGET_INTEGRAL_TYPE>
void int_to_int(SOURCE_INTEGRAL_TYPE src, TARGET_INTEGRAL_TYPE& dst)
{
    typedef boost::integer_traits<SOURCE_INTEGRAL_TYPE> source_integral_type_traits;
    typedef boost::integer_traits<TARGET_INTEGRAL_TYPE> target_integral_type_traits;

    BOOST_MPL_ASSERT_MSG(source_integral_type_traits::is_integral, source_type_have_to_be_integral, (SOURCE_INTEGRAL_TYPE));
    BOOST_MPL_ASSERT_MSG(target_integral_type_traits::is_integral, target_type_have_to_be_integral, (TARGET_INTEGRAL_TYPE));
    try
    {
        dst = boost::numeric_cast<TARGET_INTEGRAL_TYPE>(src);
    }
    catch (const boost::bad_numeric_cast&)
    {
        throw IntegerConversionFailed();
    }
}

/**
 * Basic integral types conversion with under/overflow detection
 */
template <class SOURCE_INTEGRAL_TYPE, class TARGET_INTEGRAL_TYPE>
void unwrap_int(SOURCE_INTEGRAL_TYPE src, TARGET_INTEGRAL_TYPE& dst)
{
    int_to_int(src, dst);
}

/**
 * Basic integral types conversion with under/overflow detection
 */
template <class SOURCE_INTEGRAL_TYPE, class TARGET_INTEGRAL_TYPE>
void wrap_int(SOURCE_INTEGRAL_TYPE src, TARGET_INTEGRAL_TYPE& dst)
{
    int_to_int(src, dst);
}

template <class TARGET_INTEGRAL_TYPE, class SOURCE_INTEGRAL_TYPE>
TARGET_INTEGRAL_TYPE wrap_int(SOURCE_INTEGRAL_TYPE src)
{
    TARGET_INTEGRAL_TYPE dst;
    wrap_int(src, dst);
    return dst;
}


} // namespace LibFred::Corba
} // namespace LibFred

#endif
