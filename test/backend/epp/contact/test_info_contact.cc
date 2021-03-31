/*
 * Copyright (C) 2016-2020  CZ.NIC, z. s. p. o.
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
#include "test/backend/epp/contact/util.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/contact/info_contact.hh"
#include "src/backend/epp/contact/info_contact_config_data.hh"
#include "src/backend/epp/contact/util.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/session_data.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(InfoContact)

namespace {

bool info_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(info_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::info_contact(
            ctx,
            ValidHandle().handle,
            DefaultInfoContactConfigData(),
            session_with_unauthenticated_registrar.data
        ),
        ::Epp::EppResponseFailure,
        info_invalid_registrar_id_exception);
}

namespace {

bool info_fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(info_fail_nonexistent_handle, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::info_contact(
            ctx,
            NonexistentHandle().handle,
            DefaultInfoContactConfigData(),
            session.data
        ),
        ::Epp::EppResponseFailure,
        info_fail_nonexistent_handle_exception);
}

BOOST_FIXTURE_TEST_CASE(info_ok_full_data_for_sponsoring_registrar, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    check_equal(
            ::Epp::Contact::info_contact(
                    ctx,
                    contact.data.handle,
                    DefaultInfoContactConfigData(),
                    session.data),
            contact.data);
}

BOOST_FIXTURE_TEST_CASE(info_ok_full_data_for_different_registrar, supply_ctx<HasRegistrarWithSessionAndContactOfDifferentRegistrar>)
{
    check_equal_but_no_authinfopw(
            ::Epp::Contact::info_contact(
                    ctx,
                    contact_of_different_registrar.data.handle,
                    DefaultInfoContactConfigData(),
                    session.data),
            contact_of_different_registrar.data);
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact/InfoContact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

}//namespace Test
