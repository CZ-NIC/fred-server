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

#ifndef OPTIONAL_HH_42E8981D4545409A9C808870CB70787F
#define OPTIONAL_HH_42E8981D4545409A9C808870CB70787F

#include <algorithm>
#include <string>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>


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

template <> class OptionalType <boost::gregorian::date>
{
public:
    typedef  boost::gregorian::date TypeOfValue;
private:
    boost::gregorian::date value_;
    bool is_value_set_;
public:
    OptionalType()
    : value_()
    , is_value_set_(false)
    {}//ctor
    OptionalType(const boost::gregorian::date& value)
    : value_(value)
    , is_value_set_(true)
    {}//init
    OptionalType(const std::string& value)
        : value_(boost::gregorian::from_string(value))
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
    boost::gregorian::date get_value() const { return value_; }
    bool is_value_set() const { return is_value_set_; }

    // conversion is possible but explicit using look more readable
    operator bool () const { return is_value_set_; }
    operator const boost::gregorian::date& () const { return value_; }
};//OptionalType


template <> class OptionalType <boost::posix_time::ptime>
{
public:
    typedef  boost::posix_time::ptime TypeOfValue;
private:
    boost::posix_time::ptime value_;
    bool is_value_set_;
public:
    OptionalType()
    : value_()
    , is_value_set_(false)
    {}//ctor
    OptionalType(const boost::posix_time::ptime& value)
    : value_(value)
    , is_value_set_(true)
    {}//init
    OptionalType(const std::string& value)
        : value_(boost::posix_time::time_from_string(value))
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
    boost::posix_time::ptime get_value() const { return value_; }
    bool is_value_set() const { return is_value_set_; }

    // conversion is possible but explicit using look more readable
    operator bool () const { return is_value_set_; }
    operator const boost::posix_time::ptime& () const { return value_; }
};//OptionalType


typedef OptionalType<std::string> optional_string;
typedef OptionalType<unsigned long long> optional_id;
typedef OptionalType<unsigned long long> optional_ulonglong;
typedef OptionalType<unsigned long> optional_ulong;
typedef OptionalType<unsigned short> optional_ushort;
typedef OptionalType<long> optional_long;
typedef OptionalType<bool> optional_bool;
typedef OptionalType<double> optional_double;
typedef OptionalType<boost::gregorian::date> optional_date;
typedef OptionalType<boost::posix_time::ptime> optional_ptime;

//save ARG into VALUE
//boost program options notifier callback
//VALUE have to be constructible from ARG
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

template <>
class save_arg<boost::gregorian::date>
{
    boost::gregorian::date& val_;
public:
    //ctor taking reference to variable where arg value will be stored
    save_arg(boost::gregorian::date& val) : val_(val) {}

    void operator()(const std::string& arg)//assign value
    {
        //std::cout << "notify_arg: " << arg << std::endl;
        val_=boost::gregorian::from_string(arg);
    }
};

template <>
class save_arg<boost::posix_time::ptime>
{
    boost::posix_time::ptime& val_;
public:
    //ctor taking reference to variable where arg value will be stored
    save_arg(boost::posix_time::ptime& val) : val_(val) {}

    void operator()(const std::string& arg)//assign value
    {
        //std::cout << "notify_arg: " << arg << std::endl;
        val_=boost::posix_time::time_from_string(arg);
    }
};

template <>
class save_arg<std::vector<boost::asio::ip::address> >
{
    std::vector<boost::asio::ip::address>& val_;
public:
    save_arg(std::vector<boost::asio::ip::address>& val) : val_(val) {}

    void operator()(const std::vector<std::string>& arg)
    {
        std::for_each(arg.begin(), arg.end(),
                [this](const std::string& s) { val_.push_back(boost::asio::ip::address::from_string(s)); });
    }
};

typedef save_arg<optional_string> save_optional_string;
typedef save_arg<optional_id> save_optional_id;
typedef save_arg<optional_ulonglong> save_optional_ulonglong;
typedef save_arg<optional_ulong> save_optional_ulong;
typedef save_arg<optional_ushort> save_optional_ushort;
typedef save_arg<optional_bool> save_optional_bool;
typedef save_arg<optional_double> save_optional_double;
typedef save_arg<optional_date> save_optional_date;
typedef save_arg<optional_ptime> save_optional_ptime;

//for VALUEs and ARGs like std::vector or compatible
template <typename VALUE> class insert_arg
{
    VALUE& val_;
public:
    //ctor taking reference to variable where arg value will be stored
    insert_arg(VALUE& val) : val_(val) {}

    template <typename ARG>
    void operator()(const ARG& arg)//assign value
    {
        val_.insert(val_.end(), arg.begin(), arg.end());
    }
};//template <typename VALUE> class insert_arg


//save ARG into VALUE using ARG::to_string()
//boost program options notifier callback
//VALUE have to be constructible from ARG::to_string() return type
//and constructed VALUE have to be assignable
template <typename VALUE>
class save_arg_using_to_string
{
    VALUE& val_;
public:
    //ctor taking reference to variable where arg value will be stored
    save_arg_using_to_string(VALUE& val) : val_(val) {}

    template <typename ARG>
    void operator()(const ARG& arg)//assign value
    {
        val_=VALUE(arg.to_string());
    }
};


#endif //OPTIONAL_H_

