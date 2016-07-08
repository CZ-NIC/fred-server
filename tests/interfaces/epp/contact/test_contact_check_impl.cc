/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 */

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

#include "tests/interfaces/epp/util.h"
#include "tests/interfaces/epp/contact/fixture.h"

#include "src/epp/contact/contact_check_impl.h"

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_FIXTURE_TEST_SUITE(ContactCheckImpl, Test::autocommitting_context)

BOOST_AUTO_TEST_CASE(test_result_size_empty)
{
    Fred::OperationContextCreator ctx;

    BOOST_CHECK_EQUAL(
        Epp::contact_check_impl(
            ctx,
            std::set<std::string>()
        ).size(),
        0
    );
}

BOOST_AUTO_TEST_CASE(test_result_size_nonempty)
{
    const std::set<std::string> contact_handles
        = boost::assign::list_of
            ("a")
            ("b")
            ("c")
            ("d")
            ("e")
            ("a1")
            ("b1")
            ("c1")
            ("d1")
            ("e1");

    Fred::OperationContextCreator ctx;

    BOOST_CHECK_EQUAL(
        Epp::contact_check_impl(
            ctx,
            contact_handles
        ).size(),
        contact_handles.size()
    );
}

BOOST_AUTO_TEST_CASE(test_invalid_handle)
{
    const std::set<std::string> contact_handles
        = boost::assign::list_of
            ("")
            ("1234567890123456789012345678901234567890123456789012345678901234") // <== 64 chars
            ("*")
            ("!")
            ("@")
            ("a*")
            ("*a")
            ("a*a");

    Fred::OperationContextCreator ctx;

    const std::map<std::string, Nullable<Epp::ContactHandleRegistrationObstruction::Enum> > check_res =
        Epp::contact_check_impl(
            ctx,
            contact_handles
        );

    for(std::map<std::string, Nullable<Epp::ContactHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
        it != check_res.end();
        ++it
    ) {
        BOOST_CHECK(it->second.get_value() == Epp::ContactHandleRegistrationObstruction::invalid_handle);
    }
}

struct has_protected_handles : has_registrar {
    std::set<std::string> protected_handles;
    has_protected_handles() {
        protected_handles = boost::assign::list_of
            ("protHandleX1")
            ("protHandleX2")
            ("protHandleX3")
            ("protHandleXa")
            ("protHandleXb")
            ("protHandleXc");

        BOOST_FOREACH(const std::string& handle, protected_handles) {
            Fred::CreateContact(handle, registrar.handle).exec(ctx);
            Fred::DeleteContactByHandle(handle).exec(ctx);
        }
    }
};

BOOST_FIXTURE_TEST_CASE(test_protected_handle, has_protected_handles)
{
    const std::map<std::string, Nullable<Epp::ContactHandleRegistrationObstruction::Enum> > check_res =
        Epp::contact_check_impl(
            ctx,
            protected_handles
        );

    for(std::map<std::string, Nullable<Epp::ContactHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
        it != check_res.end();
        ++it
    ) {
        BOOST_CHECK(it->second.get_value() == Epp::ContactHandleRegistrationObstruction::protected_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(test_nonexistent_handle, Test::autocommitting_context)
{
    const std::set<std::string> contact_handles
        = boost::assign::list_of
            ("abc123")
            ("def234")
            ("ghi345")
            ("jkl456")
            ("mno567")
            ("pqr678")
            ("xyz789");

    Fred::OperationContextCreator ctx;

    const std::map<std::string, Nullable<Epp::ContactHandleRegistrationObstruction::Enum> > check_res =
        Epp::contact_check_impl(
            ctx,
            contact_handles
        );

    for(std::map<std::string, Nullable<Epp::ContactHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
        it != check_res.end();
        ++it
    ) {
        BOOST_CHECK( it->second.isnull() );
    }
}

struct has_existing_contacts : has_registrar {
    std::set<std::string> existing_contact_handles;
    has_existing_contacts() {
        existing_contact_handles = boost::assign::list_of
            ("handle01")
            ("handle02")
            ("handle03")
            ("handle0a")
            ("handle0b")
            ("handle0c");

        BOOST_FOREACH(const std::string& handle, existing_contact_handles) {
            Fred::CreateContact(handle, registrar.handle).exec(ctx);
        }
    }
};

BOOST_FIXTURE_TEST_CASE(test_existing, has_existing_contacts)
{
    const std::map<std::string, Nullable<Epp::ContactHandleRegistrationObstruction::Enum> > check_res =
        Epp::contact_check_impl(
            ctx,
            existing_contact_handles
        );

    for(std::map<std::string, Nullable<Epp::ContactHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
        it != check_res.end();
        ++it
    ) {
        BOOST_CHECK(it->second.get_value() == Epp::ContactHandleRegistrationObstruction::registered_handle);
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
