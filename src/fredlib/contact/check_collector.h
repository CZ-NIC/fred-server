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
 *  Check collection template classes
 */

#ifndef CHECK_COLLECTOR_H_88829F1E63D951E2C5975F79439ECDBF//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CHECK_COLLECTOR_H_88829F1E63D951E2C5975F79439ECDBF

#include <boost/static_assert.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/void.hpp>
#include <boost/regex.hpp>

/// Fred
namespace Fred {

/**
 * Executes partial checks and collects results of theirs.
 * @param CHECKERS a list of partial checks
 * @param SIZE number of partial checks, don't touch!!!
 * @note  CHECKERS is [boost::mpl::list](http://www.boost.org/doc/libs/1_48_0/libs/mpl/doc/refmanual/list.html)
 *        where each item represents one partial check class. Each item has to contain constructor with the
 *        same arguments and const method `success` without arguments like this example:
~~~~~~~~~~~~~~{.cpp}
struct partial_check_one
{
    partial_check_one(Data);
    bool success()const;
};

struct partial_check_two
{
    partial_check_two(Data);
    bool success()const;
};

typedef Check< list< partial_check_one, partial_check_two > > compound_check;
~~~~~~~~~~~~~~
 * This class publicly inherits from each partial check so details of all cheks are still accessible. It's to be
 * seen in more complex example:
~~~~~~~~~~~~~~{.cpp}
#include "src/fredlib/public_request/checkers.h"
#include <iostream>
#include <cstdlib>

namespace example
{

struct data
{
    std::string aaa;
    std::string bbb;
    std::string ccc;
};

struct check_item_aaa_presence_and_validity
{
    check_item_aaa_presence_and_validity(const data &_data)
    :   absent(_data.aaa.empty()),
        invalid(_data.aaa != "aaa")
    { }
    bool success()const { return !(absent || invalid); }
    bool absent:1;
    bool invalid:1;
};

struct check_item_bbb_presence_and_validity
{
    check_item_bbb_presence_and_validity(const data &_data)
    :   absent(_data.bbb.empty()),
        invalid(_data.bbb != "bbb")
    { }
    bool success()const { return !(absent || invalid); }
    bool absent:1;
    bool invalid:1;
};

struct check_item_ccc_presence_and_validity
{
    check_item_ccc_presence_and_validity(const data &_data)
    :   absent(_data.ccc.empty()),
        invalid(_data.ccc != "ccc")
    { }
    bool success()const { return !(absent || invalid); }
    bool absent:1;
    bool invalid:1;
};

typedef Fred::PublicRequest::Check< boost::mpl::list<
            check_item_aaa_presence_and_validity,
            check_item_bbb_presence_and_validity,
            check_item_ccc_presence_and_validity > > check_some_data_expectations;

typedef bool CheckSuccessful;

CheckSuccessful check()
{
    data d;
    d.aaa = "aaa";
    d.ccc = "cccx";
    const check_some_data_expectations r(d);
    if (r.success()) {
        return true;
    }
    if (!r.check_item_aaa_presence_and_validity::success()) {
        if (r.check_item_aaa_presence_and_validity::absent) {
            std::cout << "aaa absent" << std::endl;
        }
        if (r.check_item_aaa_presence_and_validity::invalid) {
            std::cout << "aaa invalid" << std::endl;
        }
    }
    if (!r.check_item_bbb_presence_and_validity::success()) {
        if (r.check_item_bbb_presence_and_validity::absent) {
            std::cout << "bbb absent" << std::endl;
        }
        if (r.check_item_bbb_presence_and_validity::invalid) {
            std::cout << "bbb invalid" << std::endl;
        }
    }
    if (!r.check_item_ccc_presence_and_validity::success()) {
        if (r.check_item_ccc_presence_and_validity::absent) {
            std::cout << "ccc absent" << std::endl;
        }
        if (r.check_item_ccc_presence_and_validity::invalid) {
            std::cout << "ccc invalid" << std::endl;
        }
    }
    return false;
}

}//example

int main()
{
    return example::check() ? EXIT_SUCCESS : EXIT_FAILURE;
}
~~~~~~~~~~~~~~
 */
template < typename CHECKERS, ::size_t SIZE = boost::mpl::size< CHECKERS >::type::value >
struct Check
:   boost::mpl::front< CHECKERS >::type,
    Check< typename boost::mpl::pop_front< CHECKERS >::type >
{
    BOOST_STATIC_ASSERT(1 < SIZE);
    BOOST_STATIC_ASSERT(SIZE == boost::mpl::size< CHECKERS >::type::value);
    typedef typename boost::mpl::front< CHECKERS >::type Current;
    typedef Check< typename boost::mpl::pop_front< CHECKERS >::type > Tail;
    /**
     * Executes collection of partial checks on arbitrary data type.
     * @param _data checked data
     */
    template < typename DATA >
    Check(DATA &_data)
    :   Current(_data),
        Tail(_data)
    { }
    /**
     * Executes collection of partial checks on arbitrary two arguments.
     * @param _data0 first argument
     * @param _data1 second argument
     */
    template < typename DATA0, typename DATA1 >
    Check(DATA0 &_data0, DATA1 &_data1)
    :   Current(_data0, _data1),
        Tail(_data0, _data1)
    { }
    /**
     * Checks finished successfully.
     * @return true if all partial checks finished successfully
     */
    bool success()const
    {
        return this->Current::success() && this->Tail::success();
    }
};

/**
 * Specialization for SIZE=1. It stops the recursion.
 */
template < typename CHECKERS >
struct Check< CHECKERS, 1 >
:   boost::mpl::front< CHECKERS >::type
{
    BOOST_STATIC_ASSERT(boost::mpl::size< CHECKERS >::type::value == 1);
    typedef typename boost::mpl::front< CHECKERS >::type Current;
    template < typename DATA >
    Check(DATA &_data)
    :   Current(_data)
    { }
    template < typename DATA0, typename DATA1 >
    Check(DATA0 &_data0, DATA1 &_data1)
    :   Current(_data0, _data1)
    { }
    bool success()const
    {
        return this->Current::success();
    }
};

/**
 * Encapsulates up to 5 function arguments into one object.
 * @param T0 type of first argument
 * @param T1 type of second argument
 * @param T2 type of third argument
 * @param T3 type of fourth argument
 * @param T4 type of fifth argument
 */
template < typename T0, typename T1 = boost::mpl::void_, typename T2 = boost::mpl::void_,
           typename T3 = boost::mpl::void_, typename T4 = boost::mpl::void_ >
struct Args
{
    typedef T0 Current;
    typedef Args< T1, T2, T3, T4 > Tail;
    Args(T0 &_v, const Tail &_t):v(_v), t(_t) { }
    Current& first() { return v; }
    const Tail& rest()const { return t; }
    template < ::size_t IDX, typename T = Current > struct At
    {
        typedef typename Tail::template At< IDX - 1 >::type type;
        static type& value(const Args &_args) { return _args.rest().template value< IDX - 1 >(); }
    };
    template < typename T > struct At< 0, T >
    {
        typedef T type;
        static type& value(const Args &_args) { return _args.v; }
    };
    template < ::size_t IDX >
    typename At< IDX >::type& value()const { return At< IDX >::value(*this); }
    Current &v;
    Tail t;
};

/**
 * Specialization for one argument.
 */
template < typename T0 >
struct Args< T0 >
{
    typedef T0 Current;
    Args(T0 &_v0):v(_v0) { }
    Current& first() { return v; }
    template < ::size_t IDX, typename T = Current > struct At;
    template < typename T > struct At< 0, T >
    {
        typedef T type;
        static type& value(const Args &_args) { return _args.v; }
    };
    template < ::size_t IDX >
    typename At< IDX >::type& value()const { return v; }
    Current &v;
};

/**
 * Encapsulates one argument of arbitrary type into one object.
 * @param a0 first argument
 * @return object with this argument
 */
template < typename T0 >
Args< T0 > make_args(T0 &a0) { return Args< T0 >(a0); }

/**
 * Joins two arguments of arbitrary types into one object.
 * @param a0 first argument
 * @param a1 second argument
 * @return collection of two arguments
 */
template < typename T0, typename T1 >
Args< T0, T1 > make_args(T0 &a0, T1 &a1) { return Args< T0, T1 >(a0, make_args(a1)); }

/**
 * Joins three arguments of arbitrary types into one object.
 * @param a0 first argument
 * @param a1 second argument
 * @param a2 third argument
 * @return collection of three arguments
 */
template < typename T0, typename T1, typename T2 >
Args< T0, T1, T2 > make_args(T0 &a0, T1 &a1, T2 &a2)
{ return Args< T0, T1, T2 >(a0, make_args(a1, a2)); }

/**
 * Joins four arguments of arbitrary types into one object.
 * @param a0 first argument
 * @param a1 second argument
 * @param a2 third argument
 * @param a3 fourth argument
 * @return collection of four arguments
 */
template < typename T0, typename T1, typename T2, typename T3 >
Args< T0, T1, T2, T3 > make_args(T0 &a0, T1 &a1, T2 &a2, T3 &a3)
{ return Args< T0, T1, T2, T3 >(a0, make_args(a1, a2, a3)); }

/**
 * Joins five arguments of arbitrary types into one object.
 * @param a0 first argument
 * @param a1 second argument
 * @param a2 third argument
 * @param a3 fourth argument
 * @param a4 fifth argument
 * @return collection of five arguments
 */
template < typename T0, typename T1, typename T2, typename T3, typename T4 >
Args< T0, T1, T2, T3, T4 > make_args(T0 &a0, T1 &a1, T2 &a2, T3 &a3, T4 &a4)
{ return Args< T0, T1, T2, T3, T4 >(a0, make_args(a1, a2, a3, a4)); }

/**
 * Unifies constructor call with different number of arguments into one uniform interface.
 * @param C class which constructor is called
 */
template < typename C >
struct ConstructWithArgs:C
{
    /**
     * Constructor with five arguments is used.
     */
    template < typename T0, typename T1, typename T2, typename T3, typename T4 >
    ConstructWithArgs(const Args< T0, T1, T2, T3, T4 > &_a)
    :   C(_a.template value< 0 >(),
          _a.template value< 1 >(),
          _a.template value< 2 >(),
          _a.template value< 3 >(),
          _a.template value< 4 >()) { }
    /**
     * Constructor with four arguments is used.
     */
    template < typename T0, typename T1, typename T2, typename T3 >
    ConstructWithArgs(const Args< T0, T1, T2, T3 > &_a)
    :   C(_a.template value< 0 >(),
          _a.template value< 1 >(),
          _a.template value< 2 >(),
          _a.template value< 3 >()) { }
    /**
     * Constructor with three arguments is used.
     */
    template < typename T0, typename T1, typename T2 >
    ConstructWithArgs(const Args< T0, T1, T2 > &_a)
    :   C(_a.template value< 0 >(),
          _a.template value< 1 >(),
          _a.template value< 2 >()) { }
    /**
     * Constructor with two arguments is used.
     */
    template < typename T0, typename T1 >
    ConstructWithArgs(const Args< T0, T1 > &_a)
    :   C(_a.template value< 0 >(),
          _a.template value< 1 >()) { }
    /**
     * Constructor with one argument is used.
     */
    template < typename T0 >
    ConstructWithArgs(const Args< T0 > &_a)
    :   C(_a.template value< 0 >()) { }
};

/**
 * Collection of partial checks with different arguments, restricted to a maximum 5 checks. The first 'H'
 * means heterogeneous due to different arguments for executing partial checks.
 * @param CHECK0 first partial check
 * @param CHECK1 second partial check
 * @param CHECK2 third partial check
 * @param CHECK3 fourth partial check
 * @param CHECK4 fifth partial check
 * @note  Each check has to contain const method `success` without arguments like this example:
~~~~~~~~~~~~~~{.cpp}
struct checkA0
{
    checkA0(int) { }
    bool success()const { return !invalid; }
    bool invalid:1;
};

struct checkA1
{
    checkA1(int) { }
    bool success()const { return !invalid; }
    bool invalid:1;
};

typedef Check< list< checkA0, checkA1 > > checkA;

struct checkB
{
    checkB(std::string) { }
    bool success()const { return !invalid; }
    bool invalid:1;
};

struct checkC
{
    checkC(void*, int) { }
    bool success()const { return !invalid; }
    bool invalid:1;
};

typedef HCheck< checkA, checkB, checkC > HCheckABC;

HCheckABC c(make_args(0), make_args("1"), make_args((void*)NULL, 2));
~~~~~~~~~~~~~~
 */
template < typename CHECK0, typename CHECK1 = boost::mpl::void_, typename CHECK2 = boost::mpl::void_,
           typename CHECK3 = boost::mpl::void_, typename CHECK4 = boost::mpl::void_ >
struct HCheck
:   ConstructWithArgs< CHECK0 >,
    HCheck< CHECK1, CHECK2, CHECK3, CHECK4 >
{
    typedef ConstructWithArgs< CHECK0 > Current;
    typedef HCheck< CHECK1, CHECK2, CHECK3, CHECK4 > Tail;
    /**
     * Executes 5 checks with different sets of arguments and stores their results into this object.
     * @param _a0 arguments for first checks
     * @param _a1 arguments for second checks
     * @param _a2 arguments for third checks
     * @param _a3 arguments for fourth checks
     * @param _a4 arguments for fifth checks
     * @note arguments can be created by using @ref make_args template function
     */
    template < typename T0, typename T1, typename T2, typename T3, typename T4 >
    HCheck(const T0 &_a0, const T1 &_a1, const T2 &_a2, const T3 &_a3, const T4 &_a4)
    :   Current(_a0),
        Tail(_a1, _a2, _a3, _a4) { }
    /**
     * Executes 4 checks with different sets of arguments and stores their results into this object.
     * @param _a0 arguments for first checks
     * @param _a1 arguments for second checks
     * @param _a2 arguments for third checks
     * @param _a3 arguments for fourth checks
     * @note arguments can be created by using @ref make_args template function
     */
    template < typename T0, typename T1, typename T2, typename T3 >
    HCheck(const T0 &_a0, const T1 &_a1, const T2 &_a2, const T3 &_a3)
    :   Current(_a0),
        Tail(_a1, _a2, _a3) { }
    /**
     * Executes 3 checks with different sets of arguments and stores their results into this object.
     * @param _a0 arguments for first checks
     * @param _a1 arguments for second checks
     * @param _a2 arguments for third checks
     * @note arguments can be created by using @ref make_args template function
     */
    template < typename T0, typename T1, typename T2 >
    HCheck(const T0 &_a0, const T1 &_a1, const T2 &_a2)
    :   Current(_a0),
        Tail(_a1, _a2) { }
    /**
     * Executes 2 checks with different sets of arguments and stores their results into this object.
     * @param _a0 arguments for first checks
     * @param _a1 arguments for second checks
     * @note arguments can be created by using @ref make_args template function
     */
    template < typename T0, typename T1 >
    HCheck(const T0 &_a0, const T1 &_a1)
    :   Current(_a0),
        Tail(_a1) { }
    /**
     * Checks finished successfully.
     * @return true if all checks finished successfully
     */
    bool success()const { return this->Current::success() && this->Tail::success(); }
};

/**
 * Specialization for one check. It stops the recursion.
 */
template < typename CHECK0 >
struct HCheck< CHECK0 >
:   ConstructWithArgs< CHECK0 >
{
    typedef ConstructWithArgs< CHECK0 > Current;
    template < typename T0 >
    HCheck(const T0 &_a0)
    :   Current(_a0) { }
    bool success()const { return this->Current::success(); }
};

}//Fred

#endif//CHECK_COLLECTOR_H_88829F1E63D951E2C5975F79439ECDBF
