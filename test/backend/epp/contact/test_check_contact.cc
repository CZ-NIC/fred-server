/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "test/backend/epp/contact/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/contact/check_contact.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Contact)

BOOST_FIXTURE_TEST_SUITE(CheckContact, autocommitting_context)

bool test_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(test_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    const std::set<std::string> contact_handles =
            boost::assign::list_of(ValidHandle().handle).convert_to_container<std::set<std::string> >();

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::check_contact(
                    ctx,
                    contact_handles,
                    DefaultCheckContactConfigData(),
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            test_invalid_registrar_id_exception);
}

BOOST_FIXTURE_TEST_CASE(test_result_size_empty, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EQUAL(
            ::Epp::Contact::check_contact(
                    ctx,
                    std::set<std::string>(),
                    DefaultCheckContactConfigData(),
                    session.data).size(),
            0);
}

BOOST_FIXTURE_TEST_CASE(test_result_size_nonempty, supply_ctx<HasRegistrarWithSession>)
{
    const std::set<std::string> contact_handles =
            boost::assign::list_of
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

    BOOST_CHECK_EQUAL(
            ::Epp::Contact::check_contact(
                    ctx,
                    contact_handles,
                    DefaultCheckContactConfigData(),
                    session.data)
                    .size(),
            contact_handles.size());
}

BOOST_FIXTURE_TEST_CASE(test_invalid_handle, supply_ctx<HasRegistrarWithSession>)
{
    const std::set<std::string> contact_handles =
            boost::assign::list_of
                    ("")
                    ("1234567890123456789012345678901234567890123456789012345678901234") // <== 64 chars
                    ("*")
                    ("!")
                    ("@")
                    ("a*")
                    ("*a")
                    ("a*a").convert_to_container<std::set<std::string> >();

    const std::map<std::string, Nullable< ::Epp::Contact::ContactHandleRegistrationObstruction::Enum> > check_res =
            ::Epp::Contact::check_contact(
                    ctx,
                    contact_handles,
                    DefaultCheckContactConfigData(),
                    session.data);

    for (std::map<std::string, Nullable< ::Epp::Contact::ContactHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
         it != check_res.end();
         ++it)
    {
        BOOST_CHECK(
                it->second.get_value() ==
                ::Epp::Contact::ContactHandleRegistrationObstruction::invalid_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(test_protected_handle, supply_ctx<HasRegistrarWithSession>)
{
    std::set<std::string> protected_handles;
   
    protected_handles =
            boost::assign::list_of
                    ("protHandleX1")
                    ("protHandleX2")
                    ("protHandleX3")
                    ("protHandleXa")
                    ("protHandleXb")
                    ("protHandleXc").convert_to_container<std::set<std::string> >();

    BOOST_FOREACH (const std::string& handle, protected_handles)
    {
        ::LibFred::CreateContact(handle, registrar.data.handle).exec(ctx);
        ::LibFred::DeleteContactByHandle(handle).exec(ctx);
    }

    const std::map<std::string, Nullable< ::Epp::Contact::ContactHandleRegistrationObstruction::Enum> > check_res =
            ::Epp::Contact::check_contact(
                    ctx,
                    protected_handles,
                    DefaultCheckContactConfigData(),
                    session.data);

    for(std::map<std::string, Nullable< ::Epp::Contact::ContactHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
        it != check_res.end();
        ++it
    ) {
        BOOST_CHECK(it->second.get_value() == ::Epp::Contact::ContactHandleRegistrationObstruction::protected_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(test_nonexistent_handle, supply_ctx<HasRegistrarWithSession>)
{
    const std::set<std::string> contact_handles =
            boost::assign::list_of
                    ("abc123")
                    ("def234")
                    ("ghi345")
                    ("jkl456")
                    ("mno567")
                    ("pqr678")
                    ("xyz789").convert_to_container<std::set<std::string> >();

    const std::map<std::string, Nullable< ::Epp::Contact::ContactHandleRegistrationObstruction::Enum> > check_res =
            ::Epp::Contact::check_contact(
                    ctx,
                    contact_handles,
                    DefaultCheckContactConfigData(),
                    session.data);

    for(std::map<std::string, Nullable< ::Epp::Contact::ContactHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
        it != check_res.end();
        ++it
    ) {
        BOOST_CHECK( it->second.isnull() );
    }
}

BOOST_FIXTURE_TEST_CASE(test_existing, supply_ctx<HasRegistrarWithSession>)
{
    std::set<std::string> existing_contact_handles;
    existing_contact_handles =
            boost::assign::list_of
                    ("handle01")
                    ("handle02")
                    ("handle03")
                    ("handle0a")
                    ("handle0b")
                    ("handle0c").convert_to_container<std::set<std::string> >();

    BOOST_FOREACH(const std::string& handle, existing_contact_handles) {
        ::LibFred::CreateContact(handle, registrar.data.handle).exec(ctx);
    }

    const std::map<std::string, Nullable< ::Epp::Contact::ContactHandleRegistrationObstruction::Enum> > check_res =
            ::Epp::Contact::check_contact(
                    ctx,
                    existing_contact_handles,
                    DefaultCheckContactConfigData(),
                    session.data);

    for(std::map<std::string, Nullable< ::Epp::Contact::ContactHandleRegistrationObstruction::Enum> >::const_iterator it = check_res.begin();
        it != check_res.end();
        ++it
    ) {
        BOOST_CHECK(it->second.get_value() == ::Epp::Contact::ContactHandleRegistrationObstruction::registered_handle);
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
