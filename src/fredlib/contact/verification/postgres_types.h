/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @file postgres_types.h
 *  postgres types
 */

#ifndef CONTACT_VERIFICATION_POSTGRES_TYPES_51237983410_
#define CONTACT_VERIFICATION_POSTGRES_TYPES_51237983410_

#include "fredlib/opexception.h"

namespace Fred
{
    namespace Pg {
        DECLARE_EXCEPTION_DATA(pg_integer_overflow, std::string);
        DECLARE_EXCEPTION_DATA(pg_integer_underflow, std::string);

        struct Exception
            :   virtual Fred::OperationException,
                ExceptionData_pg_integer_overflow<Exception>,
                ExceptionData_pg_integer_underflow<Exception>
        {};

        template<class Type, Type Min, Type Max> class Integral {
            Type value_;
            public:
                static const Type min_value_ = Min;
                static const Type max_value_ = Max;

            public:
                Integral(Type _value)
                    : value_(_value)
                {
                    if(value_ < min_value_) {
                        BOOST_THROW_EXCEPTION(
                            Exception()
                                .set_pg_integer_underflow("min=" + min_value_ + " _value=" + boost::lexical_cast<std::string>(_value)));
                    }
                    if(value_ > max_value_) {
                        BOOST_THROW_EXCEPTION(
                            Exception()
                                .set_pg_integer_underflow("min=" + min_value_ + " _value=" + boost::lexical_cast<std::string>(_value)));
                    }

                }

                operator Type() const {
                    return value_;
                }
        };
        template<class Type, Type Min, Type Max> const Type Integral<Type, Min, Max>::min_value_;
        template<class Type, Type Min, Type Max> const Type Integral<Type, Min, Max>::max_value_;

        // specific types

        typedef long Integer;
        typedef Integral<long, 1, 2147483647> Serial;
        typedef Integral<long long, 1, 9223372036854775807> BigSerial;
    }
}
#endif // CONTACT_VERIFICATION_POSTGRES_TYPES_51237983410_
