/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @optional_value.h
 *  optional value
 */

#ifndef OPTIONAL_VALUE_H_1134545331
#define OPTIONAL_VALUE_H_1134545331

#include <utility>
#include <string>
#include <sstream>
#include <stdexcept>

#include "util/printable.h"

namespace arbitrary_pair_ostream_support
{
    template<class T1, class T2> std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& input_pair)
    {
        return os << "first: " << input_pair.first << " second: " << input_pair.second;
    }

    template<class T1, class T2> std::ostream& operator<<(std::ostream& os, const std::pair<std::vector<T1>, std::vector<T2> >& input_pair)
    {
        return os << "first: " << Util::format_vector(input_pair.first) << " second: " << Util::format_vector(input_pair.second);
    }
}

//simple optional value template
template<typename T>
class Optional
{
public:
    typedef T Type;
private:
    bool isset_;
    Type value_;
    template < typename Tc >
    friend class Optional; // private members of Optional< X > are accessible in Optional< T >
public:
    // default ctor => value doesn't present
    Optional()
        : isset_(false), // value doesn't present
          value_()       // default value
    {}//default

    // init ctor; conversion Tc -> T must be possible
    template < typename Tc >
    Optional(const Tc &_value)
        : isset_(true),  // value present
          value_(_value) // conversion Tc -> T exists
    {}//init

    // copy ctor; an Optional can be constructed from another Optional of a different but convertible type
    template < typename Tc >
    Optional(const Optional< Tc > &_rhs)
        : isset_(_rhs.isset_),
          value_(_rhs.value_)
    {}//copy

    // assignment; an Optional can be assigned value of a different but convertible type
    template < typename Tc >
    Optional& operator=(const Tc &_value)
    {
        value_ = _value;
        isset_ = true;
        return *this;
    }//assignment

    // assignment; an Optional can be assigned another Optional of a different but convertible type
    template < typename Tc >
    Optional& operator=(const Optional< Tc > &_rhs)
    {
        value_ = _rhs.value_;
        isset_ = _rhs.isset_;
        return *this;
    }//assignment

    bool isset() const
    {
        return isset_;
    }

    T get_value() const
    {
        if (!isset())
        {
            throw std::logic_error("value is not set");
        }

        return value_;
    }

    T get_value_or_default() const
    {
        return value_;
    }

    friend std::ostream& operator<<(std::ostream& os, const Optional<T>& ov)
    {
        using namespace arbitrary_pair_ostream_support;
        return os << ov.get_value();
    }

    std::string print_quoted() const
    {
        std::stringstream ss;
        if(isset()) ss << (*this);
        return isset() ? std::string("'") + ss.str() + "'" : std::string("[N/A]");
    }

};

// comparison of equality
template < typename T >
bool operator==(const Optional< T > &_a, const Optional< T > &_b)
{
    return (_a.isset() && _b.isset() && (_a.get_value() == _b.get_value())) ||
           (!_a.isset() && !_b.isset());
}

template < typename T >
bool operator==(const Optional< T > &_a, const T &_b)
{
    return _a.isset() && (_a.get_value() == _b);
}

template < typename T >
bool operator==(const T &_a, const Optional< T > &_b)
{
    return _b.isset() && (_a == _b.get_value());
}

// comparison of inequality
template < typename T >
bool operator!=(const Optional< T > &_a, const Optional< T > &_b)
{
    return !(_a == _b);
}

template < typename T >
bool operator!=(const Optional< T > &_a, const T &_b)
{
    return !(_a == _b);
}

template < typename T >
bool operator!=(const T &_a, const Optional< T > &_b)
{
    return !(_a == _b);
}

#endif //OPTIONAL_VALUE_H_
