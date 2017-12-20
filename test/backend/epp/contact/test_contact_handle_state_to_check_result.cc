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

#include <boost/test/unit_test.hpp>

#include "src/backend/epp/contact/contact_handle_state_to_check_result.hh"

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(ContactHandleStateToCheckResult)

BOOST_AUTO_TEST_CASE(test_conversion)
{
    BOOST_CHECK(
        ::Epp::Contact::contact_handle_state_to_check_result(
            ::LibFred::ContactHandleState::SyntaxValidity::invalid,
            ::LibFred::ContactHandleState::Registrability::registered
        )
        ==
        ::Epp::Contact::ContactHandleRegistrationObstruction::registered_handle
    );

    BOOST_CHECK(
        ::Epp::Contact::contact_handle_state_to_check_result(
            ::LibFred::ContactHandleState::SyntaxValidity::invalid,
            ::LibFred::ContactHandleState::Registrability::available
        )
        ==
        ::Epp::Contact::ContactHandleRegistrationObstruction::invalid_handle
    );

    BOOST_CHECK(
        ::Epp::Contact::contact_handle_state_to_check_result(
            ::LibFred::ContactHandleState::SyntaxValidity::invalid,
            ::LibFred::ContactHandleState::Registrability::in_protection_period
        )
        ==
        ::Epp::Contact::ContactHandleRegistrationObstruction::protected_handle
    );

    BOOST_CHECK(
        ::Epp::Contact::contact_handle_state_to_check_result(
            ::LibFred::ContactHandleState::SyntaxValidity::valid,
            ::LibFred::ContactHandleState::Registrability::registered
        )
        ==
        ::Epp::Contact::ContactHandleRegistrationObstruction::registered_handle
    );

    BOOST_CHECK(
        ::Epp::Contact::contact_handle_state_to_check_result(
            ::LibFred::ContactHandleState::SyntaxValidity::valid,
            ::LibFred::ContactHandleState::Registrability::available
        )
        ==
        Nullable< ::Epp::Contact::ContactHandleRegistrationObstruction::Enum>()
    );

    BOOST_CHECK(
        ::Epp::Contact::contact_handle_state_to_check_result(
            ::LibFred::ContactHandleState::SyntaxValidity::valid,
            ::LibFred::ContactHandleState::Registrability::in_protection_period
        )
        ==
        ::Epp::Contact::ContactHandleRegistrationObstruction::protected_handle
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
