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
    Check< typename boost::mpl::pop_front< CHECKERS >::type, SIZE - 1 >
{
    BOOST_STATIC_ASSERT(SIZE == boost::mpl::size< CHECKERS >::type::value);
    typedef typename boost::mpl::front< CHECKERS >::type Current;
    typedef Check< typename boost::mpl::pop_front< CHECKERS >::type, SIZE - 1 > Others;
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
     * @param _data checked data
     * @param _ctx context for data checking
     */
    template < typename DATA, typename CTX >
    Check(DATA _data, CTX _ctx)
    :   Current(_data, _ctx),
        Others(_data, _ctx)
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
 * Specialization for SIZE=1. It stops the recursion.
 */
template < typename CHECKERS >
struct Check< CHECKERS, 1 >
:   boost::mpl::front< CHECKERS >::type
{
    BOOST_STATIC_ASSERT(boost::mpl::size< CHECKERS >::type::value == 1);
    typedef typename boost::mpl::front< CHECKERS >::type Current;
    template < typename DATA >
    Check(DATA _data)
    :   Current(_data)
    { }
    template < typename DATA, typename CTX >
    Check(DATA _data, CTX _ctx)
    :   Current(_data, _ctx)
    { }
    bool success()const
    {
        return this->Current::success();
    }
};

}//Fred::PublicRequest
}//Fred

#endif//CHECKERS_H_D5C22F5DFA53E06F6886FD6DE0FD30C6
