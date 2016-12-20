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

#include "src/epp/contact/delete_contact.h"

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(DeleteContact)

BOOST_FIXTURE_TEST_CASE(delete_invalid_registrar_id, has_contact)
{
    BOOST_CHECK_THROW(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            0
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(delete_fail_nonexistent_handle, has_contact)
{
    BOOST_CHECK_THROW(
        Epp::Contact::delete_contact(
            ctx,
            "SOMEobscureString",
            registrar.id
        ),
        Epp::NonexistentHandle
    );
}

BOOST_FIXTURE_TEST_CASE(delete_fail_wrong_registrar, has_contact_and_a_different_registrar)
{
    BOOST_CHECK_THROW(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            the_different_registrar.id
        ),
        Epp::AuthorizationError
    );
}

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status1, has_contact_with_server_update_prohibited)
{
    BOOST_CHECK_THROW(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            registrar.id
        ),
        Epp::ObjectStatusProhibitsOperation
    );
}

struct has_contact_with_server_delete_prohibited : has_contact_with_status {
    has_contact_with_server_delete_prohibited()
    :   has_contact_with_status("serverDeleteProhibited")
    { }
};

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status2, has_contact_with_delete_candidate)
{
    BOOST_CHECK_THROW(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            registrar.id
        ),
        Epp::ObjectStatusProhibitsOperation
    );
}

struct has_contact_owning_domain : has_contact {
    has_contact_owning_domain() {
        Fred::CreateDomain("domain.cz", registrar.handle, contact.handle).exec(ctx);
    }
};

BOOST_FIXTURE_TEST_CASE(delete_fail_owning_domain, has_contact_owning_domain)
{
    BOOST_CHECK_THROW(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            registrar.id
        ),
        Epp::ObjectAssociationProhibitsOperation
    );
}

struct has_contact_owning_domain_and_another_admin_contact : has_contact {
    has_contact_owning_domain_and_another_admin_contact() {
        const std::string another_contact_handle = "contactAnother";
        Fred::CreateContact(another_contact_handle, registrar.handle).exec(ctx);

        Fred::CreateDomain("domain.cz", registrar.handle, another_contact_handle)
            .set_admin_contacts( boost::assign::list_of(contact.handle) )
            .exec(ctx);
    }
};

BOOST_FIXTURE_TEST_CASE(delete_fail_administrating_domain, has_contact_owning_domain_and_another_admin_contact)
{
    BOOST_CHECK_THROW(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            registrar.id
        ),
        Epp::ObjectAssociationProhibitsOperation
    );
}

struct has_contact_administrating_nsset : has_contact {
    has_contact_administrating_nsset() {
        Fred::CreateNsset("nsset1x", registrar.handle)
            .set_tech_contacts( boost::assign::list_of(contact.handle) )
            .exec(ctx);
    }
};

BOOST_FIXTURE_TEST_CASE(delete_fail_linked_nsset, has_contact_administrating_nsset)
{
    BOOST_CHECK_THROW(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            registrar.id
        ),
        Epp::ObjectAssociationProhibitsOperation
    );
}

struct has_contact_administrating_keyset : has_contact {
    has_contact_administrating_keyset() {
        Fred::CreateKeyset("keyset1x", registrar.handle)
            .set_tech_contacts( boost::assign::list_of(contact.handle) )
            .exec(ctx);
    }
};

BOOST_FIXTURE_TEST_CASE(delete_fail_linked_keyset, has_contact_administrating_keyset)
{
    BOOST_CHECK_THROW(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            registrar.id
        ),
        Epp::ObjectAssociationProhibitsOperation
    );
}

BOOST_FIXTURE_TEST_CASE(delete_ok, has_contact)
{
    Epp::Contact::delete_contact(
        ctx,
        contact.handle,
        registrar.id
    );

    BOOST_CHECK_EQUAL(
        Fred::InfoContactHistoryById(contact.id).exec(ctx).rbegin()->info_contact_data.delete_time.isnull(),
        false
    );
}

BOOST_FIXTURE_TEST_CASE(delete_ok_states_are_upgraded, has_contact_with_server_transfer_prohibited_request)
{
    Epp::Contact::delete_contact(
        ctx,
        contact.handle,
        registrar.id
    );

    /* now object has the state server_transfer_prohibited request itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(contact.id).exec(ctx) ) {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find( object_states_after.begin(), object_states_after.end(), status )
            !=
            object_states_after.end()
        );
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
