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
*  header of Fred::Object::Type class
*/

#ifndef OBJECT_TYPE_H_22A124B8D173FCF75E30657FA4D8D922//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define OBJECT_TYPE_H_22A124B8D173FCF75E30657FA4D8D922

#include <map>
#include <stdexcept>

/// Fred matters
namespace Fred {
/// Fred objects matters
namespace Object {

typedef unsigned long long Id;

//boost/type_traits.hpp defines DOMAIN
#ifdef DOMAIN
#undef DOMAIN
#endif

/**
 * Bidirectional conversions between string and enum representation of object types.
 */
class Type
{
public:
    /**
     * Names of particular object types.
     */
    enum Value
    {
        CONTACT,///< object is contact
        NSSET,  ///< object is nsset
        DOMAIN, ///< object is domain
        KEYSET, ///< object is keyset
    };
    /**
     * From enum value creates object having methods for conversion to its string representation.
     * @param _value enum value
     */
    explicit Type(Value _value):value_(_value) { }
    /**
     * String value converts to its enum equivalent.
     * @param _str database representation of object type
     * @return its enum equivalent
     * @throw std::runtime_error if conversion is impossible
     */
    static Value from(const std::string &_str)
    {
        const StrToValue& str2val = str_to_value();
        StrToValue::const_iterator item_ptr = str2val.find(_str);
        if (item_ptr != str2val.end()) {
            return item_ptr->second;
        }
        throw std::runtime_error("Unknown object state '" + _str + "'");
    }
    /**
     * Enum value converts to its string representation.
     * @param _str where store the result
     * @return its string representation
     * @throw std::runtime_error if conversion is impossible
     */
    std::string& into(std::string &_str)const
    {
        const ValueToStr& val2str = value_to_str();
        ValueToStr::const_iterator item_ptr = val2str.find(value_);
        if (item_ptr != val2str.end()) {
            return _str = item_ptr->second;
        }
        throw std::runtime_error("Invalid object state value");
    }
    /**
     * Enum value converts to value of other type.
     * @tparam T destination type
     * @return converted value
     * @throw std::runtime_error if conversion is impossible
     */
    template < typename T >
    T into()const
    {
        T result;
        this->into(result);
        return result;
    }
private:
    const Value value_;
    typedef std::map< std::string, Value > StrToValue;
    typedef std::map< Value, std::string > ValueToStr;
    static const StrToValue& str_to_value()
    {
        static StrToValue result;
        if (result.empty()) {
            result["contact"] = CONTACT;
            result["nsset"]   = NSSET;
            result["domain"]  = DOMAIN;
            result["keyset"]  = KEYSET;
        }
        return result;
    }
    static const ValueToStr& value_to_str()
    {
        static ValueToStr result;
        if (result.empty()) {
            const StrToValue &str2val = str_to_value();
            for (StrToValue::const_iterator ptr = str2val.begin(); ptr != str2val.end(); ++ptr) {
                result[ptr->second] = ptr->first;
            }
            if (str2val.size() != result.size()) {
                throw std::runtime_error("Type::str_to_value() returns map with non-unique values");
            }
        }
        return result;
    }
};

}//Fred::Object
}//Fred

#endif//OBJECT_TYPE_H_22A124B8D173FCF75E30657FA4D8D922
