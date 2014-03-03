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
 *  @file
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

/** Optional value template
 *
 * Maintains value of type T and flag if it has already been set (i. e. initialized or assigned).
 */

template<typename T>
class Optional
{
public:
    typedef T Type;
private:
    bool isset_;
    Type value_;
    template < typename Tc >
    friend class Optional; ///< convenient for copy-ctors and assignment operators because Optional<Tc> is not related to Optional<T>
public:

    /** default c-tor
     *
     * No value is given so object is "not set".
     * Calls default constructor of parameter type.
     */
    Optional()
        : isset_(false),
          value_()
    {}

    /** "initialization operator"
     *
     * Value is given so object is "set".
     * By using template
     * - Tc doesn't have to be exactly T
     * - but also all types that can be implicitly converted to T can be used as Tc
     *
     * Sanity of conversion is implicitly guaranteed by template instantiation mechanism.
     */
    template < typename Tc >
    Optional(const Tc &_value)
        : isset_(true),
          value_(_value)
    {}

    /** generalized copy c-tor
     *
     * Object is "set" if and only if the initialization object is "set".
     * By using template
     * - Optional<Tc> doesn't have to be exactly Optional<T>
     * - but all types that can be implicitly converted to T can be used as Tc
     *
     * Sanity of conversion is implicitly guaranteed by template instantiation mechanism.
     */
    template < typename Tc >
    Optional(const Optional< Tc > &_rhs)
        : isset_(_rhs.isset_),
          value_(_rhs.value_)
    {}

    /** "setter"
     *
     * Value is given so object is "set".
     * By using template
     * - Tc doesn't have to be exactly T
     * - but also all types that can be implicitly converted to T can be used as Tc
     *
     * Sanity of conversion is implicitly guaranteed by template instantiation mechanism.
     */
    template < typename Tc >
    Optional& operator=(const Tc &_value)
    {
        value_ = _value;
        isset_ = true;
        return *this;
    }

    /** generalized assignment operator
     *
     * Object is "set" if and only if the assigned object is "set".
     * By using template
     * - Optional<Tc> doesn't have to be exactly Optional<T>
     * - but all types that can be implicitly converted to T can be used as Tc
     *
     * Sanity of conversion is implicitly guaranteed by template instantiation mechanism.
     */
    template < typename Tc >
    Optional& operator=(const Optional< Tc > &_rhs)
    {
        value_ = _rhs.value_;
        isset_ = _rhs.isset_;
        return *this;
    }

    /**
     * @returns flag whether object has already been set
     */
    bool isset() const
    {
        return isset_;
    }

    /**
     * @returns "normal" value of the object
     * @throws std::logic_error in case object has not been set
     */
    T get_value() const
    {
        if (!isset())
        {
            throw std::logic_error("value is not set");
        }

        return value_;
    }

    /**
     * @returns "normal" value of object and in case it has not been set it returns default value of type T (defined by it's default constructor)
     */
    T get_value_or_default() const
    {
        return value_;
    }

    /**
     * overloaded printing to ostream
     */
    friend std::ostream& operator<<(std::ostream& os, const Optional<T>& ov)
    {
        using namespace arbitrary_pair_ostream_support;
        return os << ov.get_value();
    }

    /**
     * serialization (conversion to string) of object
     */
    std::string print_quoted() const
    {
        std::stringstream ss;
        if(isset()) ss << (*this);
        return isset() ? std::string("'") + ss.str() + "'" : std::string("[N/A]");
    }

};

/**
 * comparison operator
 *
 * Optional<T> == Optional<T> overload
 *
 * Intentionaly prohibiting Optional<X> == Optional<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator==(const Optional< T > &_a, const Optional< T > &_b)
{
    return (_a.isset() && _b.isset() && (_a.get_value() == _b.get_value())) ||
           (!_a.isset() && !_b.isset());
}

/**
 * comparison operator
 *
 * Optional<T> == T overload
 *
 * Intentionaly prohibiting Optional<X> == Y even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator==(const Optional< T > &_a, const T &_b)
{
    return _a.isset() && (_a.get_value() == _b);
}

/**
 * comparison operator
 *
 * T == Optional<T> overload
 *
 * Intentionaly prohibiting X == Optional<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator==(const T &_a, const Optional< T > &_b)
{
    return _b.isset() && (_a == _b.get_value());
}

/**
 * comparison operator
 *
 * Optional<T> != Optional<T> overload
 *
 * Intentionaly prohibiting Optional<X> != Optional<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator!=(const Optional< T > &_a, const Optional< T > &_b)
{
    return !(_a == _b);
}

/**
 * comparison operator
 *
 * Optional<T> != T overload
 *
 * Intentionaly prohibiting Optional<X> != Y even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator!=(const Optional< T > &_a, const T &_b)
{
    return !(_a == _b);
}

/**
 * comparison operator
 *
 * T != Optional<T> overload
 *
 * Intentionaly prohibiting X != Optional<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator!=(const T &_a, const Optional< T > &_b)
{
    return !(_a == _b);
}

#endif
