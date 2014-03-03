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

#include "util/db/query_param.h"
#include "util/db/value.h"
#include <stdexcept>

template<typename T>
class Nullable
{
public:
    typedef T Type;
private:
    bool isnull_;
    Type value_;
    template < typename Tc >
    friend class Nullable; // private members of Nullable< X > are accessible in Nullable< T >
public:
    // default ctor => value is NULL
    Nullable()
        : isnull_(true), // value is NULL
          value_()       // default value
    {}

    // init ctor; conversion Tc -> T must be possible
    template < typename Tc >
    Nullable(const Tc &_value)
        : isnull_(false), // value isn't NULL
          value_(_value)  // conversion Tc -> T exists
    {}

    // copy ctor; an Nullable can be constructed from another Nullable of a different but convertible type
    template < typename Tc >
    Nullable(const Nullable< Tc > &_rhs)
        : isnull_(_rhs.isnull_),
          value_(_rhs.value_)
    {}

    // assignment; an Nullable can be assigned value of a different but convertible type
    template < typename Tc >
    Nullable& operator=(const Tc &_value)
    {
        value_ = _value;
        isnull_ = false;
        return *this;
    }

    // assignment; an Nullable can be assigned another Nullable of a different but convertible type
    template < typename Tc >
    Nullable& operator=(const Nullable< Tc > &_rhs)
    {
        value_ = _rhs.value_;
        isnull_ = _rhs.isnull_;
        return *this;
    }

    bool isnull() const
    {
        return isnull_;
    }

    T get_value() const
    {
        if (isnull())
        {
            throw std::logic_error("value is null");
        }

        return value_;
    }

    T get_value_or_default() const
    {
        return value_;
    }

    operator Database::QueryParam()
    {
        if (isnull())
            return ::Database::QueryParam();
        else
            return ::Database::QueryParam(value_);
    }

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


    friend std::ostream& operator<<(std::ostream &_os, const Nullable<T> &_v)
    {
        static const char null_value_look[] = "[NULL]";
        return _v.isnull() ? _os << null_value_look : _os << _v.get_value();
    }

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

// comparison of equality
template < typename T >
bool operator==(const Nullable< T > &_a, const Nullable< T > &_b)
{
    return (!_a.isnull() && !_b.isnull() && (_a.get_value_or_default() == _b.get_value_or_default())) ||
           (_a.isnull() && _b.isnull());
}

template < typename T >
bool operator==(const Nullable< T > &_a, const T &_b)
{
    return !_a.isnull() && (_a.get_value_or_default() == _b);
}

template < typename T >
bool operator==(const T &_a, const Nullable< T > &_b)
{
    return !_b.isnull() && (_a == _b.get_value_or_default());
}

// comparison of inequality
template < typename T >
bool operator!=(const Nullable< T > &_a, const Nullable< T > &_b)
{
    return !(_a == _b);
}

template < typename T >
bool operator!=(const Nullable< T > &_a, const T &_b)
{
    return !(_a == _b);
}

template < typename T >
bool operator!=(const T &_a, const Nullable< T > &_b)
{
    return !(_a == _b);
}

// never use Nullable< T* >; isnull() has two meanings
template < typename T > class Nullable< T* >;

#endif
