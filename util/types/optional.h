/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @optional.h
 *  optional types
 */

#ifndef OPTIONAL_H_
#define OPTIONAL_H_

#include <string>

/**
 * \class OptionalType
 * \brief optional type template
 * TYPE has to be constructible, copyable and assignable
 */
template <typename TYPE > class OptionalType
{
public:
    typedef  TYPE TypeOfValue;
private:
    TYPE value_;
    bool is_value_set_;
public:
    OptionalType()
    : value_()
    , is_value_set_(false)
    {}//ctor
    OptionalType(const TYPE& value)
    : value_(value)
    , is_value_set_(true)
    {}//init
    OptionalType(OptionalType const & rhs)
    : value_(rhs.value_)
    , is_value_set_(rhs.is_value_set_)
    {}//copy
    OptionalType& operator=(OptionalType const & rhs)
    {
        if (this != &rhs)
        {
            value_=rhs.value_;
            is_value_set_ = rhs.is_value_set_;
        }
        return *this;
    }//assignment

    //getters
    TYPE get_value() const { return value_; }
    bool is_value_set() const { return is_value_set_; }

    // conversion is possible but explicit using look more readable
    operator bool () const { return is_value_set_; }
    operator const TYPE& () const { return value_; }
};//OptionalType

typedef OptionalType<std::string> optional_string;
typedef OptionalType<unsigned long long> optional_id;
typedef OptionalType<unsigned long long> optional_ulonglong;
typedef OptionalType<unsigned long> optional_ulong;
typedef OptionalType<long> optional_long;
typedef OptionalType<bool> optional_bool;
typedef OptionalType<double> optional_double;

//save ARG into VALUE
//boost program options notifiers not only for OptionalType
//VALUE have to be contructible from ARG
//and constructed VALUE have to be assignable
template <typename VALUE>
class save_arg
{
    VALUE& val_;
public:
    //ctor taking reference to variable where arg value will be stored
    save_arg(VALUE& val) : val_(val) {}

    template <typename ARG>
    void operator()(const ARG& arg)//assign value
    {
        //std::cout << "notify_arg: " << arg << std::endl;
        val_=VALUE(arg);
    }
};

typedef save_arg<optional_string> save_optional_string;
typedef save_arg<optional_id> save_optional_id;
typedef save_arg<optional_ulonglong> save_optional_ulonglong;
typedef save_arg<optional_ulong> save_optional_ulong;
typedef save_arg<optional_bool> save_optional_bool;
typedef save_arg<optional_double> save_optional_double;

#endif //OPTIONAL_H_

