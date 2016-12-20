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
 * along with FRED.  If not, see <http://www.gnu.or/licenses/>.
 */

/**
 *  @file
 */

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

#include "tests/interfaces/epp/util.h"
#include "tests/interfaces/epp/nsset/fixture.h"

#include "src/epp/nsset/check_nsset.h"

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_FIXTURE_TEST_SUITE(CheckNsset, Test::autocommitting_context)

BOOST_FIXTURE_TEST_CASE(test_invalid_registrar_id, has_invalid_registrar_id)
{
    BOOST_CHECK_THROW(
        Fred::OperationContextCreator ctx;

        Epp::Nsset::check_nsset(
            ctx,
            std::set<std::string>(),
            invalid_registrar_id
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(test_result_size_empty, has_registrar)
{
    Fred::OperationContextCreator ctx;

    BOOST_CHECK_EQUAL(
        Epp::Nsset::check_nsset(
            ctx,
            std::set<std::string>(),
            registrar.id
        ).size(),
        0
    );
}

BOOST_FIXTURE_TEST_CASE(test_result_size_nonempty, has_registrar)
{
    const std::set<std::string> nsset_handles
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
            ("e1").convert_to_container<std::set<std::string> >();

    Fred::OperationContextCreator ctx;

    BOOST_CHECK_EQUAL(
        Epp::Nsset::check_nsset(
            ctx,
            nsset_handles,
            registrar.id
        ).size(),
        nsset_handles.size()
    );
}

BOOST_FIXTURE_TEST_CASE(test_invalid_handle, has_registrar)
{
    const std::set<std::string> nsset_handles
        = boost::assign::list_of
            ("")
            ("1234567890123456789012345678901234567890123456789012345678901234") // <== 64 chars
            ("*")
            ("!")
            ("@")
            ("a*")
            ("*a")
            ("a*a").convert_to_container<std::set<std::string> >();

    Fred::OperationContextCreator ctx;

    const std::map<std::string, Nullable<Epp::Nsset::NssetHandleRegistrationObstruction::Enum> > check_res =
        Epp::Nsset::check_nsset(
            ctx,
            nsset_handles,
            registrar.id
        );

    for(std::map<std::string, Nullable<Epp::Nsset::NssetHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
        it != check_res.end();
        ++it
    ) {
        BOOST_CHECK(it->second.get_value() == Epp::Nsset::NssetHandleRegistrationObstruction::invalid_handle);
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
            ("protHandleXc").convert_to_container<std::set<std::string> >();

        BOOST_FOREACH(const std::string& handle, protected_handles) {
            Fred::CreateNsset(handle, registrar.handle).exec(ctx);
            Fred::DeleteNssetByHandle(handle).exec(ctx);
        }
    }
};

BOOST_FIXTURE_TEST_CASE(test_protected_handle, has_protected_handles)
{
    const std::map<std::string, Nullable<Epp::Nsset::NssetHandleRegistrationObstruction::Enum> > check_res =
        Epp::Nsset::check_nsset(
            ctx,
            protected_handles,
            registrar.id
        );

    for(std::map<std::string, Nullable<Epp::Nsset::NssetHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
        it != check_res.end();
        ++it
    ) {
        BOOST_CHECK(it->second.get_value() == Epp::Nsset::NssetHandleRegistrationObstruction::protected_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(test_nonexistent_handle, has_registrar)
{
    const std::set<std::string> nsset_handles
        = boost::assign::list_of
            ("abc123")
            ("def234")
            ("ghi345")
            ("jkl456")
            ("mno567")
            ("pqr678")
            ("xyz789").convert_to_container<std::set<std::string> >();

    Fred::OperationContextCreator ctx;

    const std::map<std::string, Nullable<Epp::Nsset::NssetHandleRegistrationObstruction::Enum> > check_res =
        Epp::Nsset::check_nsset(
            ctx,
            nsset_handles,
            registrar.id
        );

    for(std::map<std::string, Nullable<Epp::Nsset::NssetHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
        it != check_res.end();
        ++it
    ) {
        BOOST_CHECK( it->second.isnull() );
    }
}

struct has_existing_nssets : has_registrar {
    std::set<std::string> existing_nsset_handles;
    has_existing_nssets() {
        existing_nsset_handles = boost::assign::list_of
            ("handle01")
            ("handle02")
            ("handle03")
            ("handle0a")
            ("handle0b")
            ("handle0c").convert_to_container<std::set<std::string> >();

        BOOST_FOREACH(const std::string& handle, existing_nsset_handles) {
            Fred::CreateNsset(handle, registrar.handle).exec(ctx);
        }
    }
};

BOOST_FIXTURE_TEST_CASE(test_existing, has_existing_nssets)
{
    const std::map<std::string, Nullable<Epp::Nsset::NssetHandleRegistrationObstruction::Enum> > check_res =
        Epp::Nsset::check_nsset(
            ctx,
            existing_nsset_handles,
            registrar.id
        );

    for(std::map<std::string, Nullable<Epp::Nsset::NssetHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
        it != check_res.end();
        ++it
    ) {
        BOOST_CHECK(it->second.get_value() == Epp::Nsset::NssetHandleRegistrationObstruction::registered_handle);
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
