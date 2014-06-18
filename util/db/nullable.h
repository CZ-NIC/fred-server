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
 *  nullable value
 */

#ifndef NULLABLE_H_10011114543210
#define NULLABLE_H_10011114543210

#include "util/db/value.h"
#include <stdexcept>

/** Nullable value template
 *
 * Maintains value of type T and flag if it has value or is null.
 */

template<typename T>
class Nullable
{
public:
    typedef T Type;
private:
    bool isnull_;
    Type value_;
    template < typename Tc >
    friend class Nullable; ///< convenient for copy-ctors and assignment operators because Nullable<Tc> is not related to Nullable<T>
public:

    /** default c-tor
     *
     * No value is given so object "is null".
     * Calls default constructor of parameter type.
     */
    Nullable()
        : isnull_(true),
          value_()
    {}

    /** "initialization operator"
     *
     * Value is given so object "is not null".
     * By using template
     * - Tc doesn't have to be exactly T
     * - but also all types that can be implicitly converted to T can be used as Tc
     *
     * Sanity of conversion is implicitly guaranteed by template instantiation mechanism.
     */
    template < typename Tc >
    Nullable(const Tc &_value)
        : isnull_(false),
          value_(_value)
    {}

    /** copy c-tor
     *
     * Object is "null" if and only if the initialization object is "null".
     *
     * @remark Always used instead of @ref Nullable(const Nullable< Tc > &_rhs) (see 12.8 of C++ standard).
     */
    Nullable(const Nullable &_rhs)
        : isnull_(_rhs.isnull_),
          value_(_rhs.value_)
    {}

    /** generalized copy c-tor
     *
     * Object is "null" if and only if the initialization object is "null".
     * By using template
     * - Nullable<Tc> doesn't have to be exactly Nullable<T>
     * - but all types that can be implicitly converted to T can be used as Tc
     *
     * Sanity of conversion is implicitly guaranteed by template instantiation mechanism.
     */
    template < typename Tc >
    Nullable(const Nullable< Tc > &_rhs)
        : isnull_(_rhs.isnull_),
          value_(_rhs.value_)
    {}

    /** "setter"
     *
     * Value is given so object "is not null".
     * By using template
     * - Tc doesn't have to be exactly T
     * - but also all types that can be implicitly converted to T can be used as Tc
     *
     * Sanity of conversion is implicitly guaranteed by template instantiation mechanism.
     */
    template < typename Tc >
    Nullable& operator=(const Tc &_value)
    {
        value_ = _value;
        isnull_ = false;
        return *this;
    }

    /** assignment operator
     *
     * Object is "null" if and only if the assigned object is "null".
     *
     * @remark Always used instead of @ref operator=(const Nullable< Tc > &_rhs) (see 12.8 of C++ standard).
     */
    Nullable& operator=(const Nullable &_rhs)
    {
        value_ = _rhs.value_;
        isnull_ = _rhs.isnull_;
        return *this;
    }

    /** generalized assignment operator
     *
     * Object is "null" if and only if the assigned object is "null".
     * By using template
     * - Nullable<Tc> doesn't have to be exactly Nullable<T>
     * - but all types that can be implicitly converted to T can be used as Tc
     *
     * Sanity of conversion is implicitly guaranteed by template instantiation mechanism.
     */
    template < typename Tc >
    Nullable& operator=(const Nullable< Tc > &_rhs)
    {
        value_ = _rhs.value_;
        isnull_ = _rhs.isnull_;
        return *this;
    }

    /**
     * @returns flag whether object is null
     */
    bool isnull() const
    {
        return isnull_;
    }

    /**
     * @returns "normal" value of the object
     * @throws std::logic_error in case object is null
     */
    T get_value() const
    {
        if (isnull())
        {
            throw std::logic_error("value is null");
        }

        return value_;
    }

    /**
     * @returns "normal" value of object and in case it is null it returns default value of type T (defined by it's default constructor)
     */
    T get_value_or_default() const
    {
        return value_;
    }

    /**
     * Overload of assignment operator for Database::Value
     *
     * \remark In case the _v is null default value of Database::QueryParam is assigned to the object.
     */
    Nullable& operator=(const Database::Value &_v)
    {
        if (_v.isnull()) {
            isnull_ = true;
        }
        else {
            isnull_ = false;
            value_ = static_cast<T>(_v);
        }
        return *this;
    }

    /**
     * overloaded printing to ostream
     */
    friend std::ostream& operator<<(std::ostream &_os, const Nullable<T> &_v)
    {
        static const char null_value_look[] = "[NULL]";
        return _v.isnull() ? _os << null_value_look : _os << _v.get_value();
    }

    /**
     * serialization (conversion to string) of object
     */
    std::string print_quoted() const
    {
        std::ostringstream ss;
        if (isnull()) {
            ss << (*this);               // [NULL]
        }
        else {
            ss << "'" << (*this) << "'"; // 'value'
        }
        return ss.str();
    }
};

/**
 * comparison operator
 *
 * Nullable<T> == Nullable<T> overload
 *
 * Intentionaly prohibiting Nullable<X> == Nullable<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator==(const Nullable< T > &_a, const Nullable< T > &_b)
{
    return (!_a.isnull() && !_b.isnull() && (_a.get_value_or_default() == _b.get_value_or_default())) ||
           (_a.isnull() && _b.isnull());
}

/**
 * comparison operator
 *
 * Nullable<T> == T overload
 *
 * Intentionaly prohibiting Nullable<X> == Y even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator==(const Nullable< T > &_a, const T &_b)
{
    return !_a.isnull() && (_a.get_value_or_default() == _b);
}

/**
 * comparison operator
 *
 * T == Nullable<T> overload
 *
 * Intentionaly prohibiting X == Nullable<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator==(const T &_a, const Nullable< T > &_b)
{
    return !_b.isnull() && (_a == _b.get_value_or_default());
}

/**
 * comparison operator
 *
 * Nullable<T> != Nullable<T> overload
 *
 * Intentionaly prohibiting Nullable<X> != Nullable<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator!=(const Nullable< T > &_a, const Nullable< T > &_b)
{
    return !(_a == _b);
}

/**
 * comparison operator
 *
 * Nullable<T> != T overload
 *
 * Intentionaly prohibiting Nullable<X> != Y even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator!=(const Nullable< T > &_a, const T &_b)
{
    return !(_a == _b);
}

/**
 * comparison operator
 *
 * T != Nullable<T> overload
 *
 * Intentionaly prohibiting X != Nullable<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator!=(const T &_a, const Nullable< T > &_b)
{
    return !(_a == _b);
}

/**
 * assurance that Nullable< T* > is never used
 *
 * \warning Never use Nullable< T* > because isnull() method would be ambiguous.
 */
template < typename T > class Nullable< T* >;

#endif
