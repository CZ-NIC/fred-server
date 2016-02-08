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
 *  implementation for CORBA conversion
 */

#include "util/corba_conversion.h"

namespace CorbaConversion
{

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
    catch (const boost::numeric::negative_overflow &e) {
        throw IntegralConversionOutOfRange(e.what());
    }
    catch (const boost::numeric::positive_overflow &e) {
        throw IntegralConversionOutOfRange(e.what());
    }
}

IntegralConversionOutOfRange::IntegralConversionOutOfRange()
:   std::invalid_argument("Converted value is out of range")
{
}

IntegralConversionOutOfRange::IntegralConversionOutOfRange(const std::string &msg)
:   std::invalid_argument(msg)
{
}

//int_to_int< SRC_TYPE, DST_TYPE > template function instantiation
#define INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, DST_TYPE) \
    template void int_to_int(SRC_TYPE, DST_TYPE&)

//int_to_int< SRC_TYPE, any integer type > template function instantiation
#define INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(SRC_TYPE) \
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, bool);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, char);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, signed char);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, unsigned char);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, wchar_t);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, short);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, unsigned short);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, int);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, unsigned int);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, long);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, unsigned long);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, long long);\
    INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES(SRC_TYPE, unsigned long long)

//int_to_int< any integer type, any integer type > template function instantiation
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(bool);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(char);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(signed char);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(unsigned char);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(wchar_t);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(short);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(unsigned short);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(int);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(unsigned int);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(long);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(unsigned long);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(long long);
INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES(unsigned long long);

#undef INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_ANY_DST_TYPES
#undef INSTANTIATE_INT_TO_INT_CONVERTER_FOR_GIVEN_SRC_AND_DST_TYPES

}
