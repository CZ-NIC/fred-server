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

#include "tests/interfaces/epp/fixture.h"
#include "tests/interfaces/epp/contact/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/contact/delete_contact.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(DeleteContact)

bool delete_invalid_registrar_id_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_invalid_registrar_id, HasInvalidSessionRegistrar)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::delete_contact(
            ctx,
            "contacthandle",
            config.delete_contact_config_data,
            session.data
        ),
        Epp::EppResponseFailure,
        delete_invalid_registrar_id_exception
    );
}

bool delete_fail_nonexistent_handle_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_nonexistent_handle, HasContact)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::delete_contact(
            ctx,
            "SOMEobscureString",
            config.delete_contact_config_data,
            session.data
        ),
        Epp::EppResponseFailure,
        delete_fail_nonexistent_handle_exception
    );
}

bool delete_fail_wrong_registrar_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authorization_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::registrar_autor);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::unauthorized_registrar);
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_wrong_registrar, HasContact)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            config.delete_contact_config_data,
            Test::Session(ctx, Test::Registrar(ctx).data.id).data
        ),
        Epp::EppResponseFailure,
        delete_fail_wrong_registrar_exception
    );
}

bool delete_fail_prohibiting_status1_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status1, HasContactWithServerUpdateProhibited)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            config.delete_contact_config_data,
            session.data
        ),
        Epp::EppResponseFailure,
        delete_fail_prohibiting_status1_exception
    );
}

struct has_contact_with_server_delete_prohibited : HasContactWithStatus {
    has_contact_with_server_delete_prohibited()
    :   HasContactWithStatus("serverDeleteProhibited")
    { }
};

bool delete_fail_prohibiting_status2_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status2, HasContactWithDeleteCandidate)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            config.delete_contact_config_data,
            session.data
        ),
        Epp::EppResponseFailure,
        delete_fail_prohibiting_status2_exception
    );
}

struct HasContactOwningDomain : HasContact {
    HasContactOwningDomain() {
        Fred::CreateDomain("domain.cz", registrar.handle, contact.handle).exec(ctx);
    }
};

bool delete_fail_owning_domain_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_association_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_owning_domain, HasContactOwningDomain)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            config.delete_contact_config_data,
            session.data
        ),
        Epp::EppResponseFailure,
        delete_fail_owning_domain_exception
    );
}

struct has_contact_owning_domain_and_another_admin_contact : HasContact {
    has_contact_owning_domain_and_another_admin_contact() {
        const std::string another_contact_handle = "contactAnother";
        Fred::CreateContact(another_contact_handle, registrar.handle).exec(ctx);

        Fred::CreateDomain("domain.cz", registrar.handle, another_contact_handle)
            .set_admin_contacts( boost::assign::list_of(contact.handle) )
            .exec(ctx);
    }
};

bool delete_fail_administrating_domain_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_association_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_administrating_domain, has_contact_owning_domain_and_another_admin_contact)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            config.delete_contact_config_data,
            session.data
        ),
        Epp::EppResponseFailure,
        delete_fail_administrating_domain_exception
    );
}

struct has_contact_administrating_nsset : HasContact {
    has_contact_administrating_nsset() {
        Fred::CreateNsset("nsset1x", registrar.handle)
            .set_tech_contacts( boost::assign::list_of(contact.handle) )
            .exec(ctx);
    }
};

bool delete_fail_linked_nsset_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_association_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_linked_nsset, has_contact_administrating_nsset)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            config.delete_contact_config_data,
            session.data
        ),
        Epp::EppResponseFailure,
        delete_fail_linked_nsset_exception
    );
}

struct HasContactAdministratingKeyset : HasContact {
    HasContactAdministratingKeyset() {
        Fred::CreateKeyset("keyset1x", registrar.handle)
            .set_tech_contacts( boost::assign::list_of(contact.handle) )
            .exec(ctx);
    }
};

bool delete_fail_linked_keyset_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_association_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_linked_keyset, HasContactAdministratingKeyset)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::delete_contact(
            ctx,
            contact.handle,
            config.delete_contact_config_data,
            session.data
        ),
        Epp::EppResponseFailure,
        delete_fail_linked_keyset_exception
    );
}

BOOST_FIXTURE_TEST_CASE(delete_ok, HasContact)
{
    Epp::Contact::delete_contact(
        ctx,
        contact.handle,
        config.delete_contact_config_data,
        session.data
    );

    BOOST_CHECK_EQUAL(
        Fred::InfoContactHistoryById(contact.id).exec(ctx).rbegin()->info_contact_data.delete_time.isnull(),
        false
    );
}

BOOST_FIXTURE_TEST_CASE(delete_ok_states_are_upgraded, HasContactWithServerTransferProhibitedRequest)
{
    Epp::Contact::delete_contact(
        ctx,
        contact.handle,
        config.delete_contact_config_data,
        session.data
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
