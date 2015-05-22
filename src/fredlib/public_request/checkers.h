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
 *  declaration of Check template class
 */

#ifndef CHECKERS_H_D5C22F5DFA53E06F6886FD6DE0FD30C6//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CHECKERS_H_D5C22F5DFA53E06F6886FD6DE0FD30C6

#include "src/fredlib/contact_verification/contact.h"
#include "src/fredlib/opcontext.h"

#include <boost/static_assert.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/void.hpp>
#include <boost/regex.hpp>

/// Fred
namespace Fred {
/// PublicRequest
namespace PublicRequest {

std::string email_phone_protection_period() { return "1MONTH"; }

const boost::regex& phone_pattern()
{
    static const boost::regex pattern("[[:space:]]*\\+[0-9]{1,3}\\.[0-9]{1,14}[[:space:]]*");
    return pattern;
};

struct check_contact_name
{
    check_contact_name(const Contact::Verification::Contact &_data);
    bool success()const { return !(first_name_absents || last_name_absents); }
    bool first_name_absents:1;
    bool last_name_absents:1;
};

struct check_contact_mailing_address
{
    check_contact_mailing_address(const Contact::Verification::Contact &_data);
    bool success()const { return !(street1_absents || city_absents || postalcode_absents || country_absents); }
    bool street1_absents:1;
    bool city_absents:1;
    bool postalcode_absents:1;
    bool country_absents:1;
};

struct check_contact_email_presence
{
    check_contact_email_presence(const Contact::Verification::Contact &_data);
    bool success()const { return !absents; }
    bool absents:1;
};

struct check_contact_email_validity
{
    check_contact_email_validity(const Contact::Verification::Contact &_data);
    bool success()const { return !invalid; }
    bool invalid:1;
};

struct check_contact_email_availability:check_contact_email_presence
{
    check_contact_email_availability(const Contact::Verification::Contact &_data, OperationContext &_ctx);
    bool success()const { return this->check_contact_email_presence::success() && !used_recently; }
    bool used_recently:1;
};

struct check_contact_phone_presence
{
    check_contact_phone_presence(const Contact::Verification::Contact &_data);
    bool success()const { return !absents; }
    bool absents:1;
};

struct check_contact_phone_validity
{
    check_contact_phone_validity(const Contact::Verification::Contact &_data);
    bool success()const { return !invalid; }
    bool invalid:1;
};

struct check_contact_phone_availability:check_contact_phone_presence
{
    check_contact_phone_availability(const Contact::Verification::Contact &_data, OperationContext &_ctx);
    bool success()const { return this->check_contact_phone_presence::success() && !used_recently; }
    bool used_recently:1;
};

struct check_contact_fax_validity
{
    check_contact_fax_validity(const Contact::Verification::Contact &_data);
    bool success()const { return !invalid; }
    bool invalid:1;
};

namespace MojeID {

const boost::regex& username_pattern()
{
    static const boost::regex pattern("[0-9A-Za-z]([-0-9A-Za-z]{0,28}[0-9A-Za-z])?");
    return pattern;
};

struct check_contact_username
{
    check_contact_username(const Contact::Verification::Contact &_data);
    bool success()const { return !(absents || invalid); }
    bool absents:1;
    bool invalid:1;
};

struct check_contact_birthday_validity
{
    check_contact_birthday_validity(const Contact::Verification::Contact &_data);
    bool success()const { return !invalid; }
    bool invalid:1;
};

}//Fred::PublicRequest::MojeID

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
    BOOST_STATIC_ASSERT(2 < SIZE);
    BOOST_STATIC_ASSERT(SIZE == boost::mpl::size< CHECKERS >::type::value);
    typedef typename boost::mpl::front< CHECKERS >::type Current;
    typedef Check< typename boost::mpl::pop_front< CHECKERS >::type > Others;
    /**
     * Executes collection of partial checks on arbitrary data type.
     * @param _data checked data
     */
    template < typename DATA >
    Check(DATA _data)
    :   Current(_data),
        Others(_data)
    { }
    /**
     * Executes collection of partial checks on arbitrary two arguments.
     * @param _data0 first argument
     * @param _data1 second argument
     */
    template < typename DATA0, typename DATA1 >
    Check(DATA0 _data0, DATA1 _data1)
    :   Current(_data0, _data1),
        Others(_data0, _data1)
    { }
    /**
     * Copy constructor.
     * @param _src source data
     */
    Check(const Check &_src)
    :   Current(_src),
        Others(_src)
    { }
    /**
     * Checks finished successfully.
     * @return true if all partial checks finished successfully
     */
    bool success()const
    {
        return this->Current::success() && this->Others::success();
    }
};

/**
 * Specialization for SIZE=2. It stops the recursion.
 */
template < typename CHECKERS >
struct Check< CHECKERS, 2 >
:   boost::mpl::front< CHECKERS >::type,
    boost::mpl::front< typename boost::mpl::pop_front< CHECKERS >::type >::type
{
    BOOST_STATIC_ASSERT(boost::mpl::size< CHECKERS >::type::value == 2);
    typedef typename boost::mpl::front< CHECKERS >::type Current;
    typedef typename boost::mpl::front< typename boost::mpl::pop_front< CHECKERS >::type >::type Last;
    template < typename DATA >
    Check(DATA _data)
    :   Current(_data),
        Last(_data)
    { }
    template < typename DATA0, typename DATA1 >
    Check(DATA0 _data0, DATA1 _data1)
    :   Current(_data0, _data1),
        Last(_data0, _data1)
    { }
    Check(const Check &_src)
    :   Current(_src),
        Last(_src)
    { }
    bool success()const
    {
        return this->Current::success() && this->Last::success();
    }
};

/**
 * Encapsulates up to 3 function arguments into one object.
 * @param T0 type of first argument
 * @param T1 type of second argument
 * @param T2 type of third argument
 */
template < typename T0, typename T1 = boost::mpl::void_, typename T2 = boost::mpl::void_ >
struct Args;

template < typename T0 >
struct Args< T0 >
{
    typedef T0 Current;
    template < ::size_t IDX, typename T = Current > struct At;
    template < typename T > struct At< 0, T >
    {
        typedef T type;
        static type value(const Args &_args) { return _args.v; }
    };
    Args(T0 _v0):v(_v0) { }
    Current v;
};

template < typename T0, typename T1 >
struct Args< T0, T1 >:Args< T1 >
{
    typedef T0 Current;
    typedef Args< T1 > Others;
    template < ::size_t IDX, typename T = Current > struct At
    {
        typedef typename Others::template At< IDX - 1 >::type type;
        static type value(const Args &_args) { return Others::template At< IDX - 1 >::value(_args); }
    };
    template < typename T > struct At< 0, T >
    {
        typedef T type;
        static type value(const Args &_args) { return _args.v; }
    };
    Args(T0 _v0, T1 _v1):Others(_v1), v(_v0) { }
    Current v;
};

template < typename T0, typename T1, typename T2 >
struct Args:Args< T1, T2 >
{
    typedef T0 Current;
    typedef Args< T1, T2 > Others;
    template < ::size_t IDX, typename T = Current > struct At
    {
        typedef typename Others::template At< IDX - 1 >::type type;
        static type value(const Args &_args) { return Others::template At< IDX - 1 >::value(_args); }
    };
    template < typename T > struct At< 0, T >
    {
        typedef T type;
        static type value(const Args &_args) { return _args.v; }
    };
    Args(T0 _v0, T1 _v1, T2 _v2):Others(_v1, _v2), v(_v0) { }
    Current v;
};

/**
 * Joins three arguments of arbitrary types into one object.
 * @param a0 first argument
 * @param a1 second argument
 * @param a2 third argument
 * @return collection of three arguments
 */
template < typename T0, typename T1, typename T2 >
Args< T0, T1, T2 > make_args(T0 a0, T1 a1, T2 a2) { return Args< T0, T1, T2 >(a0, a1, a2); }

/**
 * Joins two arguments of arbitrary types into one object.
 * @param a0 first argument
 * @param a1 second argument
 * @return collection of two arguments
 */
template < typename T0, typename T1 >
Args< T0, T1 > make_args(T0 a0, T1 a1) { return Args< T0, T1 >(a0, a1); }

/**
 * Encapsulates one argument of arbitrary type into one object.
 * @param a0 first argument
 * @return object with this argument
 */
template < typename T0 >
Args< T0 > make_args(T0 a0) { return Args< T0 >(a0); }

/**
 * Unifies constructor call with different number of arguments into one uniform interface.
 * @param C class which constructor is called
 */
template < typename C >
struct ConstructWithArgs:C
{
    /**
     * Constructor with three arguments is used.
     */
    template < typename T0, typename T1, typename T2 >
    ConstructWithArgs(const Args< T0, T1, T2 > &_a)
    :   C(Args< T0, T1, T2 >::template At< 0 >::value(_a),
          Args< T0, T1, T2 >::template At< 1 >::value(_a),
          Args< T0, T1, T2 >::template At< 2 >::value(_a)) { }
    /**
     * Constructor with two arguments is used.
     */
    template < typename T0, typename T1 >
    ConstructWithArgs(const Args< T0, T1 > &_a)
    :   C(Args< T0, T1 >::template At< 0 >::value(_a),
          Args< T0, T1 >::template At< 1 >::value(_a)) { }
    /**
     * Constructor with one argument is used.
     */
    template < typename T0 >
    ConstructWithArgs(const Args< T0 > &_a)
    :   C(Args< T0 >::template At< 0 >::value(_a)) { }
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
template < typename CHECK0, typename CHECK1, typename CHECK2 = boost::mpl::void_,
           typename CHECK3 = boost::mpl::void_, typename CHECK4 = boost::mpl::void_ >
struct HCheck
:   ConstructWithArgs< CHECK0 >,
    HCheck< CHECK1, CHECK2, CHECK3, CHECK4 >
{
    typedef ConstructWithArgs< CHECK0 > Current;
    typedef HCheck< CHECK1, CHECK2, CHECK3, CHECK4 > Others;
    /**
     * Executes 5 checks with different sets of arguments.
     * @param _a0 arguments for first check
     * @param _a1 arguments for second check
     * @param _a2 arguments for third check
     * @param _a3 arguments for fourth check
     * @param _a4 arguments for fifth check
     * @note arguments can be created by using @ref make_args template function
     */
    template < typename ARG0, typename ARG1, typename ARG2, typename ARG3, typename ARG4 >
    HCheck(const ARG0 &_a0, const ARG1 &_a1, const ARG2 &_a2, const ARG3 &_a3, const ARG4 &_a4)
    : Current(_a0), Others(_a1, _a2, _a3, _a4) { }
    /**
     * Checks finished successfully.
     * @return true if all checks finished successfully
     */
    bool success()const { return this->Current::success() && this->Others::success(); }
};

template < typename CHECK0, typename CHECK1, typename CHECK2, typename CHECK3 >
struct HCheck< CHECK0, CHECK1, CHECK2, CHECK3 >
:   ConstructWithArgs< CHECK0 >,
    HCheck< CHECK1, CHECK2 >
{
    typedef ConstructWithArgs< CHECK0 > Current;
    typedef HCheck< CHECK1, CHECK2, CHECK3 > Others;
    template < typename ARG0, typename ARG1, typename ARG2, typename ARG3 >
    HCheck(const ARG0 &_a0, const ARG1 &_a1, const ARG2 &_a2, const ARG3 &_a3)
    : Current(_a0), Others(_a1, _a2, _a3) { }
    bool success()const { return this->Current::success() && this->Others::success(); }
};

template < typename CHECK0, typename CHECK1, typename CHECK2 >
struct HCheck< CHECK0, CHECK1, CHECK2 >
:   ConstructWithArgs< CHECK0 >,
    HCheck< CHECK1, CHECK2 >
{
    typedef ConstructWithArgs< CHECK0 > Current;
    typedef HCheck< CHECK1, CHECK2 > Others;
    template < typename ARG0, typename ARG1, typename ARG2 >
    HCheck(const ARG0 &_a0, const ARG1 &_a1, const ARG2 &_a2)
    : Current(_a0), Others(_a1, _a2) { }
    bool success()const { return this->Current::success() && this->Others::success(); }
};

/**
 * Specialization for two checks. It stops the recursion.
 */
template < typename CHECK0, typename CHECK1 >
struct HCheck< CHECK0, CHECK1 >
:   ConstructWithArgs< CHECK0 >,
    ConstructWithArgs< CHECK1 >
{
    typedef ConstructWithArgs< CHECK0 > Current;
    typedef ConstructWithArgs< CHECK1 > Last;
    template < typename ARG0, typename ARG1 >
    HCheck(const ARG0 &_a0, const ARG1 &_a1)
    : Current(_a0), Last(_a1) { }
    bool success()const { return this->Current::success() && this->Last::success(); }
};

}//Fred::PublicRequest
}//Fred

#endif//CHECKERS_H_D5C22F5DFA53E06F6886FD6DE0FD30C6
