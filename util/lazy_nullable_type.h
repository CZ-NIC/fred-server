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
 *  lazily constructed Nullable value
 */

#ifndef LAZY_NULLABLE_H_4684768494103410
#define LAZY_NULLABLE_H_4684768494103410

#include "util/db/value.h"
#include <stdexcept>

/** LazyNullable value template
 *
 * Able to provide value of type T knows if it has been set or not.
 *
 * IMPORTANT: Actual call of constructor of Type T is deferred as much as possible (Lazy). Keep that in mind in regard to your type's side-effects.
 */

template<typename T> class LazyNullable {
public:
    typedef T Type;
private:
    template < typename Tc > friend class LazyNullable; ///< convenient for copy-ctors and assignment operators because LazyNullable<Tc> is not related to LazyNullable<T>

private:
    bool isnull_;
    boost::function<T()> value_provider_;

    // C++ standard forbids taking address of c-tors. This is workaround how to bind argument to constructor call.
    // https://stackoverflow.com/questions/954548/how-to-pass-a-function-pointer-that-points-to-constructor
    template<class TFrom, class TTo> static TTo indirect_ctor_call(const TFrom& init) {
        return TTo(init);
    }

public:


    /** default c-tor
     * No value is given so object "is null".
     */
    LazyNullable()
    :   isnull_(true),
        value_provider_()
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
    template<typename TInit> LazyNullable(const TInit &_value)
    :   isnull_(false),
        value_provider_(boost::bind(&indirect_ctor_call<TInit, T>, _value))
    { }

    /** copy c-tor
     * Object is "null" if and only if the initialization object is "null".
     *
     * @remark Always used instead of @ref LazyNullable(const LazyNullable< Tc > &_rhs) (see 12.8 of C++ standard).
     */
    LazyNullable(const LazyNullable &_rhs)
    :   isnull_(_rhs.isnull_),
        value_provider_(_rhs.value_provider_)
    { }

    // copy c-tor is not generalized yet as I want to keep it simple and don't want to deal with boost::function conversions

    /** "setter"
     * Value is given so object "is not null".
     * By using template
     * - Tc doesn't have to be exactly T
     * - but also all types that can be implicitly converted to T can be used as Tc
     *
     * Sanity of conversion is implicitly guaranteed by template instantiation mechanism.
     */
    template<typename Tc> LazyNullable& operator=(const Tc &_value) {
        value_provider_ = boost::bind(&indirect_ctor_call<Tc, T>, _value);
        isnull_ = false;

        return *this;
    }

    /** assignment operator
     * Object is "null" if and only if the assigned object is "null".
     *
     * @remark Always used instead of @ref operator=(const LazyNullable< Tc > &_rhs) (see 12.8 of C++ standard).
     */
    LazyNullable& operator=(const LazyNullable &_rhs) {
        value_provider_ = _rhs.value_provider_;
        isnull_ = _rhs.isnull_;

        return *this;
    }

    // assignment is not generalized yet as I want to keep it simple and don't want to deal with boost::function conversions

    /**
     * @returns flag whether object is null
     */
    bool isnull() const {
        return isnull_;
    }

    /**
     * @returns "normal" value of the object
     * @throws std::logic_error in case object is null
     */
    T get_value() const {
        if (isnull()) {
            throw std::logic_error("value is null");
        }

        return value_provider_();
    }

    /**
     * @returns "normal" value of object and in case it is null it returns default value of type T (defined by it's default constructor)
     */
    T get_value_or_default() const {
        if (isnull()) {
            return T();
        } else {
            return value_provider_();
        }
    }

    /**
     * Overload of assignment operator for Database::Value
     *
     * \remark In case the _v is null default value of Database::QueryParam is assigned to the object.
     */
    LazyNullable& operator=(const Database::Value &_v)
    {
        if (_v.isnull()) {
            isnull_ = true;
        }
        else {
            *this = static_cast<T>(_v);
        }
        return *this;
    }

    /**
     * overloaded printing to ostream
     */
    friend std::ostream& operator<<(std::ostream &_os, const LazyNullable<T> &_v)
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
 * LazyNullable<T> == LazyNullable<T> overload
 *
 * Intentionaly prohibiting LazyNullable<X> == LazyNullable<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator==(const LazyNullable< T > &_a, const LazyNullable< T > &_b) {
    return (!_a.isnull() && !_b.isnull() && (_a.get_value_or_default() == _b.get_value_or_default())) ||
           (_a.isnull() && _b.isnull());
}

/**
 * comparison operator
 *
 * LazyNullable<T> == T overload
 *
 * Intentionaly prohibiting LazyNullable<X> == Y even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator==(const LazyNullable< T > &_a, const T &_b) {
    return !_a.isnull() && (_a.get_value_or_default() == _b);
}

/**
 * comparison operator
 *
 * T == LazyNullable<T> overload
 *
 * Intentionaly prohibiting X == LazyNullable<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator==(const T &_a, const LazyNullable< T > &_b) {
    return !_b.isnull() && (_a == _b.get_value_or_default());
}

/**
 * comparison operator
 *
 * LazyNullable<T> != LazyNullable<T> overload
 *
 * Intentionaly prohibiting LazyNullable<X> != LazyNullable<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator!=(const LazyNullable< T > &_a, const LazyNullable< T > &_b) {
    return !(_a == _b);
}

/**
 * comparison operator
 *
 * LazyNullable<T> != T overload
 *
 * Intentionaly prohibiting LazyNullable<X> != Y even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator!=(const LazyNullable< T > &_a, const T &_b) {
    return !(_a == _b);
}

/**
 * comparison operator
 *
 * T != LazyNullable<T> overload
 *
 * Intentionaly prohibiting X != LazyNullable<Y> even for implicitly convertible types.
 * Guaranteed by the fact that template instantiation enforces exact type match and precedes overload resolution.
 */
template < typename T >
bool operator!=(const T &_a, const LazyNullable< T > &_b) {
    return !(_a == _b);
}

/**
 * assurance that LazyNullable< T* > is never used
 *
 * \warning Never use LazyNullable< T* > because isnull() method would be ambiguous.
 */
template < typename T > class LazyNullable< T* >;

#endif
