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
 *  @file
 *  print description of object data to the string and ostream for debugging and error handling
 */


#ifndef PRINTABLE_H_
#define PRINTABLE_H_

#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <iterator>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>

#include "util/util.h"

namespace Util
{

    /**
     * Base class that adds ostream& @ref operator<<.
     */
    class Printable
    {
    public:
        /**
         * Empty destructor.
         */
        virtual ~Printable(){};
        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        virtual std::string to_string() const = 0;
        /**
        * Dumps state of the instance into stream
        * @param os contains output stream reference
        * @param i reference of instance to be dumped into the stream
        * @return output stream reference
        */
        friend std::ostream& operator<<(std::ostream& os, const Printable& i)
        {
            return os << i.to_string();
        }
    };

    /**
     * Print state of operation into the string.
     * @param operation_name is name of the operation
     * @param key_value_list names and values of operation data members
     * @return description of operation state using implemented format
     */
    std::string format_operation_state(const std::string& operation_name,
        const std::vector<std::pair<std::string, std::string> >& key_value_list);

    /**
     * Print data of data-structure into the string.
     * @param data_structure_name is name of the data-structure
     * @param key_value_list names and values of the structure data members
     * @return description of the data-structure state using implemented format
     */
    std::string format_data_structure(const std::string& data_structure_name,
        const std::vector<std::pair<std::string, std::string> >& key_value_list);

    /**
     * Types of conversions to std::string detectable by @ref ConversionToString template
     */
    struct TypeOfCoversionToString{enum Type{NONE,METHOD_TO_STRING, CONVERTIBLE_TO_STRING};};

    /**
     * Template detecting types of conversions to std::string
     * @param T examined type
     * @return result member set to detected type of conversion viz @ref TypeOfCoversionToString
     */
    template <typename T> class ConversionToString
    {
        typedef struct Tmp1 {char tmp[1];} none_;
        typedef struct Tmp2 {char tmp[2];} to_string_;
        typedef struct Tmp3 {char tmp[3];} convertible_to_string_;

        template<typename U, std::string (U::*)() const> struct RETURNING_STRING_CONST_MEMBER_PTR {};
        template<typename U> static to_string_ Test1(RETURNING_STRING_CONST_MEMBER_PTR<U, &U::to_string>*);
        template<typename U> static none_ Test1(...);

        static T MakeT();
        static convertible_to_string_ Test2(const std::string&);
        static none_ Test2(...);

    public:
        /**
         * template parameter type
         */
        typedef T value_type;

        /**
         * Detected type of conversion
         */
        static const TypeOfCoversionToString::Type result
            = (sizeof(Test1<T>(0)) == sizeof(to_string_))
                ? TypeOfCoversionToString::METHOD_TO_STRING
            : (sizeof(Test2(MakeT())) == sizeof(convertible_to_string_))
                ? TypeOfCoversionToString::CONVERTIBLE_TO_STRING
                    : TypeOfCoversionToString::NONE;
    };

    /**
     * Overload of conversion to std::string using to_string() method
     */
    template <class T> std::string printable_conversion_to_string(const T& t,
            EnumType<TypeOfCoversionToString::METHOD_TO_STRING>)
    {
        return t.to_string();
    };

    /**
     * Overload of conversion to std::string using implicit conversion
     */
    template <class T> std::string printable_conversion_to_string(const T& t,
            EnumType<TypeOfCoversionToString::CONVERTIBLE_TO_STRING>)
    {
        return t;
    };

    /**
     * Print vector of elements into the string. Values from input vector have to have conversion to string.
     * @param IN is input vector element type
     * @param in is input vector
     * @return description of input vector using implemented format
     */
    template<class IN> std::string format_vector(const std::vector<IN>& in)
    {
        std::string out;
        typename std::vector<IN>::const_iterator i = in.begin();

        if(i != in.end())
        {
            out += printable_conversion_to_string(*i,EnumType<ConversionToString<IN>::result >());
            ++i;
        }

        for(; i != in.end(); ++i)
        {
            out += " ";
            out += printable_conversion_to_string(*i,EnumType<ConversionToString<IN>::result >());
        }
        return out;
    }


}//namespace Util

#endif //PRINTABLE_H_
