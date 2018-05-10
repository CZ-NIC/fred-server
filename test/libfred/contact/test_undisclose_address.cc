/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#define BOOST_TEST_NO_MAIN

#include "src/libfred/object_state/create_object_state_request_id.hh"
#include "src/libfred/object_state/get_object_states.hh"
#include "src/libfred/object_state/perform_object_state_request.hh"
#include "src/libfred/registrable_object/contact/create_contact.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/libfred/registrar/create_registrar.hh"
#include "src/libfred/registrable_object/contact/undisclose_address.hh"
#include "test/libfred/contact/fixture.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test {
namespace LibFred {
namespace Contact {

BOOST_AUTO_TEST_SUITE(TestUndiscloseAddress)

// fixtures

namespace {

struct context_holder
    : virtual instantiate_db_template
{
    ::LibFred::OperationContextCreator ctx;
};

template <class T>
struct supply_ctx : context_holder, T
{
    supply_ctx()
        : context_holder(),
          T(ctx)
    {
    }
};

struct ContactWithStatusIdentifiedContact
    : Test::LibFred::Contact::ContactWithStatus
{


    ContactWithStatusIdentifiedContact(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatus(_ctx, _registrar_handle, "identifiedContact")
    {
    }


};

struct HasIdentifiedContactWithoutOrganization {
    Test::LibFred::Contact::Registrar registrar;
    ContactWithStatusIdentifiedContact identified_contact;
    Test::LibFred::Contact::Registrar dedicated_registrar;
    HasIdentifiedContactWithoutOrganization(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          identified_contact(_ctx, registrar.data.handle),
          dedicated_registrar(_ctx, "REG-DEDICATED")
    {
        ::LibFred::UpdateContactById(identified_contact.data.id, registrar.data.handle).set_discloseaddress(true).set_organization("").exec(_ctx);

        BOOST_REQUIRE(::LibFred::ObjectStatesInfo(::LibFred::GetObjectStates(identified_contact.data.id).exec(_ctx)).presents(::LibFred::Object_State::identified_contact));
        const ::LibFred::InfoContactData data = ::LibFred::InfoContactById(identified_contact.data.id).exec(_ctx).info_contact_data;
        BOOST_REQUIRE(data.discloseaddress);
        BOOST_REQUIRE(data.organization.get_value_or("").empty());
    }
};

} // namespace Test::LibFred::Contact::TestUndiscloseAddress::{anonymous}

BOOST_FIXTURE_TEST_CASE(test_without_organization_ok, supply_ctx<HasIdentifiedContactWithoutOrganization>)
{
    BOOST_CHECK_NO_THROW(::LibFred::Contact::undisclose_address(ctx, identified_contact.data.id, dedicated_registrar.data.handle););
    const auto info_contact_data = ::LibFred::InfoContactById(identified_contact.data.id).exec(ctx).info_contact_data;
    BOOST_CHECK_EQUAL(info_contact_data.discloseaddress, false);
    BOOST_CHECK_EQUAL(info_contact_data.update_registrar_handle, dedicated_registrar.data.handle);
}

BOOST_FIXTURE_TEST_CASE(test_nonexisting_registrar_fail, supply_ctx<HasIdentifiedContactWithoutOrganization>)
{
    BOOST_CHECK_THROW(
            ::LibFred::Contact::undisclose_address(ctx, identified_contact.data.id, "REG-NONEXISTING"),
            std::exception
    );
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test::LibFred::Contact
} // namespace Test::LibFred
} // namespace Test
